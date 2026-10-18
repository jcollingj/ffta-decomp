"""Host tests for the content-entry coordinate routines."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which("cc"), "requires a host C compiler")
class ContentPositionTests(unittest.TestCase):
    def test_setter_and_forwarder_with_controlled_copied_dependency(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / "content-position-test"
            subprocess.run([
                "cc", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
                "-I", str(ROOT / "include"),
                str(ROOT / "tests/c/content_position_harness.c"),
                str(ROOT / "src/content_position.c"),
                str(ROOT / "src/content_coordinate_value.c"),
                "-o", str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True,
                           capture_output=True, text=True)


if __name__ == "__main__":
    unittest.main()
