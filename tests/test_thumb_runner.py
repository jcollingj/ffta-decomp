"""Synthetic ARM Thumb differential runner tests (no private ROM required)."""

from dataclasses import replace
import hashlib
import io
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

from tools.decomp.thumb_runner import (
    EWRAM_BASE,
    ExecutionTimedOut,
    EXPECTED_USA_SHA1,
    InstructionBudgetExceeded,
    MemoryRegion,
    RunnerError,
    RunSpec,
    WatchRange,
    _load_spec,
    main,
    run_differential,
)


try:
    import unicorn  # noqa: F401
    HAS_UNICORN = True
except ImportError:
    HAS_UNICORN = False


@unittest.skipUnless(HAS_UNICORN, "run with tools/.deps/thumb-runner/bin/python")
class ThumbRunnerTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.rom_path = Path(self.temp.name) / "synthetic.gba"
        self.rom = bytearray(0x1000)
        self.rom_path.write_bytes(self.rom)

    def spec(self, original: bytes, candidate: bytes, *, args=(), budget=100, timeout_us=250000,
             watch=True, region=True):
        self.rom[0x100:0x100 + len(original)] = original
        self.rom_path.write_bytes(self.rom)
        return RunSpec(
            rom_path=self.rom_path,
            entry_offset=0x100,
            overlay_offset=0x100,
            candidate_bytes=candidate,
            args=tuple(args),
            ram_regions=((MemoryRegion(EWRAM_BASE, 0x1000, b"\xAA" * 0x1000),)
                         if region else ()),
            instruction_budget=budget,
            timeout_us=timeout_us,
            watch_ranges=((WatchRange(EWRAM_BASE + 0x100, 4, "result"),)
                          if watch else ()),
            compare_registers=("r0", "r1", "r2", "r3"),
            expected_sha1=hashlib.sha1(self.rom).hexdigest(),
        )

    def test_register_and_memory_diff_with_independent_runs(self):
        # movs r0,#2; adds r0,#1; str r0,[r1]; bx lr
        original = bytes.fromhex("02 20 01 30 08 60 70 47")
        # Same instruction shape with a distinguishing value of four.
        candidate = bytes.fromhex("04 20 01 30 08 60 70 47")
        spec = self.spec(original, candidate, args=(0, EWRAM_BASE + 0x100))
        before_hash = hashlib.sha1(self.rom_path.read_bytes()).hexdigest()
        report = run_differential(spec)
        self.assertEqual(report["status"], "DIFF")
        self.assertEqual(report["original"]["registers"]["r0"], 3)
        self.assertEqual(report["candidate"]["registers"]["r0"], 5)
        self.assertEqual(report["original"]["watched_memory_hex"]["result"], "03000000")
        self.assertEqual(report["candidate"]["watched_memory_hex"]["result"], "05000000")
        self.assertEqual(len(report["differences"]), 2)
        self.assertFalse(report["recovery_credit"])
        self.assertEqual(hashlib.sha1(self.rom_path.read_bytes()).hexdigest(), before_hash)

    def test_identical_code_matches_and_returns_to_sentinel(self):
        code = bytes.fromhex("01 20 70 47")
        spec = self.spec(code, code)
        spec = replace(spec, compare_registers=("r0", "r4"))
        report = run_differential(spec)
        self.assertEqual(report["status"], "MATCH")
        self.assertEqual(report["original"]["return_pc"], "0x0fff0000")
        self.assertEqual(report["candidate"]["return_pc"], "0x0fff0000")
        self.assertEqual(report["original"]["registers"]["r4"], 0xA5A50004)

    def test_instruction_budget_is_enforced(self):
        # b . (self-loop)
        code = bytes.fromhex("fe e7")
        with self.assertRaisesRegex(InstructionBudgetExceeded, "budget=7"):
            run_differential(self.spec(code, code, budget=7, watch=False))

    def test_wall_clock_timeout_is_enforced(self):
        code = bytes.fromhex("fe e7")
        with self.assertRaisesRegex(ExecutionTimedOut, "timed out"):
            run_differential(self.spec(code, code, budget=100000, watch=False,
                                       timeout_us=1))

    def test_unmapped_write_is_rejected(self):
        # str r0,[r1]; bx lr, with r1 outside every explicit RAM map.
        code = bytes.fromhex("08 60 70 47")
        with self.assertRaisesRegex(RunnerError, "UC_ERR_WRITE_UNMAPPED"):
            run_differential(self.spec(code, code,
                                       args=(1, 0x05000000), watch=False,
                                       region=False))

    def test_invalid_thumb_instruction_is_rejected(self):
        code = bytes.fromhex("00 de 70 47")  # UDF #0
        with self.assertRaisesRegex(RunnerError, "execution failed"):
            run_differential(self.spec(code, code, watch=False))

    def test_software_interrupt_is_rejected_without_bios_emulation(self):
        code = bytes.fromhex("00 df 70 47")  # SWI #0
        with self.assertRaisesRegex(RunnerError, "uses SWI without BIOS emulation"):
            run_differential(self.spec(code, code, watch=False))

    def test_rom_hash_and_overlay_bounds_are_checked(self):
        code = bytes.fromhex("70 47")
        spec = self.spec(code, code)
        with self.assertRaisesRegex(RunnerError, "SHA-1 mismatch"):
            run_differential(replace(spec, expected_sha1="0" * 40))
        with self.assertRaisesRegex(RunnerError, "USA ROM must be mapped"):
            run_differential(replace(spec, rom_base=0x09000000))
        with self.assertRaisesRegex(RunnerError, "extends beyond"):
            run_differential(replace(spec, overlay_offset=0xFFF,
                                     candidate_bytes=b"abcd"))

    def test_cli_scenarios_cannot_override_usa_hash_or_rom_base(self):
        scenario = Path(self.temp.name) / "scenario.json"
        scenario.write_text(json.dumps({"rom_path": "synthetic.gba",
                                        "expected_sha1": "0" * 40}))
        with self.assertRaisesRegex(RunnerError, "expected USA ROM SHA-1"):
            _load_spec(scenario)
        scenario.write_text(json.dumps({"rom_path": "synthetic.gba",
                                        "rom_base": "0x09000000"}))
        with self.assertRaisesRegex(RunnerError, "map the USA ROM at 0x08000000"):
            _load_spec(scenario)

    def test_unexpected_execution_outside_leaf_extent_is_rejected(self):
        # b.n 0x120, outside the declared two-byte leaf body.
        code = bytes.fromhex("0e e0")
        with self.assertRaisesRegex(RunnerError, "outside its leaf-function extent"):
            run_differential(self.spec(code, code, watch=False))

    def test_thumb2_and_arm_state_execution_are_rejected(self):
        # LDR.W r0,[r1] starts with a Thumb-2 32-bit prefix.
        thumb2 = bytes.fromhex("d1 f8 00 00 70 47")
        with self.assertRaisesRegex(RunnerError, "unsupported 32-bit Thumb encoding"):
            run_differential(self.spec(thumb2, thumb2, watch=False))
        # BL's Thumb-1 paired encoding is also rejected because the runner's
        # contract prohibits calls, including calls whose target is in-range.
        bl = bytes.fromhex("00 f0 00 f8 70 47")
        with self.assertRaisesRegex(RunnerError, "unsupported 32-bit Thumb encoding"):
            run_differential(self.spec(bl, bl, watch=False))
        # BLX register is a 16-bit encoding and needs its own guard.
        blx = bytes.fromhex("80 47 70 47")
        with self.assertRaisesRegex(RunnerError, "uses BLX"):
            run_differential(self.spec(blx, blx,
                                       args=(0x08000104,), watch=False))
        # BX r2 enters the same bounded bytes in ARM state when r2 is even.
        arm_switch = bytes.fromhex("10 47 00 bf 70 47")
        with self.assertRaisesRegex(RunnerError, "switched to ARM state"):
            run_differential(self.spec(arm_switch, arm_switch,
                                       args=(0, 0, 0x08000104), watch=False))

    def test_vacuous_comparison_is_rejected(self):
        code = bytes.fromhex("70 47")
        spec = self.spec(code, code)
        with self.assertRaisesRegex(RunnerError, "select at least one"):
            run_differential(replace(spec, compare_registers=(), watch_ranges=()))

    def test_cli_returns_nonzero_for_a_behavioral_diff(self):
        original = bytes.fromhex("01 20 70 47")
        candidate = bytes.fromhex("02 20 70 47")
        spec = self.spec(original, candidate)
        with patch("tools.decomp.thumb_runner._load_spec", return_value=spec), \
                patch.object(sys, "argv", ["thumb_runner.py", "scenario.json"]), \
                patch("sys.stdout", new_callable=io.StringIO):
            self.assertEqual(main(), 1)

    def test_mmio_cannot_be_declared_as_ram(self):
        code = bytes.fromhex("70 47")
        spec = self.spec(code, code, watch=False)
        with self.assertRaisesRegex(RunnerError, "unsupported MMIO/video memory"):
            run_differential(replace(spec, ram_regions=(
                MemoryRegion(0x04000000, 0x1000, label="fake-ram"),)))

    @unittest.skipUnless((Path(__file__).resolve().parents[1] / "baserom.gba").exists(),
                         "optional real allocator fixture requires local USA ROM")
    def test_original_allocator_recenter_first_hop_counts_zero_through_four(self):
        root = Path(__file__).resolve().parents[1]
        rom_path = root / "baserom.gba"
        rom = rom_path.read_bytes()
        self.assertEqual(hashlib.sha1(rom).hexdigest(), EXPECTED_USA_SHA1)
        start, end = 0x6D38, 0x6D9E
        original_function = rom[start:end]
        self.assertEqual(len(original_function), 102)

        owner = EWRAM_BASE + 0x100
        for count, expected_offset in ((0, 8), (1, 8), (2, 8), (3, 8), (4, 12)):
            initial = bytearray(0x1000)
            owner_index = owner - EWRAM_BASE
            initial[owner_index:owner_index + 2] = (8).to_bytes(2, "little")
            initial[owner_index + 2:owner_index + 4] = b"\xD4\xB6"
            initial[owner_index + 4:owner_index + 6] = count.to_bytes(2, "little")
            # Head offset 8 points to owner+32; its +10 link points to
            # word offset 12. Counts 0..3 must leave that link unread.
            initial[owner_index + 32 + 10:owner_index + 32 + 12] = (12).to_bytes(2, "little")
            initial[owner_index + 48 + 10:owner_index + 48 + 12] = (0).to_bytes(2, "little")
            spec = RunSpec(
                rom_path=rom_path,
                entry_offset=start,
                overlay_offset=start,
                candidate_bytes=original_function,
                original_function_size=len(original_function),
                args=(owner,),
                ram_regions=(MemoryRegion(EWRAM_BASE, 0x1000, bytes(initial), "ewram"),),
                instruction_budget=100,
                watch_ranges=(WatchRange(owner, 64, "owner_and_nodes"),),
                compare_registers=(),
                expected_sha1=EXPECTED_USA_SHA1,
            )
            report = run_differential(spec)
            self.assertEqual(report["status"], "MATCH", f"count={count}: {report['differences']}")
            expected = bytearray(initial[owner_index:owner_index + 64])
            expected[2:4] = expected_offset.to_bytes(2, "little")
            actual = bytes.fromhex(report["original"]["watched_memory_hex"]["owner_and_nodes"])
            self.assertEqual(actual, bytes(expected), f"original count={count}")
        self.assertEqual(hashlib.sha1(rom_path.read_bytes()).hexdigest(), EXPECTED_USA_SHA1)


if __name__ == "__main__":
    unittest.main()
