"""Composition checks for the context-selected tree payload visitors."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which("cc"), "requires a host C compiler")
class ContentTreeContextVisitTests(unittest.TestCase):
    def test_six_visitors_compose_with_real_tree_adapter_and_traversals(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / "content-tree-context-visit-test"
            subprocess.run([
                "cc", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
                "-I", str(ROOT / "include"),
                str(ROOT / "tests/c/content_tree_context_visit_harness.c"),
                str(ROOT / "src/content_tree_context_visit.c"),
                str(ROOT / "src/content_tree_payload_callback.c"),
                str(ROOT / "src/list_accessors.c"),
                str(ROOT / "src/list_traversal.c"),
                str(ROOT / "src/tree_reverse_visit.c"),
                "-o", str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True,
                           capture_output=True, text=True)


if __name__ == "__main__":
    unittest.main()
