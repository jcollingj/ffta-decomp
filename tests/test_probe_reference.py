import importlib.util
import hashlib
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import Mock, patch

ROOT = Path(__file__).resolve().parents[1]


def load(name):
    spec = importlib.util.spec_from_file_location(
        name, ROOT / 'tools/decomp' / (name + '.py'))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


probe_reference = load('probe_reference')
compare = load('compare')
with patch.dict(sys.modules, {'compare': compare}):
    probe_report = load('probe_report')


class ProbeReferenceTests(unittest.TestCase):
    def test_no_undefined_symbols_needs_no_elf_read_or_readelf(self):
        with tempfile.TemporaryDirectory() as temporary:
            missing = Path(temporary) / 'does-not-exist.elf'
            readelf = Mock(side_effect=AssertionError('readelf must not run'))
            symbols, identity = probe_reference.load_reference_symbols(
                [], missing, Path(temporary) / 'probe', check_output=readelf)
        self.assertEqual(symbols, {})
        self.assertIsNone(identity)
        readelf.assert_not_called()

    def test_symbol_parser_preserves_arm_thumb_function_types_and_skips_data(self):
        output = (
            'Symbol table \' .symtab\' contains entries:\n'
            '   Num:    Value  Size Type    Bind   Vis      Ndx Name\n'
            '     1: 08000001    4 FUNC    GLOBAL DEFAULT    1 ThumbCall\n'
            '     2: 08000100    4 FUNC    GLOBAL DEFAULT    1 ArmCall\n'
            '     3: 08000200    4 OBJECT  GLOBAL DEFAULT    2 DataObject\n'
            '     4: 00000000    0 NOTYPE  GLOBAL DEFAULT  UND Undefined\n')
        parsed = probe_reference.parse_readelf_symbols(output)
        self.assertEqual(parsed, {
            'ThumbCall': (0x08000001, 'FUNC'),
            'ArmCall': (0x08000100, 'FUNC'),
            'DataObject': (0x08000200, 'OBJECT'),
        })
        # These values/types are consumed by probe.py's existing typed alias
        # emitter: odd FUNC remains Thumb; even FUNC remains ARM.
        self.assertEqual(parsed['ThumbCall'][0] & 1, 1)
        self.assertEqual(parsed['ArmCall'][0] & 1, 0)
        self.assertEqual(parsed['ThumbCall'][1], 'FUNC')
        self.assertEqual(parsed['DataObject'][1], 'OBJECT')

    def test_reference_is_snapshotted_before_symbol_parse_and_hashed(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            selected = root / 'build' / 'ffta.elf'
            selected.parent.mkdir()
            original = b'\x7fELFimmutable reference bytes'
            selected.write_bytes(original)
            probe_dir = root / 'probe'

            def fake_readelf(command, text):
                self.assertEqual(command[:2], ['arm-none-eabi-readelf', '-sW'])
                snapshot = Path(command[2])
                self.assertEqual(snapshot.parent, probe_dir)
                self.assertEqual(snapshot.read_bytes(), original)
                selected.write_bytes(b'\x7fELFchanged after snapshot')
                return '     1: 08000001    4 FUNC GLOBAL DEFAULT 1 Call\n'

            symbols, identity = probe_reference.load_reference_symbols(
                ['Call'], selected, probe_dir, check_output=fake_readelf)
            self.assertEqual(symbols, {'Call': (0x08000001, 'FUNC')})
            self.assertEqual(identity['selected_reference_elf'], str(selected.resolve()))
            self.assertEqual(Path(identity['reference_elf_snapshot']).read_bytes(), original)
            self.assertEqual(identity['reference_elf_sha256'], hashlib.sha256(original).hexdigest())

    def test_missing_and_invalid_reference_are_clear_errors(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            with self.assertRaisesRegex(probe_reference.ReferenceElfError, 'cannot read reference ELF'):
                probe_reference.load_reference_symbols(
                    ['Missing'], root / 'missing.elf', root / 'probe')
            invalid = root / 'invalid.elf'
            invalid.write_bytes(b'not an ELF')
            with self.assertRaisesRegex(probe_reference.ReferenceElfError, 'invalid reference ELF'):
                probe_reference.load_reference_symbols(
                    ['Missing'], invalid, root / 'probe')

    def test_readelf_invocation_uses_argument_list_and_snapshot(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            selected = root / 'reference.elf'
            selected.write_bytes(b'\x7fELFfixture')
            runner = Mock(return_value='')
            _, identity = probe_reference.load_reference_symbols(
                ['External'], selected, root / 'probe', check_output=runner)
        runner.assert_called_once()
        command = runner.call_args.args[0]
        self.assertIsInstance(command, list)
        self.assertEqual(command[:2], ['arm-none-eabi-readelf', '-sW'])
        self.assertEqual(str(Path(command[2]).resolve()), identity['reference_elf_snapshot'])
        self.assertTrue(command[2].endswith('/reference.elf'))

    def test_strict_byte_and_symbol_layout_acceptance_remains_required(self):
        refs = {'Example': dict(name='Example', start=2, end=6)}
        exact = probe_report.compare_functions(
            b'xxabcdyy', b'abcd', 2, refs, {'Example': (2, 4)})[0]
        shifted = probe_report.compare_functions(
            b'xxabcdyy', b'00abcd', 2, refs, {'Example': (4, 4)})[0]
        changed = probe_report.compare_functions(
            b'xxabcdyy', b'abXd', 2, refs, {'Example': (2, 4)})[0]
        self.assertEqual(exact['status'], 'MATCH')
        self.assertTrue(exact['matches'])
        self.assertTrue(exact['layout_matches'])
        self.assertEqual(shifted['status'], 'DIFF')
        self.assertTrue(shifted['matches'])
        self.assertFalse(shifted['layout_matches'])
        self.assertEqual(changed['status'], 'DIFF')
        self.assertFalse(changed['matches'])


if __name__ == '__main__':
    unittest.main()
