"""Separate-TU behavioral checks for content tree-key refresh callbacks."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which("cc"), "requires a host C compiler")
class ContentKeyRefreshTests(unittest.TestCase):
    def compile_and_run(self, directory, name, sources, define=None):
        program = Path(directory) / name
        command = [
            "cc", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
            "-I", str(ROOT / "include"),
        ]
        if define:
            command.extend(["-D", define])
        command.extend(str(ROOT / source) for source in sources)
        command.extend(["-o", str(program)])
        subprocess.run(command, check=True, capture_output=True, text=True)
        subprocess.run([str(program)], check=True, capture_output=True,
                       text=True)

    def test_real_callback_adapter_visitor_search_and_setter(self):
        with tempfile.TemporaryDirectory() as directory:
            self.compile_and_run(directory, "content-key-refresh-real", [
                "tests/c/content_key_refresh_harness.c",
                "src/content_key_refresh.c",
                "src/content_key_refresh_callback.c",
                "src/content_payload_callback.c",
                "src/list_accessors.c",
                "src/list_visit.c",
                "src/tree_search.c",
                "src/list_traversal.c",
                "src/list_search_callback.c",
                "src/list_set_words.c",
            ])

    def test_wrapper_reloads_owner_after_controlled_callback_mutation(self):
        with tempfile.TemporaryDirectory() as directory:
            self.compile_and_run(directory, "content-key-refresh-reload", [
                "tests/c/content_key_refresh_harness.c",
                "src/content_key_refresh.c",
                "src/content_payload_callback.c",
                "src/list_accessors.c",
                "src/list_visit.c",
            ], "CONTROLLED_CALLBACK")


if __name__ == "__main__":
    unittest.main()
