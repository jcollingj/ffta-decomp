import contextlib
import importlib.util
import io
import json
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools/decomp"))
SPEC = importlib.util.spec_from_file_location(
    "luna_experiment_report", ROOT / "tools/decomp/luna_experiment_report.py")
luna_experiment_report = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(luna_experiment_report)


def make_record(session_id, status="reviewed", **values):
    row = {"id": session_id, "status": status}
    if status != "planned":
        row.update(agent=f"agent-{session_id}", model="luna", task=f"task-{session_id}")
    if status == "reviewed":
        row["review_evidence"] = "independent synthetic evidence"
    row.update(values)
    return row


class LunaExperimentReportTests(unittest.TestCase):
    def test_pending_accepted_fields_do_not_enter_reviewed_totals(self):
        registry = {"target_sessions": 3, "sessions": [
            make_record(1, accepted_matching_functions=1, accepted_matching_bytes=6,
                        accepted_matching_code_bytes=6, revision_rounds=1,
                        parent_corrective_edits=0, first_review_accepted=True),
            make_record(2, "review_pending", accepted_matching_functions=99,
                        accepted_matching_bytes=999, accepted_matching_code_bytes=999),
            make_record(3, "planned"),
        ]}
        result = luna_experiment_report.assess(registry)
        self.assertEqual((result["started"], result["reviewed"], result["planned"]), (2, 1, 1))
        metrics = result["reviewed_metrics"]
        self.assertEqual(metrics["accepted_matching_functions"], 1)
        self.assertEqual(metrics["accepted_matching_bytes"], 6)
        self.assertEqual(metrics["accepted_matching_code_bytes"], 6)
        self.assertEqual(metrics["reviewed_accepted_outputs"], 1)
        self.assertFalse(result["complete"])

    def test_missing_fields_are_distinct_from_explicit_measured_zero_in_batch(self):
        registry = {"target_sessions": 10, "sessions": [
            make_record(1, accepted_matching_functions=0, accepted_matching_bytes=0,
                        accepted_matching_code_bytes=0, accepted_matching_data_bytes=0,
                        revision_rounds=0, parent_corrective_edits=0,
                        first_review_accepted=False, parent_minutes=0, agent_minutes=None),
            make_record(2),
        ]}
        result = luna_experiment_report.assess(registry)
        cov = result["reviewed_field_coverage"]
        self.assertEqual(cov["revision_rounds"], {"recorded": 1, "missing": 1})
        self.assertEqual(cov["parent_corrective_edits"], {"recorded": 1, "missing": 1})
        self.assertEqual(cov["accepted_matching_bytes"], {"recorded": 1, "missing": 1})
        self.assertEqual(cov["first_review_accepted"], {"recorded": 1, "missing": 1})
        self.assertEqual(cov["parent_minutes"], {"recorded": 1, "missing": 1})
        self.assertEqual(cov["agent_minutes"], {"recorded": 0, "missing": 2})
        batch = result["reviewed_batches_of_10"][0]
        self.assertEqual(batch["revision_rounds"], 0)
        self.assertEqual(batch["field_coverage"]["revision_rounds"],
                         {"recorded": 1, "missing": 1})
        self.assertEqual(batch["field_coverage"]["parent_corrective_edits"],
                         {"recorded": 1, "missing": 1})

    def test_first_review_all_unknown_has_no_percentage(self):
        registry = {"target_sessions": 2, "sessions": [make_record(1), make_record(2)]}
        output = luna_experiment_report.render_markdown(
            luna_experiment_report.assess(registry))
        self.assertIn("2 unknown; known-only rate not available (0 known)", output)
        self.assertIn("0/0/2 (n/a (0 known))", output)

    def test_duplicate_agent_identity_uses_registry_validation(self):
        registry = {"target_sessions": 2, "sessions": [
            make_record(1, agent="same"), make_record(2, agent="same")
        ]}
        with self.assertRaisesRegex(ValueError, "duplicate agent identity"):
            luna_experiment_report.assess(registry)

    def test_completion_requires_all_target_sessions_reviewed(self):
        ninety_nine = {"target_sessions": 100, "sessions": [
            make_record(i, "running" if i == 100 else "reviewed")
            for i in range(1, 101)
        ]}
        result_99 = luna_experiment_report.assess(ninety_nine)
        self.assertEqual((result_99["started"], result_99["reviewed"]), (100, 99))
        self.assertFalse(result_99["complete"])
        complete = {"target_sessions": 100, "sessions": [
            make_record(i) for i in range(1, 101)
        ]}
        result_100 = luna_experiment_report.assess(complete)
        self.assertEqual((result_100["started"], result_100["reviewed"]), (100, 100))
        self.assertTrue(result_100["complete"])

    def test_refresh_replaces_only_delimited_section(self):
        source = "before\n" + luna_experiment_report.BEGIN_MARKER + "\nold\n" + \
            luna_experiment_report.END_MARKER + "\nafter\n"
        updated = luna_experiment_report.replace_generated_section(source, "new report\n")
        self.assertTrue(updated.startswith("before\n" + luna_experiment_report.BEGIN_MARKER))
        self.assertTrue(updated.endswith(luna_experiment_report.END_MARKER + "\nafter\n"))
        self.assertIn("new report", updated)
        self.assertNotIn("old", updated)
        with self.assertRaisesRegex(ValueError, "exactly one"):
            luna_experiment_report.replace_generated_section("no markers", "report")

    def test_cli_can_refresh_temporary_document_and_json_is_deterministic(self):
        registry = {"target_sessions": 1, "sessions": [make_record(1)]}
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            registry_path = root / "registry.json"
            document_path = root / "doc.md"
            registry_path.write_text(json.dumps(registry))
            document_path.write_text("before\n" + luna_experiment_report.BEGIN_MARKER +
                                     "\nold\n" + luna_experiment_report.END_MARKER + "\nafter\n")
            with contextlib.redirect_stdout(io.StringIO()):
                luna_experiment_report.main([str(registry_path), "--refresh-doc",
                                             "--document", str(document_path)])
            text = document_path.read_text()
            self.assertTrue(text.startswith("before\n"))
            self.assertTrue(text.endswith("\nafter\n"))
            self.assertIn("Experiment status: complete", text)
            a = luna_experiment_report.render_markdown(luna_experiment_report.assess(registry))
            b = luna_experiment_report.render_markdown(luna_experiment_report.assess(registry))
            self.assertEqual(a, b)


if __name__ == "__main__":
    unittest.main()
