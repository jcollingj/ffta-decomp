"""Separate-translation-unit host composition for item/unit/job stat readers."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
SOURCES = (
    "src/item.c",
    "src/job_property.c",
    "src/unit_property.c",
    "src/unit_equipped_stats.c",
    "src/unit_stat_dispatch.c",
    "src/unit_stat_caps.c",
    "src/unit_equipment_value16.c",
    # Explicitly nonmatching research only; it receives no recovery credit.
    "src/nonmatching/unit_clamped_values.c",
)


@unittest.skipUnless(shutil.which("cc"), "requires a host C compiler")
class UnitStatCompositionCTests(unittest.TestCase):
    def test_real_dependency_composition(self):
        with tempfile.TemporaryDirectory() as directory:
            temp = Path(directory)
            objects = []
            for source in SOURCES:
                obj = temp / (Path(source).stem + "-" + str(len(objects)) + ".o")
                subprocess.run(
                    [
                        "cc", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
                        "-I", str(ROOT / "include"), "-c", str(ROOT / source),
                        "-o", str(obj),
                    ],
                    check=True, capture_output=True, text=True,
                )
                objects.append(str(obj))

            program = temp / "unit-stat-composition-test"
            subprocess.run(
                [
                    "cc", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
                    "-I", str(ROOT / "include"),
                    str(ROOT / "tests/c/unit_stat_composition_harness.c"),
                    *objects, "-o", str(program),
                ],
                check=True, capture_output=True, text=True,
            )
            subprocess.run([str(program)], check=True, capture_output=True, text=True)


if __name__ == "__main__":
    unittest.main()
