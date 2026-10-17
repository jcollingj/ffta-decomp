"""Composition checks for the recovered content list fill wrapper."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which("cc"), "requires a host C compiler")
class ContentFillTests(unittest.TestCase):
    def test_wrapper_composes_real_list_payload_and_append_sources(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / "content-fill-test"
            subprocess.run([
                "cc", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
                "-I", str(ROOT / "include"),
                str(ROOT / "tests/c/content_fill_harness.c"),
                str(ROOT / "src/content_fill.c"),
                str(ROOT / "src/list_visit.c"),
                str(ROOT / "src/list_accessors.c"),
                str(ROOT / "src/content_payload_callback.c"),
                str(ROOT / "src/content_append.c"),
                "-o", str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True,
                           text=True)


if __name__ == "__main__":
    unittest.main()
