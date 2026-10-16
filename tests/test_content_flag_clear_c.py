"""Independent host checks for ClearContentFlag21FIfSet."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which("cc"), "requires a host C compiler")
class ContentFlagClearTests(unittest.TestCase):
    def _build_and_run(self, directory, name, extra_sources=(), define=None):
        program = Path(directory) / name
        command = [
            "cc", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
            "-I", str(ROOT / "include"),
        ]
        if define:
            command.append("-D" + define)
        command.extend([
            str(ROOT / "tests/c/content_flag_clear_harness.c"),
            str(ROOT / "src/nonmatching/content_flag_clear.c"),
        ])
        command.extend(str(ROOT / path) for path in extra_sources)
        command.extend(["-o", str(program)])
        subprocess.run(command, check=True, capture_output=True, text=True)
        subprocess.run([str(program)], check=True, capture_output=True, text=True)

    def test_controlled_getter_domain_and_call_contract(self):
        with tempfile.TemporaryDirectory() as directory:
            self._build_and_run(directory, "controlled")

    def test_real_game_flag_api_integration(self):
        with tempfile.TemporaryDirectory() as directory:
            self._build_and_run(
                directory, "real-apis", ("src/game_flags.c",),
                define="USE_REAL_GAME_FLAGS",
            )
