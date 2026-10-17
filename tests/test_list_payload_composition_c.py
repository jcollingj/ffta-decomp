"""Native pointer and numeric payload composition across production TUs."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which("cc"), "requires a host C compiler")
class ListPayloadCompositionTests(unittest.TestCase):
    def test_real_append_get_and_find_compose_without_narrowing(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / "list-payload-composition-test"
            subprocess.run([
                "cc", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
                "-I", str(ROOT / "include"),
                str(ROOT / "tests/c/list_payload_composition_harness.c"),
                str(ROOT / "src/list_append.c"),
                str(ROOT / "src/list_accessors.c"),
                str(ROOT / "src/list_find_word.c"),
                "-o", str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True,
                           capture_output=True, text=True)


if __name__ == "__main__":
    unittest.main()
