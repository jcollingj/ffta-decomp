import importlib.util
import contextlib
import io
import json
import math
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("luna_sessions", ROOT / "tools/decomp/luna_sessions.py")
luna_sessions = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(luna_sessions)


class LunaSessionRegistryTests(unittest.TestCase):
    def setUp(self):
        self.registry = {"target_sessions": 2, "sessions": [
            {"id": 1, "agent": "fixture-agent", "model": "model-x", "task": "fixture task",
             "status": "reviewed", "review_evidence": "synthetic evidence"},
            {"id": 2, "status": "planned"},
        ]}

    def test_synthetic_registry_is_valid_and_unknown_timing_stays_unknown(self):
        luna_sessions.validate(self.registry)
        result = luna_sessions.report(self.registry)
        self.assertLessEqual(result["reviewed"], result["started"])
        self.assertLessEqual(result["started"], result["target_sessions"])

    def test_synthetic_fixture_has_exact_counts_and_unknown_metrics(self):
        r = {"target_sessions": 10, "sessions": [
            {"id": 1, "agent": "agent-a", "model": "model-x", "task": "task-a",
             "status": "reviewed", "review_commit": "abc", "accepted_matching_functions": 2,
             "accepted_matching_bytes": 12, "accepted_matching_code_bytes": 4,
             "accepted_matching_data_bytes": 8, "parent_minutes": 4, "agent_minutes": None,
             "revision_rounds": 1, "parent_corrective_edits": 2, "first_review_accepted": False,
             "started_utc": "2026-10-11T10:00:00-06:00", "finished_utc": "2026-10-11T10:30:00-06:00"},
            {"id": 2, "agent": "agent-b", "model": "model-x", "task": "task-b", "status": "running"},
            {"id": 3, "status": "planned"}]}
        result = luna_sessions.report(r)
        self.assertEqual((result["started"], result["reviewed"], result["planned"]), (2, 1, 1))
        m = result["reviewed_metrics"]
        self.assertEqual((m["accepted_matching_functions"], m["accepted_matching_bytes"]), (2, 12))
        self.assertEqual((m["accepted_matching_code_bytes"], m["accepted_matching_data_bytes"],
                          m["unclassified_accepted_bytes"]), (4, 8, 0))
        self.assertEqual(m["first_review_accepted"], {"true": 0, "false": 1, "unknown": 0})
        self.assertEqual(m["reviewed_accepted_outputs"], 1)
        self.assertEqual(m["parent_corrective_edits"], 2)
        self.assertEqual(m["revision_rounds"], 1)
        self.assertEqual(m["elapsed_minutes"], {"known_minutes": 30, "known_records": 1, "unknown_records": 0})
        self.assertEqual(m["parent_minutes"], {"known_minutes": 4, "known_records": 1, "unknown_records": 0})
        self.assertEqual(m["agent_minutes"], {"known_minutes": 0, "known_records": 0, "unknown_records": 1})
        self.assertEqual(result["reviewed_batches_of_10"][0]["reviewed"], 1)

    def test_planned_is_not_started(self):
        r = {"target_sessions": 2, "sessions": [
            {"id": 1, "agent": "agent-a", "model": "model-x", "task": "task-a",
             "status": "reviewed", "review_commit": "abc"},
            {"id": 2, "status": "planned"}]}
        out = luna_sessions.report(r)
        self.assertEqual((out["started"], out["reviewed"], out["planned"]), (1, 1, 1))

    def test_rejects_duplicate_ids_unknown_status_and_gaps(self):
        base = {"target_sessions": 4, "sessions": []}
        for rows in (
            [{"id": 1, "status": "planned"}, {"id": 1, "status": "planned"}],
            [{"id": 1, "status": "finished"}],
            [{"id": 1, "status": "planned"}, {"id": 3, "status": "planned"}],
        ):
            with self.subTest(rows=rows), self.assertRaises(ValueError):
                luna_sessions.validate(dict(base, sessions=rows))

    def test_rejects_over_target_and_review_without_evidence(self):
        with self.assertRaisesRegex(ValueError, "above target"):
            luna_sessions.validate({"target_sessions": 1, "sessions": [
                {"id": 1, "status": "planned"}, {"id": 2, "status": "planned"}]})
        with self.assertRaisesRegex(ValueError, "review evidence"):
            luna_sessions.validate({"target_sessions": 1, "sessions": [{
                "id": 1, "agent": "agent-a", "model": "model-x", "task": "task-a", "status": "reviewed"}]})

    def test_reviewed_metrics_count_only_reviewed_and_handle_unknowns(self):
        r = {"target_sessions": 3, "sessions": [
            {"id": 1, "agent": "agent-a", "model": "model-x", "task": "task-a",
             "status": "reviewed", "review_evidence": "accepted", "accepted_matching_bytes": 12,
             "parent_minutes": 4, "agent_minutes": None, "revision_rounds": 1},
            {"id": 2, "agent": "agent-b", "model": "model-x", "task": "task-b",
             "status": "running", "accepted_matching_bytes": 90, "parent_minutes": 100},
            {"id": 3, "status": "planned", "accepted_matching_bytes": 400}]}
        out = luna_sessions.report(r)
        self.assertEqual(out["reviewed_metrics"]["accepted_matching_bytes"], 12)
        self.assertEqual(out["started_metrics"]["accepted_matching_bytes"], 102)
        self.assertEqual(out["reviewed_metrics"]["agent_minutes"]["unknown_records"], 1)

    def test_rejects_invalid_metric_types(self):
        with self.assertRaisesRegex(ValueError, "nonnegative integer"):
            luna_sessions.validate({"target_sessions": 1, "sessions": [{
                "id": 1, "agent": "agent-a", "model": "model-x", "task": "task-a",
                "status": "reviewed", "review_commit": "abc", "revision_rounds": True}]})

    def test_historical_unclassified_bytes_and_running_review_evidence(self):
        r = {"target_sessions": 2, "sessions": [
            {"id": 1, "agent": "agent-a", "model": "model-x", "task": "task-a",
             "status": "reviewed", "review_evidence": "verified", "accepted_matching_bytes": 10,
             "first_review_accepted": True},
            {"id": 2, "agent": "agent-b", "model": "model-x", "task": "task-b",
             "status": "running", "review_evidence": "earlier review", "accepted_matching_bytes": 6}]}
        out = luna_sessions.report(r)
        self.assertEqual(out["reviewed_metrics"]["unclassified_accepted_bytes"], 10)
        self.assertEqual(out["reviewed_metrics"]["first_review_accepted"], {"true": 1, "false": 0, "unknown": 0})

    def test_rejects_bad_classification_first_review_and_timestamps(self):
        base = {"id": 1, "agent": "agent-a", "model": "model-x", "task": "task-a", "status": "running"}
        cases = [
            {"accepted_matching_bytes": 5, "accepted_matching_code_bytes": 6},
            {"first_review_accepted": 1},
            {"started_utc": "2026-10-11T10:00:00", "finished_utc": "2026-10-11T11:00:00Z"},
            {"finished_utc": "not-a-timestamp"},
            {"started_utc": "2026-10-11T11:00:00Z", "finished_utc": "2026-10-11T10:00:00Z"},
        ]
        for extra in cases:
            with self.subTest(extra=extra), self.assertRaises(ValueError):
                luna_sessions.validate({"target_sessions": 1, "sessions": [dict(base, **extra)]})

    def test_one_valid_timestamp_does_not_create_elapsed_or_effort(self):
        r = {"target_sessions": 1, "sessions": [{
            "id": 1, "agent": "agent-a", "model": "model-x", "task": "task-a",
            "status": "reviewed", "review_evidence": "reviewed", "started_utc": "2026-10-11T10:00:00Z",
            "agent_minutes": None}]}
        metrics = luna_sessions.report(r)["reviewed_metrics"]
        self.assertEqual(metrics["elapsed_minutes"], {"known_minutes": 0, "known_records": 0, "unknown_records": 1})
        self.assertEqual(metrics["agent_minutes"], {"known_minutes": 0, "known_records": 0, "unknown_records": 1})

    def test_lifecycle_timestamps_are_optional_but_validated_individually(self):
        base = {"target_sessions": 1, "sessions": [{
            "id": 1, "agent": "agent-a", "model": "model-x", "task": "task-a",
            "status": "reviewed", "review_evidence": "reviewed",
        }]}
        # Missing registration/review timestamps stay unknown and do not infer elapsed or effort.
        metrics = luna_sessions.report(base)["reviewed_metrics"]
        self.assertEqual(metrics["elapsed_minutes"]["unknown_records"], 1)
        self.assertEqual(metrics["agent_minutes"]["unknown_records"], 1)
        for field in ("registered_utc", "reviewed_utc"):
            for value in ("not-a-timestamp", "2026-10-12T01:00:00"):
                malformed = json.loads(json.dumps(base))
                malformed["sessions"][0][field] = value
                with self.subTest(field=field, value=value), self.assertRaisesRegex(ValueError, field):
                    luna_sessions.validate(malformed)

    def test_started_records_need_metadata_and_unique_agent_identity(self):
        incomplete = {"id": 1, "status": "running", "agent": "agent-a", "model": "model-x"}
        with self.assertRaisesRegex(ValueError, "nonempty task"):
            luna_sessions.validate({"target_sessions": 1, "sessions": [incomplete]})
        duplicate_agents = {"target_sessions": 2, "sessions": [
            {"id": 1, "agent": "agent-a", "model": "model-x", "task": "first", "status": "running"},
            {"id": 2, "agent": " agent-a ", "model": "model-y", "task": "followup counted as new", "status": "running"}]}
        with self.assertRaisesRegex(ValueError, "duplicate agent identity"):
            luna_sessions.validate(duplicate_agents)

    def test_rejects_nonfinite_and_negative_timings(self):
        for value in (math.nan, math.inf, -math.inf, -0.01):
            with self.subTest(value=value), self.assertRaisesRegex(ValueError, "nonnegative number"):
                luna_sessions.validate({"target_sessions": 1, "sessions": [{
                    "id": 1, "agent": "agent-a", "model": "model-x", "task": "task-a",
                    "status": "running", "agent_minutes": value}]})

    def test_text_report_includes_every_nonempty_batch_through_tenth(self):
        sessions = []
        for sid in range(1, 101):
            record = {"id": sid, "status": "planned"}
            if sid in (1, 21, 91):
                record.update(agent=f"agent-{sid}", model="model-x", task="task",
                              status="reviewed", review_evidence="fixture")
            sessions.append(record)
        with tempfile.TemporaryDirectory() as temp_dir:
            fixture_path = Path(temp_dir) / "registry.json"
            fixture_path.write_text(json.dumps({"target_sessions": 100, "sessions": sessions}))
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                self.assertEqual(luna_sessions.main([str(fixture_path)]), 0)
            lines = output.getvalue().splitlines()
            batch_lines = [line for line in lines if line.startswith("Reviewed batch ")]
            self.assertEqual(len(batch_lines), 3)
            self.assertIn("Reviewed batch 1–10: 1 reviewed", batch_lines[0])
            self.assertIn("Reviewed batch 21–30: 1 reviewed", batch_lines[1])
            self.assertIn("Reviewed batch 91–100: 1 reviewed", batch_lines[2])

            json_output = io.StringIO()
            with contextlib.redirect_stdout(json_output):
                self.assertEqual(luna_sessions.main([str(fixture_path), "--json"]), 0)
            parsed = json.loads(json_output.getvalue())
            self.assertEqual(parsed["reviewed_batches_of_10"], luna_sessions.report(
                json.loads(fixture_path.read_text()))["reviewed_batches_of_10"])


if __name__ == "__main__":
    unittest.main()
