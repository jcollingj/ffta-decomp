"""Host checks for adjacent global content flag modifiers."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which("cc"), "requires a host C compiler")
class ContentGlobalFlagsCTests(unittest.TestCase):
    def test_modifiers_match_fixed_whole_owner_images_and_reload_global(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / "content-global-flags-test"
            subprocess.run([
                "cc", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
                "-I", str(ROOT / "include"),
                str(ROOT / "tests/c/content_global_flags_harness.c"),
                str(ROOT / "src/content_global_flags.c"),
                str(ROOT / "src/content_globals.c"),
                "-o", str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True,
                           text=True)
