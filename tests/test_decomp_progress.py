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

    def test_explicit_object_lists_accept_single_line_and_continuations(self):
        expected = ['build/first.o', 'build/second-file.o', 'build/third.o']
        self.assertEqual(progress.parse_build_objects(
            'OBJS := build/first.o build/second-file.o build/third.o\n'), expected)
        self.assertEqual(progress.parse_build_objects(
            'OBJS    := \\\n\tbuild/first.o \\\n\tbuild/second-file.o \\\n\tbuild/third.o\n'), expected)

    def test_explicit_object_list_rejects_unsupported_tokens_and_duplicates(self):
        for makefile in (
                'OBJS := build/test.o $(EXTRA_OBJECTS)\n',
                'OBJS := build/test.o build/test.o\n',
                'OBJS := build/test.o; touch bad\n',
                'OBJS := build/test.o\nOBJS += build/other.o\n',
                'OBJS := build/test.o \\\n\tUNTERMINATED\n'):
            with self.subTest(makefile=makefile), self.assertRaises(ValueError):
                progress.parse_build_objects(makefile)

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
        self.manifest['functions'] = [dict(
            start=0, end=4, status='matching_c', source='src/recovered.c')]
        with self.assertRaisesRegex(ValueError, 'decompiled C'):
            progress.measure(self.manifest)

    def test_decompiled_c_region_requires_a_matching_function(self):
        self.manifest['regions'][1] = dict(
            start=6, end=8, kind='decompiled_c', source='src/recovered.c')
        with self.assertRaisesRegex(ValueError, 'at least one same-source matching C function'):
            progress.measure(self.manifest)

    def test_matching_function_and_region_require_sources(self):
        self.manifest['regions'][1] = dict(start=6, end=8, kind='decompiled_c')
        with self.assertRaisesRegex(ValueError, 'region .* must declare a source'):
            progress.measure(self.manifest)

        self.manifest['regions'][1]['source'] = 'src/recovered.c'
        self.manifest['functions'] = [dict(start=6, end=8, status='matching_c')]
        with self.assertRaisesRegex(ValueError, 'function .* must declare a source'):
            progress.measure(self.manifest)

    def test_matching_function_must_match_region_source(self):
        self.manifest['regions'][1] = dict(
            start=6, end=8, kind='decompiled_c', source='src/recovered.c')
        self.manifest['functions'] = [dict(
            start=6, end=8, status='matching_c', source='src/other.c')]
        with self.assertRaisesRegex(ValueError, 'does not match decompiled C region source'):
            progress.measure(self.manifest)

    def test_matching_function_must_be_wholly_within_c_region(self):
        self.manifest['regions'][1] = dict(
            start=6, end=8, kind='decompiled_c', source='src/recovered.c')
        self.manifest['functions'] = [dict(
            start=5, end=7, status='matching_c', source='src/recovered.c')]
        with self.assertRaisesRegex(ValueError, 'must be wholly within'):
            progress.measure(self.manifest)

    def test_multiple_functions_may_share_region_with_unattributed_padding(self):
        self.manifest['rom_size'] = 12
        self.manifest['regions'][1] = dict(
            start=6, end=12, kind='decompiled_c', source='src/recovered.c')
        self.manifest['functions'] = [
            dict(start=6, end=8, status='matching_c', source='src/recovered.c'),
            dict(start=10, end=12, status='matching_c', source='src/recovered.c'),
        ]
        result = progress.measure(self.manifest)
        self.assertEqual(result['bytes_by_kind']['decompiled_c'], 6)
        self.assertEqual(result['matching_c_functions'], 2)
        self.assertIsNone(result['total_functions'])
        self.assertIsNone(result['code_completion_percent'])
