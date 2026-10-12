"""End-to-end host composition through the shared Container owner type."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which("cc"), "requires a host C compiler")
class ContainerOwnerIntegrationTests(unittest.TestCase):
    def test_constructor_list_and_tree_apis_compose_without_casts(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / "container-owner-integration"
            sources = [
                "src/container_create.c",
                "src/list_count.c",
                "src/list_find_word.c",
                "src/list_visit.c",
                "src/nonmatching/tree_count.c",
                "src/list_traversal.c",
                "src/tree_reverse_visit.c",
                "src/tree_collect.c",
                "src/list_callbacks.c",
            ]
            subprocess.run([
                "cc", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
                "-I", str(ROOT / "include"),
                *(str(ROOT / source) for source in sources),
                str(ROOT / "tests/c/container_owner_integration_harness.c"),
                "-o", str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
