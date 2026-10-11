"""Frame-stepped, silent video recording for fftactl. One controller per port."""
import json
from pathlib import Path


def save_json(path, value):
    temp = path.with_suffix('.tmp')
    temp.write_text(json.dumps(value, indent=2) + '\n')
    temp.replace(path)


def capture(root, state, request):
    data = request('/screen', binary=True)
    if not data.startswith(b'\x89PNG\r\n\x1a\n'):
        raise RuntimeError('emulator returned an invalid screenshot')
    (root / 'frames' / f'{state["video_frames"]:08d}.png').write_bytes(data)
    state['video_frames'] += 1


def advance(active, count, request):
    root = Path(json.loads(active.read_text())['directory'])
    path = root / 'recording.json'
    state = json.loads(path.read_text())
    if state['incomplete']:
        raise RuntimeError('recording is incomplete; use clip abort before advancing again')
    result = None
    try:
        while count:
            n = min(count, 2 - state['pending_frames'])
            result = request(f'/step?frames={n}')
            state['emulated_frames'] += n
            state['pending_frames'] += n
            count -= n
            if state['pending_frames'] == 2:
                capture(root, state, request)
                state['pending_frames'] = 0
        return result
    except Exception:
        state['incomplete'] = True
        raise
    finally:
        save_json(path, state)
