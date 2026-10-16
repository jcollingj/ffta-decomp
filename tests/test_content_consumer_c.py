"""Host behavioral checks for the bounded generic content consumer."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class ContentConsumerCTests(unittest.TestCase):
    def test_fixed_whole_state_images_and_real_known_wrappers(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'content-consumer-test'
            subprocess.run([
                'cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                '-I', str(ROOT / 'include'),
                str(ROOT / 'tests/c/content_consumer_harness.c'),
                str(ROOT / 'src/content_consumer.c'),
                str(ROOT / 'src/content_globals.c'),
                str(ROOT / 'src/content_index.c'),
                str(ROOT / 'src/list_count_wrappers.c'),
                str(ROOT / 'src/list_count.c'),
                '-o', str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True,
                           text=True)
