"""Host fixtures for the adjacent unit content setup routines."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which("cc"), "requires a host C compiler")
class UnitContentSetupTests(unittest.TestCase):
    def compile_and_run(self, controlled):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / "unit-content-setup-test"
            command = [
                "cc", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
                "-Wno-pointer-to-int-cast", "-Wno-int-to-pointer-cast",
                "-I", str(ROOT / "include"),
            ]
            if controlled:
                command.append("-DCONTROLLED_PREDICATES")
            command.extend([
                str(ROOT / "tests/c/unit_content_setup_harness.c"),
                str(ROOT / "src/unit_content_setup.c"),
            ])
            if not controlled:
                command.append(str(ROOT / "src/unit_fields.c"))
            command.extend(["-o", str(program)])
            subprocess.run(command, check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True,
                           text=True)

    def test_real_unit_predicate_composition(self):
        self.compile_and_run(controlled=False)

    def test_controlled_predicate_order_and_reload(self):
        self.compile_and_run(controlled=True)
