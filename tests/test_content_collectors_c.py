"""Separate-translation-unit composition checks for context collectors."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which("cc"), "requires a host C compiler")
class ContentCollectorsTests(unittest.TestCase):
    def test_all_context_collectors_compose_real_dependencies(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / "content-collectors-test"
            subprocess.run([
                "cc", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
                "-I", str(ROOT / "include"),
                str(ROOT / "tests/c/content_collectors_harness.c"),
                str(ROOT / "src/content_collect_at10.c"),
                str(ROOT / "src/content_collect_remaining.c"),
                str(ROOT / "src/list_visit.c"),
                str(ROOT / "src/list_traversal.c"),
                str(ROOT / "src/tree_reverse_visit.c"),
                str(ROOT / "src/list_accessors.c"),
                str(ROOT / "src/content_payload_callback.c"),
                str(ROOT / "src/content_tree_payload_callback.c"),
                str(ROOT / "src/content_append.c"),
                "-o", str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True,
                           text=True)


if __name__ == "__main__":
    unittest.main()
