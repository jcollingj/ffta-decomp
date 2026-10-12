import hashlib
import importlib.util
import io
import json
import contextlib
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location('task_packet', ROOT / 'tools/decomp/task_packet.py')
task_packet = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(task_packet)


class TaskPacketTests(unittest.TestCase):
    def setUp(self):
        self.rom = bytes(range(16))
        self.sha1 = hashlib.sha1(self.rom).hexdigest()
        self.matching = {'rom_size': len(self.rom), 'rom_sha1': self.sha1,
                         'functions': []}
        self.candidates = {'functions': [
            {'name': 'Example', 'start': 4, 'end': 10, 'mode': 'thumb',
             'source': 'src/example.c'}]}

    def packet(self, name='Example'):
        return task_packet.build_packet(name, self.matching, self.candidates,
                                        self.rom, self.sha1, len(self.rom))

    def test_candidate_packet_has_exact_decimal_hex_hash_and_probe(self):
        packet = self.packet()
        self.assertEqual(packet['classification'], 'research candidate')
        self.assertEqual(packet['source_status'], 'known')
        self.assertEqual((packet['start_decimal'], packet['end_decimal']), (4, 10))
        self.assertEqual((packet['start_hex'], packet['end_hex']), ('0x4', '0xA'))
        self.assertEqual(packet['interval_sha256'], hashlib.sha256(self.rom[4:10]).hexdigest())
        self.assertEqual(packet['probe_command'],
                         'python3 tools/decomp/probe.py src/example.c --start 0x4 --end 0xA')
        self.assertTrue(any('literal pool or padding' in note for note in packet['caveats']))
        self.assertTrue(any('control-flow back-edges' in note for note in packet['caveats']))
        self.assertTrue(any('unmanifested functions' in note for note in packet['caveats']))

    def test_matching_entry_is_classified_as_admitted_and_overrides_identical_candidate(self):
        matching_entry = {'name': 'Example', 'start': 4, 'end': 10,
                          'mode': 'thumb', 'status': 'matching_c', 'source': 'src/example.c'}
        self.matching['functions'].append(matching_entry)
        packet = self.packet()
        self.assertEqual(packet['classification'], 'admitted matching function')
        self.assertEqual(packet['source'], 'src/example.c')
        self.assertIsNone(packet['probe_command'])
        self.assertEqual(packet['comparison_command'], 'python3 tools/decomp/compare.py Example')

    def test_conflicting_same_name_bounds_fail(self):
        self.matching['functions'].append({'name': 'Example', 'start': 2, 'end': 10,
                                           'mode': 'thumb', 'status': 'matching_c'})
        with self.assertRaisesRegex(ValueError, 'conflicting reference bounds'):
            self.packet()

    def test_unknown_function_fails(self):
        with self.assertRaisesRegex(ValueError, 'unknown function'):
            self.packet('Missing')

    def test_multiple_functions_in_one_source_suppress_misplaced_probe(self):
        self.candidates['functions'].append(
            {'name': 'Second', 'start': 10, 'end': 14, 'mode': 'thumb',
             'source': 'src/example.c'})
        packet = self.packet()
        self.assertIsNone(packet['probe_command'])
        self.assertEqual(packet['comparison_command'], None)
        self.assertIn('Multiple manifest functions share', packet['command_note'])

    def test_wrong_full_rom_identity_fails(self):
        with self.assertRaisesRegex(ValueError, 'wrong base ROM SHA-1'):
            task_packet.build_packet('Example', self.matching, self.candidates,
                                     self.rom, '0' * 40, len(self.rom))

    def test_invalid_or_unaligned_thumb_bounds_fail(self):
        for start, end in ((4, 4), (4, 18), (5, 10), (4, 9)):
            self.candidates['functions'][0].update(start=start, end=end)
            with self.subTest(start=start, end=end), self.assertRaises(ValueError):
                self.packet()

    def test_known_content_pointer_packet_uses_manifest_hex_without_manual_conversion(self):
        matching = {'functions': []}
        candidate = {'functions': [{'name': 'GetRemappedContentPointer',
                                    'start': 832736, 'end': 832804, 'mode': 'thumb',
                                    'source': 'src/nonmatching/content_pointers.c'}]}
        rom = bytes(0x1000000)
        sha1 = hashlib.sha1(rom).hexdigest()
        packet = task_packet.build_packet('GetRemappedContentPointer', matching, candidate,
                                          rom, sha1, len(rom))
        self.assertEqual((packet['start_decimal'], packet['end_decimal']), (832736, 832804))
        self.assertEqual((packet['start_hex'], packet['end_hex']), ('0xCB4E0', '0xCB524'))
        self.assertEqual(packet['interval_size'], 68)
        self.assertEqual(packet['probe_command'],
                         'python3 tools/decomp/probe.py src/nonmatching/content_pointers.c --start 0xCB4E0 --end 0xCB524')

    def test_disassembly_uses_safe_argument_list_and_missing_tool_is_clear(self):
        packet = self.packet()
        with patch.object(task_packet.subprocess, 'run') as run, \
                patch.object(task_packet.sys, 'stdout', io.StringIO()):
            task_packet.disassemble_original(packet, root='/fixture')
        command = run.call_args.args[0]
        self.assertEqual(command[0], 'arm-none-eabi-objdump')
        self.assertIn('--start-address=4', command)
        self.assertIn('--stop-address=10', command)
        self.assertNotIn('shell', run.call_args.kwargs)

        with patch.object(task_packet.subprocess, 'run', side_effect=FileNotFoundError), \
                patch.object(task_packet.sys, 'stdout', io.StringIO()):
            with self.assertRaisesRegex(RuntimeError, 'arm-none-eabi-objdump is required'):
                task_packet.disassemble_original(packet, root='/fixture')

    def test_cli_json_mode_loads_repository_manifests_and_rom(self):
        # Keep this test hermetic by supplying all inputs through a tiny temp repo.
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'config').mkdir()
            (root / 'baserom.gba').write_bytes(self.rom)
            (root / 'config/decomp_regions.json').write_text(
                json.dumps({'rom_size': len(self.rom), 'rom_sha1': self.sha1,
                            'functions': self.matching['functions']}))
            (root / 'config/decomp_candidates.json').write_text(json.dumps(self.candidates))
            with patch.object(task_packet, 'EXPECTED_ROM_SHA1', self.sha1), \
                    patch.object(task_packet, 'EXPECTED_ROM_SIZE', len(self.rom)):
                packet = task_packet.load_packet('Example', root=root)
            self.assertEqual(packet['function'], 'Example')
            self.assertEqual(packet['rom_sha1'], self.sha1)

    def test_cli_json_is_valid_and_mutually_exclusive_with_disassembly(self):
        packet = self.packet()
        output = io.StringIO()
        with patch.object(task_packet, 'load_packet', return_value=packet), \
                patch.object(task_packet, 'disassemble_original') as disassemble, \
                contextlib.redirect_stdout(output):
            task_packet.main(['Example', '--json'])
        self.assertEqual(json.loads(output.getvalue()), packet)
        disassemble.assert_not_called()

        with contextlib.redirect_stderr(io.StringIO()):
            with self.assertRaises(SystemExit) as error:
                task_packet.main(['Example', '--json', '--disassemble'])
        self.assertEqual(error.exception.code, 2)


if __name__ == '__main__':
    unittest.main()
