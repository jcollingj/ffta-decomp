"""Full real-chain host composition checks for the content consumer."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which("cc"), "requires a host C compiler")
class ContentConsumerCompositionTests(unittest.TestCase):
    def test_full_real_chain_composition(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / "content-consumer-composition-test"
            subprocess.run([
                "cc", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
                "-Wno-pointer-to-int-cast", "-I", str(ROOT / "include"),
                str(ROOT / "tests/c/content_consumer_composition_harness.c"),
                str(ROOT / "src/content_consumer.c"),
                str(ROOT / "src/content_globals.c"),
                str(ROOT / "src/content_fill.c"),
                str(ROOT / "src/content_append.c"),
                str(ROOT / "src/content_payload_callback.c"),
                str(ROOT / "src/list_visit.c"),
                str(ROOT / "src/list_accessors.c"),
                str(ROOT / "src/content_selection_lookup.c"),
                str(ROOT / "src/content_selection_predicate.c"),
                str(ROOT / "src/content_find_flagged.c"),
                str(ROOT / "src/content_context_visit.c"),
                str(ROOT / "src/content_unit_select.c"),
                str(ROOT / "src/unit_fields.c"),
                str(ROOT / "src/list_count_wrappers.c"),
                str(ROOT / "src/list_count.c"),
                str(ROOT / "src/content_index.c"),
                "-o", str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True,
                           text=True)


if __name__ == "__main__":
    unittest.main()
