"""Host integration checks for the original content selection lookup."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which("cc"), "requires a host C compiler")
class ContentSelectionLookupTests(unittest.TestCase):
    def test_real_getter_visitor_predicate_and_wrapper(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / "content-selection-lookup-test"
            subprocess.run([
                "cc", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
                "-Wno-pointer-to-int-cast", "-I", str(ROOT / "include"),
                str(ROOT / "tests/c/content_selection_lookup_harness.c"),
                str(ROOT / "src/content_selection_lookup.c"),
                str(ROOT / "src/content_selection_predicate.c"),
                str(ROOT / "src/list_accessors.c"),
                str(ROOT / "src/list_visit.c"),
                "-o", str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
