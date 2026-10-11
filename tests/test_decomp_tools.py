import importlib.util
import contextlib
import hashlib
import io
import json
import sys
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]

def load(name):
    spec = importlib.util.spec_from_file_location(name, ROOT / 'tools/decomp' / (name + '.py'))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module

compare = load('compare')
xrefs = load('xrefs')
with patch.dict(sys.modules, {'compare': compare}):
    probe_report = load('probe_report')


class ProbeReportTests(unittest.TestCase):
    def references(self):
        return {'Example': dict(name='Example', start=2, end=6)}

    def test_shifted_identical_bytes_never_count_as_matching(self):
        report = probe_report.compare_functions(b'xxabcdyy', b'00abcd', 2,
            self.references(), {'Example': (4, 4)})[0]
        self.assertTrue(report['matches'])
        self.assertFalse(report['layout_matches'])
        self.assertEqual(report['status'], 'DIFF')
        self.assertEqual(report['address_delta'], 2)

    def test_exact_symbol_bytes_and_layout_match(self):
        report = probe_report.compare_functions(b'xxabcdyy', b'abcd', 2,
            self.references(), {'Example': (2, 4)})[0]
        self.assertEqual(report['status'], 'MATCH')

    def test_symbols_outside_candidate_are_rejected(self):
        for symbol in [(0, 4), (2, 5), (6, 1)]:
            with self.subTest(symbol=symbol), self.assertRaisesRegex(ValueError, 'outside emitted'):
                probe_report.compare_functions(b'xxabcdyy', b'abcd', 2,
                    self.references(), {'Example': symbol})

    def test_unknown_symbols_are_not_claimed_as_recovered(self):
        result = probe_report.compare_functions(b'xxabcdyy', b'abcd', 2, {}, {'Other': (2, 4)})
        self.assertEqual(result[0]['status'], 'UNINVENTORIED')

    def test_reference_conflicts_and_invalid_bounds_fail(self):
        reference = self.references()['Example']
        with self.assertRaisesRegex(ValueError, 'conflicting'):
            probe_report.merge_references([reference], [dict(reference, end=7)])
        with self.assertRaisesRegex(ValueError, 'invalid reference'):
            probe_report.compare_functions(b'xx', b'abcd', 2, self.references(), {'Example': (2, 4)})

    def test_nm_parser_filters_data_and_normalizes_thumb_bit(self):
        symbols = probe_report.text_symbols('08000003 00000004 T Example\n'
            '08000010 00000004 D Data\n08000020 00000000 t Empty\n U Missing\n')
        self.assertEqual(symbols, {'Example': (2, 4)})

class DecompToolsTests(unittest.TestCase):
    def test_thumb_bl_forward_backward_and_non_call(self):
        # Verified ROM instructions in SelectUnitAbilityList and a synthetic +0 call.
        self.assertEqual(xrefs.thumb_bl_target(0xcce92, 0xf7fc, 0xfa2d), 0x080c92f0)
        self.assertEqual(xrefs.thumb_bl_target(0, 0xf000, 0xf800), 0x08000004)
        self.assertIsNone(xrefs.thumb_bl_target(0, 0xf000, 0xe800))
        self.assertIsNone(xrefs.thumb_bl_target(0, 0x0000, 0xf800))

    def test_comparison_reports_absolute_offsets_and_size_mismatch(self):
        report = compare.compare_bytes(b'abc', b'axcd', 0xccd50)
        self.assertFalse(report['matches'])
        self.assertEqual(report['differing_bytes'], 2)
        self.assertEqual(report['first_differences'][0]['offset'], '0x0CCD51')
        self.assertEqual(report['actual_size'], 4)

    def test_identical_bytes_match(self):
        self.assertTrue(compare.compare_bytes(b'abc', b'abc', 0)['matches'])


class FunctionComparisonGateTests(unittest.TestCase):
    """Exercise the real command/report path with tiny disposable ROM fixtures."""

    def setUp(self):
        folder = tempfile.TemporaryDirectory()
        self.addCleanup(folder.cleanup)
        self.root = Path(folder.name)
        (self.root / 'config').mkdir()
        (self.root / 'build').mkdir()
        self.base = b'01234567'
        (self.root / 'baserom.gba').write_bytes(self.base)
        (self.root / 'build/ffta.gba').write_bytes(self.base)
        manifest = dict(rom_sha1=hashlib.sha1(self.base).hexdigest(), functions=[
            dict(name='Example', start=2, end=6, mode='thumb')])
        (self.root / 'config/decomp_regions.json').write_text(json.dumps(manifest))

    def run_gate(self, nm='08000003 00000004 T Example\n'):
        with patch.object(compare, 'ROOT', self.root), \
                patch('sys.argv', ['compare.py', '--no-build']), \
                patch.object(compare.subprocess, 'check_output', return_value=nm), \
                contextlib.redirect_stdout(io.StringIO()):
            compare.main()

    def report(self):
        return json.loads((self.root / 'build/function-comparison.json').read_text())['functions'][0]

    def test_exact_thumb_function_passes(self):
        self.run_gate()
        self.assertTrue(self.report()['matches'])
        self.assertTrue(self.report()['layout_matches'])

    def test_identical_bytes_with_wrong_or_missing_symbol_fail(self):
        for symbols in ['', '08000005 00000004 T Example\n',
                        '08000003 00000002 T Example\n']:
            with self.subTest(symbols=symbols), self.assertRaises(SystemExit) as error:
                self.run_gate(symbols)
            self.assertEqual(error.exception.code, 1)
            self.assertTrue(self.report()['matches'])
            self.assertFalse(self.report()['layout_matches'])

    def test_corrupted_function_fails_with_evidence(self):
        (self.root / 'build/ffta.gba').write_bytes(b'012X4567')
        with self.assertRaises(SystemExit) as error:
            self.run_gate()
        self.assertEqual(error.exception.code, 1)
        self.assertEqual(self.report()['first_differences'], [
            dict(offset='0x000003', expected=ord('3'), actual=ord('X'))])

    def test_truncated_output_fails(self):
        (self.root / 'build/ffta.gba').write_bytes(self.base[:4])
        with self.assertRaises(SystemExit) as error:
            self.run_gate()
        self.assertEqual(error.exception.code, 1)
        self.assertEqual(self.report()['actual_size'], 2)

    def test_wrong_reference_rom_fails_before_comparison(self):
        (self.root / 'baserom.gba').write_bytes(b'wrongrom')
        with self.assertRaisesRegex(SystemExit, 'wrong base ROM'):
            self.run_gate()
        self.assertFalse((self.root / 'build/function-comparison.json').exists())
