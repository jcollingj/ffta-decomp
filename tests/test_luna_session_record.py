import contextlib
import importlib.util
import io
import json
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
TOOL_DIR = ROOT / "tools/decomp"
sys.path.insert(0, str(TOOL_DIR))
SPEC = importlib.util.spec_from_file_location("luna_session_record", TOOL_DIR / "luna_session_record.py")
luna_session_record = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(luna_session_record)
sys.path.pop(0)
luna_sessions = sys.modules["luna_sessions"]


def fixture_registry():
    return {"target_sessions": 4, "count_definition": "distinct spawned agent", "sessions": [{
        "id": 1, "agent": "agent-existing", "model": "gpt-6-luna", "task": "old",
        "status": "reviewed", "review_evidence": "prior review", "accepted_matching_bytes": 7,
        "notes": "manually preserved metadata", "agent_minutes": None,
    }]}


class LunaSessionLifecycleTests(unittest.TestCase):
    def setUp(self):
        self.temp_dir = tempfile.TemporaryDirectory()
        self.path = Path(self.temp_dir.name) / "registry.json"
        self.path.write_text(json.dumps(fixture_registry(), indent=2) + "\n")

    def tearDown(self):
        self.temp_dir.cleanup()

    def _read(self):
        return json.loads(self.path.read_text())

    def test_register_adds_only_next_distinct_agent_and_keeps_time_unknown(self):
        registered = luna_session_record.register_spawned_session(
            self.path, agent="agent-new", model="gpt-6-luna", task="bounded task",
            registered_utc="2026-10-12T02:00:00Z")
        self.assertEqual(registered["id"], 2)
        self.assertEqual(registered["registered_utc"], "2026-10-12T02:00:00+00:00")
        self.assertEqual(registered["registration_timestamp_kind"], "registration")
        self.assertNotIn("started_utc", registered)
        self.assertIn("exact spawn timestamp unavailable", registered["timing_note"])
        self.assertIsNone(registered["agent_minutes"])
        started_effort = luna_sessions.report(self._read())["started_metrics"]["agent_minutes"]
        self.assertEqual(started_effort, {"known_minutes": 0, "known_records": 0, "unknown_records": 2})
        self.assertEqual(self._read()["sessions"][0]["notes"], "manually preserved metadata")

    def test_supplied_spawn_time_is_distinct_from_registration_time(self):
        row = luna_session_record.register_spawned_session(
            self.path, agent="agent-new", model="gpt-6-luna", task="bounded task",
            spawned_utc="2026-10-12T01:00:00Z", registered_utc="2026-10-12T02:00:00Z")
        self.assertEqual(row["started_utc"], "2026-10-12T01:00:00+00:00")
        self.assertEqual(row["registered_utc"], "2026-10-12T02:00:00+00:00")
        self.assertEqual(row["registration_timestamp_kind"], "registration_with_supplied_spawn_time")

    def test_review_requires_explicit_evidence_and_counts_then_preserves_other_fields(self):
        luna_session_record.register_spawned_session(
            self.path, agent="agent-new", model="gpt-6-luna", task="bounded task",
            registered_utc="2026-10-12T02:00:00Z")
        row = luna_session_record.mark_reviewed(
            self.path, session_id=2, evidence="Parent independently reran focused tests.",
            accepted_functions=2, accepted_code_bytes=44, accepted_data_bytes=3,
            first_review_accepted=False, reviewed_utc="2026-10-12T03:00:00Z")
        self.assertEqual(row["status"], "reviewed")
        self.assertEqual(row["accepted_matching_functions"], 2)
        self.assertEqual(row["accepted_matching_code_bytes"], 44)
        self.assertEqual(row["accepted_matching_data_bytes"], 3)
        self.assertEqual(row["accepted_matching_bytes"], 47)
        self.assertEqual(row["agent_minutes"], None)
        self.assertEqual(self._read()["sessions"][0]["notes"], "manually preserved metadata")

    def test_registration_cli_emits_machine_readable_record(self):
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            rc = luna_session_record.main([str(self.path), "register", "--agent", "agent-new",
                                           "--model", "gpt-6-luna", "--task", "task"])
        self.assertEqual(rc, 0)
        self.assertEqual(json.loads(output.getvalue())["id"], 2)

    def test_review_cli_emits_machine_readable_record(self):
        luna_session_record.register_spawned_session(
            self.path, agent="agent-new", model="gpt-6-luna", task="task",
            registered_utc="2026-10-12T02:00:00Z")
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            rc = luna_session_record.main([
                str(self.path), "review", "--id", "2", "--evidence", "Independent test rerun.",
                "--accepted-functions", "1", "--accepted-code-bytes", "24",
                "--accepted-data-bytes", "0", "--first-review-accepted", "true",
            ])
        self.assertEqual(rc, 0)
        row = json.loads(output.getvalue())
        self.assertEqual(row["status"], "reviewed")
        self.assertEqual(row["review_evidence"], "Independent test rerun.")
        self.assertEqual(row["accepted_matching_bytes"], 24)

    def test_malformed_lifecycle_timestamp_is_rejected_without_rewrite(self):
        for field in ("registered_utc", "reviewed_utc"):
            malformed = fixture_registry()
            malformed["sessions"].append({
                "id": 2, "agent": "agent-running", "model": "gpt-6-luna", "task": "task",
                "status": "running", field: "2026-10-12T02:00:00",
            })
            self.path.write_text(json.dumps(malformed))
            before = self.path.read_bytes()
            with self.subTest(field=field), self.assertRaisesRegex(ValueError, field):
                luna_session_record.register_spawned_session(
                    self.path, agent="agent-new", model="gpt-6-luna", task="task")
            self.assertEqual(self.path.read_bytes(), before)

    def test_invalid_operations_leave_file_bytes_unchanged(self):
        invalid_calls = [
            lambda: luna_session_record.register_spawned_session(
                self.path, agent="agent-existing", model="gpt-6-luna", task="duplicate"),
            lambda: luna_session_record.register_spawned_session(
                self.path, agent="agent-new", model="gpt-6-luna", task="task", spawned_utc="not-a-time"),
            lambda: luna_session_record.mark_reviewed(
                self.path, session_id=1, evidence="duplicate review", accepted_functions=0,
                accepted_code_bytes=0, accepted_data_bytes=0),
            lambda: luna_session_record.mark_reviewed(
                self.path, session_id=99, evidence="missing id", accepted_functions=0,
                accepted_code_bytes=0, accepted_data_bytes=0),
            lambda: luna_session_record.mark_reviewed(
                self.path, session_id=1, evidence=" ", accepted_functions=0,
                accepted_code_bytes=0, accepted_data_bytes=0),
            lambda: luna_session_record.mark_reviewed(
                self.path, session_id=1, evidence="evidence", accepted_functions=True,
                accepted_code_bytes=0, accepted_data_bytes=0),
        ]
        for operation in invalid_calls:
            before = self.path.read_bytes()
            with self.subTest(operation=operation), self.assertRaises(ValueError):
                operation()
            self.assertEqual(self.path.read_bytes(), before)

    def test_malformed_existing_registry_is_rejected_without_rewrite(self):
        malformed = fixture_registry()
        malformed["sessions"].append({"id": 3, "agent": "agent-existing", "model": "m",
                                       "task": "duplicate identity", "status": "running"})
        self.path.write_text(json.dumps(malformed))
        before = self.path.read_bytes()
        with self.assertRaisesRegex(ValueError, "duplicate agent identity"):
            luna_session_record.register_spawned_session(
                self.path, agent="agent-new", model="gpt-6-luna", task="task")
        self.assertEqual(self.path.read_bytes(), before)


if __name__ == "__main__":
    unittest.main()
