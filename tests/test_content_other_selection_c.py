"""Host composition checks for the two other content-selection wrappers."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which("cc"), "requires a host C compiler")
class ContentOtherSelectionTests(unittest.TestCase):
    def test_real_getter_predicate_visitor_and_both_wrappers(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / "content-other-selection-test"
            subprocess.run([
                "cc", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
                "-Wno-pointer-to-int-cast", "-I", str(ROOT / "include"),
                str(ROOT / "tests/c/content_other_selection_harness.c"),
                str(ROOT / "src/content_other_selection.c"),
                str(ROOT / "src/content_selection_predicate.c"),
                str(ROOT / "src/list_accessors.c"),
                str(ROOT / "src/list_visit.c"),
                "-o", str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True,
                           text=True)
