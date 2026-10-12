#!/usr/bin/env python3
"""Create the ignored, pinned local environment for thumb_runner.py."""

from pathlib import Path
import subprocess
import venv


ROOT = Path(__file__).resolve().parents[2]
ENV = ROOT / "tools/.deps/thumb-runner"
REQUIREMENTS = ROOT / "tools/decomp/requirements-thumb-runner.txt"


def main() -> int:
    if not (ENV / "pyvenv.cfg").exists():
        venv.EnvBuilder(with_pip=True).create(ENV)
    python = ENV / "bin/python"
    subprocess.run([str(python), "-m", "pip", "install", "--disable-pip-version-check",
                    "-r", str(REQUIREMENTS)], check=True)
    subprocess.run([str(python), "-c",
                    "import unicorn; print('Unicorn', unicorn.__version__)"], check=True)
    print(f"Runner environment ready: {python}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
