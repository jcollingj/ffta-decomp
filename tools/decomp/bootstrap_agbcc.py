#!/usr/bin/env python3
"""Build the pinned Thumb C compiler locally. No global tool installation."""
from pathlib import Path
import subprocess
import json

ROOT = Path(__file__).resolve().parents[2]
DEST = ROOT / 'tools/.deps/agbcc'
CONFIG = json.loads((ROOT / 'config/toolchain.json').read_text())
REVISION = CONFIG['revision']
URL = CONFIG['repository']


def main():
    DEST.parent.mkdir(parents=True, exist_ok=True)
    if not DEST.exists():
        subprocess.run(['git', 'clone', URL, str(DEST)], check=True)
        subprocess.run(['git', '-C', str(DEST), 'checkout', '--detach', REVISION], check=True)
    actual = subprocess.check_output(['git', '-C', str(DEST), 'rev-parse', 'HEAD'], text=True).strip()
    if actual != REVISION:
        raise SystemExit(f'Compiler checkout is {actual}; expected {REVISION}. Refusing to overwrite it.')
    # Upstream generated-header dependencies race under parallel make.
    subprocess.run(['make', '-C', str(DEST / 'gcc'), '-j1'], check=True)
    print(DEST / 'gcc/agbcc')


if __name__ == '__main__':
    main()
