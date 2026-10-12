import importlib.util
import json
import math
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("luna_sessions", ROOT / "tools/decomp/luna_sessions.py")
luna_sessions = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(luna_sessions)


class LunaSessionRegistryTests(unittest.TestCase):
    def setUp(self):
        self.registry = json.loads((ROOT / "config/luna_sessions.json").read_text())

    def test_existing_registry_is_valid_and_unknown_timing_stays_unknown(self):
        luna_sessions.validate(self.registry)
        result = luna_sessions.report(self.registry)
        self.assertLessEqual(result["reviewed"], result["started"])
        self.assertLessEqual(result["started"], result["target_sessions"])

    def test_synthetic_fixture_has_exact_counts_and_unknown_metrics(self):
        r = {"target_sessions": 10, "sessions": [
            {"id": 1, "agent": "agent-a", "model": "model-x", "task": "task-a",
             "status": "reviewed", "review_commit": "abc", "accepted_matching_functions": 2,
             "accepted_matching_bytes": 12, "parent_minutes": 4, "agent_minutes": None,
             "revision_rounds": 1, "parent_corrective_edits": 0},
            {"id": 2, "agent": "agent-b", "model": "model-x", "task": "task-b", "status": "running"},
            {"id": 3, "status": "planned"}]}
        result = luna_sessions.report(r)
        self.assertEqual((result["started"], result["reviewed"], result["planned"]), (2, 1, 1))
        m = result["reviewed_metrics"]
        self.assertEqual((m["accepted_matching_functions"], m["accepted_matching_bytes"]), (2, 12))
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


if __name__ == "__main__":
    unittest.main()
