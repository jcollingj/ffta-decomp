"""Host checks for the tree numeric-key rebuild candidate."""
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


class TreeRebuildCandidateTests(unittest.TestCase):
    def build_and_run(self, directory, composition):
        program = Path(directory) / ("tree-rebuild-composition" if composition
                                     else "tree-rebuild-controlled")
        command = [
            "cc", "-std=gnu99", "-Wall", "-Wextra", "-Werror",
            "-I", str(ROOT / "include"),
        ]
        if composition:
            command.append("-DTREE_REBUILD_COMPOSITION_TEST")
        command.extend([
            str(ROOT / "tests/c/tree_rebuild_harness.c"),
            str(ROOT / "src/nonmatching/tree_rebuild.c"),
            str(ROOT / "src/tree_reverse_visit.c"),
            str(ROOT / "src/list_callbacks.c"),
        ])
        if composition:
            command.append(str(ROOT / "src/nonmatching/tree_detach.c"))
        command.extend(["-o", str(program)])
        subprocess.run(command, check=True)
        subprocess.run([str(program)], check=True)

    def test_controlled_leaf_detach_cases(self):
        with tempfile.TemporaryDirectory() as directory:
            self.build_and_run(directory, False)

    def test_real_traversal_callbacks_and_detach_composition(self):
        with tempfile.TemporaryDirectory() as directory:
            self.build_and_run(directory, True)


if __name__ == "__main__":
    unittest.main()
