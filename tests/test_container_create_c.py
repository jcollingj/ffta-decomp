"""Host behavior checks for the nonmatching container constructor."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which("cc"), "requires a host C compiler")
class ContainerCreateTests(unittest.TestCase):
    def test_initializes_all_observed_fields_and_preserves_guards(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / "container-create-test"
            subprocess.run([
                "cc", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
                "-I", str(ROOT / "include"),
                str(ROOT / "src/container_create.c"),
                str(ROOT / "tests/c/container_create_harness.c"),
                "-o", str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
