"""Regression checks for ROM boundaries and paused gameplay control."""
import contextlib
import importlib.machinery
import importlib.util
import io
import json
from pathlib import Path
import tempfile
import urllib.parse
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]


def module(name, path):
    loader = importlib.machinery.SourceFileLoader(name, str(ROOT / path))
    spec = importlib.util.spec_from_loader(name, loader)
    result = importlib.util.module_from_spec(spec)
    loader.exec_module(result)
    return result


ctl = module("fftactl", "tools/emu/fftactl")
rename = module("rename", "tools/mods/rename_ability.py")
baseline = module("baseline", "tools/emu/baseline")


class ControlTests(unittest.TestCase):
    def test_refuses_to_boot_protected_baseline(self):
        with patch("sys.argv", ["fftactl", "start", str(ROOT / "checkpoints/baselines/test/ffta.gba")]):
            with self.assertRaisesRegex(ValueError, "protected baseline"):
                ctl.main()

    def test_observation_does_not_resume(self):
        with tempfile.TemporaryDirectory() as d:
            with patch.object(ctl, "get", return_value=b"PNG") as get:
                with patch("sys.argv", ["fftactl", "screen", str(Path(d) / "screen.png")]):
                    with contextlib.redirect_stdout(io.StringIO()):
                        self.assertEqual(ctl.main(), 0)
            get.assert_called_once_with("/screen", binary=True)

    def test_input_released_if_step_fails(self):
        with patch.object(ctl, "get", side_effect=["ok", TimeoutError(), "ok"]) as get:
            with self.assertRaises(TimeoutError):
                ctl.press("A", 5)
            self.assertEqual(get.call_args_list[-1].args, ("/input?A=0",))

    def test_mash_obeys_partial_cycle_budget(self):
        with patch.object(ctl, "get", return_value="ok") as get:
            with patch("sys.argv", ["fftactl", "mash", "A", "31"]):
                with contextlib.redirect_stdout(io.StringIO()):
                    ctl.main()
            calls = [c.args[0] for c in get.call_args_list]
            self.assertEqual(sum(int(c.split("=")[-1]) for c in calls if c.startswith("/step")), 31)
            self.assertEqual(calls[-1], "/input?A=0")


class BaselineTests(unittest.TestCase):
    def test_capture_duplicates_and_protects_artifacts(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            (root / "ffta.gba").write_bytes(b"test ROM")
            (root / "ffta.sav").write_bytes(b"test battery save")
            status = {"run-mode": "PAUSE", "rom-loaded": True, "inputs": {"A": 0},
                      "rom-path": str(root / "ffta.gba"), "save-path": str(root / "ffta.sav"),
                      "emulator": "test emulator"}

            def get(route):
                if route == "status":
                    return json.dumps(status).encode()
                path = urllib.parse.parse_qs(route.split("?", 1)[1])["path"][0]
                Path(path).write_bytes(b"test checkpoint")
                return b"ok"

            with patch.object(baseline, "REPO", root), patch.object(baseline, "get", side_effect=get):
                with contextlib.redirect_stdout(io.StringIO()):
                    baseline.capture("test", "fixture")
                primary = root / "checkpoints/baselines/test"
                backup = root / "backups/baselines/test"
                self.assertEqual(baseline.verify(primary), baseline.verify(backup))
                self.assertEqual((primary / "ffta.sav").stat().st_mode & 0o222, 0)
                with self.assertRaises(ValueError):
                    baseline.capture("test", "refuse overwrite")
                (root / "ffta.sav").unlink()
                with self.assertRaises(ValueError):
                    baseline.capture("missing-save", "must fail")


@unittest.skipUnless((ROOT / "baserom.gba").exists(), "requires supplied USA ROM")
class AbilityTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data = (ROOT / "baserom.gba").read_bytes()
        cls.spec = json.loads((ROOT / "mods/ability-name.json").read_text())

    def test_patch_changes_only_three_expected_bytes(self):
        output, report = rename.patch(self.data, self.spec)
        changed = [i for i, (a, b) in enumerate(zip(self.data, output)) if a != b]
        self.assertEqual(changed, [0x554C51, 0x554C53, 0x554C55])
        self.assertEqual(len(output), len(self.data))
        self.assertEqual(report["changed_bytes"], 3)

    def test_round_trip_original_name_is_identical(self):
        spec = dict(self.spec, replacement_name="First Aid")
        output, _ = rename.patch(self.data, spec)
        self.assertEqual(output, self.data)

    def test_refuses_wrong_input_expected_name_or_length(self):
        with self.assertRaises(ValueError):
            rename.patch(b"wrong ROM", self.spec)
        for spec in (dict(self.spec, expected_name="Wrong Aid"),
                     dict(self.spec, replacement_name="Aid"),
                     dict(self.spec, string_id=398)):
            with self.assertRaises(ValueError):
                rename.patch(self.data, spec)


if __name__ == "__main__":
    unittest.main()
