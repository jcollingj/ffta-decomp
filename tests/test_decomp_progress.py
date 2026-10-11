import copy
import hashlib
import importlib.util
from pathlib import Path
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('progress', ROOT / 'tools/decomp/progress.py')
progress = importlib.util.module_from_spec(spec)
spec.loader.exec_module(progress)


class ProgressTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        (self.root / 'asm').mkdir()
        (self.root / 'asm/test.s').write_text('.incbin "baserom.gba", 0, 6\n.byte 0, 0\n')
        (self.root / 'Makefile').write_text('OBJS := build/test.o\n')
        self.rom = self.root / 'rom.gba'
        self.rom.write_bytes(bytes(8))
        self.manifest = dict(rom_size=8, rom_sha1=hashlib.sha1(bytes(8)).hexdigest(),
            code_inventory_complete=False, functions=[], regions=[
                dict(start=0, end=6, kind='copied', source='asm/test.s'),
                dict(start=6, end=8, kind='source_data', source='asm/test.s')])

    def test_unknown_code_denominator_not_reported_as_rom_percent(self):
        result = progress.audit(self.manifest, self.root, self.rom)
        self.assertEqual(result['source_or_generated_percent'], 25)
        self.assertIsNone(result['code_completion_percent'])
        self.assertIsNone(result['total_functions'])

    def test_gap_and_overlap_rejected(self):
        for start in [5, 7]:
            manifest = copy.deepcopy(self.manifest)
            manifest['regions'][1]['start'] = start
            with self.assertRaises(ValueError):
                progress.measure(manifest)

    def test_manifest_cannot_claim_copied_bytes_as_source(self):
        self.manifest['regions'][0]['end'] = 5
        self.manifest['regions'][1]['start'] = 5
        with self.assertRaisesRegex(ValueError, 'incbin'):
            progress.audit(self.manifest, self.root, self.rom)

    def test_modified_rom_rejected(self):
        self.rom.write_bytes(b'changed!')
        with self.assertRaisesRegex(ValueError, 'matching'):
            progress.audit(self.manifest, self.root, self.rom)

    def test_function_must_be_in_recovered_code_region(self):
        self.manifest['functions'] = [dict(start=0, end=4, status='matching_c')]
        with self.assertRaisesRegex(ValueError, 'decompiled C'):
            progress.measure(self.manifest)
