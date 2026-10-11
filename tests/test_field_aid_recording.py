"""Patch isolation and frame accounting for battle evidence clips."""
import importlib.util
import json
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools/mods'))
sys.path.insert(0, str(ROOT / 'tools/emu'))
import field_aid
import clip_recording


class FieldAidTests(unittest.TestCase):
    def test_only_name_and_two_effect_slots_change(self):
        original = (ROOT / 'baserom.gba').read_bytes()
        spec = json.loads((ROOT / 'mods/field-aid.json').read_text())
        modified, report = field_aid.patch(original, spec)
        self.assertEqual(len(original), len(modified))
        self.assertEqual({int(x['offset'], 16) for x in report['changes']},
                         {0x5520D8, 0x5520DA, 0x554C51, 0x554C53, 0x554C55})
        self.assertEqual(modified[0x5520D8:0x5520DC], bytes([0x59, 0x26, 0xC3, 1]))
        spec['expected_record'] = '00' * 28
        with self.assertRaisesRegex(ValueError, 'record'):
            field_aid.patch(original, spec)


class RecordingTests(unittest.TestCase):
    def test_partial_steps_share_sampling_boundary(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            (root / 'frames').mkdir()
            active = root / 'active.json'
            active.write_text(json.dumps({'directory': d}))
            clip_recording.save_json(root / 'recording.json', {
                'emulated_frames': 0, 'pending_frames': 0, 'video_frames': 0, 'incomplete': False})
            steps = []
            def request(route, binary=False):
                if route.startswith('/step'):
                    steps.append(int(route.split('=')[-1]))
                    return 'ok'
                return b'\x89PNG\r\n\x1a\n' + bytes([sum(steps)])
            for count in [5, 1, 3]:
                clip_recording.advance(active, count, request)
            state = json.loads((root / 'recording.json').read_text())
            self.assertEqual(sum(steps), 9)
            self.assertEqual(state['emulated_frames'], 9)
            self.assertEqual(state['video_frames'], 4)
            self.assertEqual(state['pending_frames'], 1)
            self.assertEqual([p.read_bytes()[-1] for p in sorted((root / 'frames').iterdir())], [2, 4, 6, 8])

    def test_capture_failure_marks_footage_incomplete(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            (root / 'frames').mkdir()
            active = root / 'active.json'
            active.write_text(json.dumps({'directory': d}))
            clip_recording.save_json(root / 'recording.json', {
                'emulated_frames': 0, 'pending_frames': 0, 'video_frames': 0, 'incomplete': False})
            with self.assertRaisesRegex(RuntimeError, 'invalid screenshot'):
                clip_recording.advance(active, 2, lambda *a, **k: b'bad')
            state = json.loads((root / 'recording.json').read_text())
            self.assertTrue(state['incomplete'])
            self.assertEqual(state['emulated_frames'], 2)


if __name__ == '__main__':
    unittest.main()
