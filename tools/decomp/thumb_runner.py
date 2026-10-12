#!/usr/bin/env python3
"""Run bounded Thumb leaf-function differential checks against the USA ROM.

This is a CPU-only code probe. It deliberately does not model BIOS calls,
MMIO, interrupts, DMA, timing, or game callbacks.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import hashlib
import json
from pathlib import Path
import time
from typing import Any


ROM_BASE = 0x08000000
EXPECTED_USA_SHA1 = "4ac05441f4de70a4ec3dd932116346c61b8783d9"
EWRAM_BASE = 0x02000000
EWRAM_SIZE = 0x00040000
IWRAM_BASE = 0x03000000
IWRAM_SIZE = 0x00008000
RETURN_SENTINEL = 0x0FFF0000
PAGE_SIZE = 0x1000
UNSUPPORTED_BUS_RANGES = ((0x00000000, 0x00004000, "BIOS"),
                          (0x04000000, 0x08000000, "MMIO/video memory"))


class RunnerError(RuntimeError):
    """Invalid setup or execution outside the supported probe contract."""


class InstructionBudgetExceeded(RunnerError):
    """The function did not return within its explicit instruction budget."""


class ExecutionTimedOut(RunnerError):
    """The function did not return within its explicit wall-clock timeout."""


@dataclass(frozen=True)
class MemoryRegion:
    address: int
    size: int
    initial: bytes = b""
    label: str = "ram"


@dataclass(frozen=True)
class WatchRange:
    address: int
    size: int
    label: str


@dataclass(frozen=True)
class RunSpec:
    rom_path: Path
    entry_offset: int
    overlay_offset: int
    candidate_bytes: bytes
    original_function_size: int | None = None
    args: tuple[int, ...] = ()
    ram_regions: tuple[MemoryRegion, ...] = ()
    stack_base: int = IWRAM_BASE
    stack_size: int = IWRAM_SIZE
    instruction_budget: int = 10000
    timeout_us: int = 250000
    watch_ranges: tuple[WatchRange, ...] = ()
    compare_registers: tuple[str, ...] = ()
    expected_sha1: str = EXPECTED_USA_SHA1
    rom_base: int = ROM_BASE


def _unicorn():
    try:
        import unicorn
        from unicorn import arm_const
    except ImportError as exc:
        raise RunnerError(
            "Unicorn is not installed; run tools/decomp/bootstrap_thumb_runner.py"
        ) from exc
    return unicorn, arm_const


def _sha1(data: bytes) -> str:
    return hashlib.sha1(data).hexdigest()


def _reg_id(name: str, arm_const: Any) -> int:
    normalized = name.lower()
    ids = {f"r{i}": getattr(arm_const, f"UC_ARM_REG_R{i}") for i in range(13)}
    ids.update({
        "sp": arm_const.UC_ARM_REG_SP,
        "lr": arm_const.UC_ARM_REG_LR,
        "pc": arm_const.UC_ARM_REG_PC,
        "cpsr": arm_const.UC_ARM_REG_CPSR,
    })
    if normalized not in ids:
        raise RunnerError(f"unsupported register in comparison: {name}")
    return ids[normalized]


def _check_range(address: int, size: int, *, what: str) -> None:
    if address < 0 or size <= 0 or address + size > 0x100000000:
        raise RunnerError(f"invalid {what} range 0x{address:x}+0x{size:x}")


def _validate_spec(spec: RunSpec, rom: bytes) -> None:
    if spec.rom_base != ROM_BASE:
        raise RunnerError("USA ROM must be mapped at 0x08000000")
    if _sha1(rom) != spec.expected_sha1:
        raise RunnerError(
            f"base ROM SHA-1 mismatch: expected {spec.expected_sha1}, got {_sha1(rom)}"
        )
    if len(rom) == 0 or len(rom) % PAGE_SIZE:
        raise RunnerError("ROM size must be a nonzero multiple of 0x1000 bytes")
    if spec.entry_offset < 0 or spec.entry_offset + 2 > len(rom):
        raise RunnerError("Thumb entry offset is outside the base ROM")
    if spec.entry_offset & 1:
        raise RunnerError("Thumb entry offset must be halfword aligned")
    _check_range(spec.overlay_offset, len(spec.candidate_bytes), what="candidate overlay")
    if spec.overlay_offset + len(spec.candidate_bytes) > len(rom):
        raise RunnerError("candidate overlay extends beyond the base ROM")
    if spec.overlay_offset != spec.entry_offset:
        raise RunnerError("candidate overlay must begin at the Thumb entry point")
    original_size = (len(spec.candidate_bytes) if spec.original_function_size is None
                     else spec.original_function_size)
    if original_size < 2 or spec.entry_offset + original_size > len(rom):
        raise RunnerError("original function extent is outside the base ROM")
    if not spec.candidate_bytes:
        raise RunnerError("candidate overlay must contain at least one byte")
    if len(spec.candidate_bytes) & 1 or original_size & 1:
        raise RunnerError("Thumb function extents must end on a halfword boundary")
    if len(spec.args) > 4:
        raise RunnerError("AAPCS leaf probes accept at most r0-r3 arguments")
    if spec.instruction_budget <= 0:
        raise RunnerError("instruction budget must be positive")
    if spec.timeout_us <= 0:
        raise RunnerError("execution timeout must be positive")
    if not spec.compare_registers and not spec.watch_ranges:
        raise RunnerError("comparison must select at least one register or watched range")
    if spec.stack_base % PAGE_SIZE or spec.stack_size <= 0 or spec.stack_size % PAGE_SIZE:
        raise RunnerError("stack mapping must use positive page-aligned bounds")
    _check_range(spec.stack_base, spec.stack_size, what="stack")
    stack_end = spec.stack_base + spec.stack_size
    if spec.stack_base < IWRAM_BASE or stack_end > IWRAM_BASE + IWRAM_SIZE:
        raise RunnerError("stack must be contained in the supported IWRAM range")
    for unsupported_start, unsupported_end, unsupported_name in UNSUPPORTED_BUS_RANGES:
        if spec.stack_base < unsupported_end and unsupported_start < stack_end:
            raise RunnerError(f"mapping unsupported {unsupported_name} as stack is forbidden")
    seen = [(spec.stack_base, stack_end, "stack")]
    for region in spec.ram_regions:
        _check_range(region.address, region.size, what=region.label)
        if region.address % PAGE_SIZE or region.size % PAGE_SIZE:
            raise RunnerError(f"{region.label} mapping must be page aligned")
        if len(region.initial) > region.size:
            raise RunnerError(f"initial bytes exceed {region.label} mapping")
        region_end = region.address + region.size
        for unsupported_start, unsupported_end, unsupported_name in UNSUPPORTED_BUS_RANGES:
            if region.address < unsupported_end and unsupported_start < region_end:
                raise RunnerError(f"mapping unsupported {unsupported_name} as RAM is forbidden")
        in_ewram = EWRAM_BASE <= region.address and region_end <= EWRAM_BASE + EWRAM_SIZE
        in_iwram = IWRAM_BASE <= region.address and region_end <= IWRAM_BASE + IWRAM_SIZE
        if not (in_ewram or in_iwram):
            raise RunnerError(f"{region.label} must be contained in supported EWRAM or IWRAM")
        seen.append((region.address, region_end, region.label))
    rom_range = (spec.rom_base, spec.rom_base + len(rom), "ROM")
    if spec.rom_base % PAGE_SIZE:
        raise RunnerError("ROM base must be page aligned")
    _check_range(spec.rom_base, len(rom), what="ROM")
    seen.append(rom_range)
    seen.append((RETURN_SENTINEL, RETURN_SENTINEL + PAGE_SIZE, "return sentinel"))
    for index, (start, end, label) in enumerate(seen):
        for other_start, other_end, other_label in seen[index + 1:]:
            if start < other_end and other_start < end:
                raise RunnerError(f"overlapping memory maps: {label} and {other_label}")
    for watch in spec.watch_ranges:
        _check_range(watch.address, watch.size, what=f"watch {watch.label}")
    labels = [watch.label for watch in spec.watch_ranges]
    if len(labels) != len(set(labels)):
        raise RunnerError("watch labels must be unique")


def _run(spec: RunSpec, rom: bytes, *, overlay: bool) -> dict[str, Any]:
    unicorn, arm_const = _unicorn()
    from unicorn import Uc

    image = bytearray(rom)
    if overlay:
        image[spec.overlay_offset:spec.overlay_offset + len(spec.candidate_bytes)] = spec.candidate_bytes

    uc = Uc(unicorn.UC_ARCH_ARM, unicorn.UC_MODE_THUMB | unicorn.UC_MODE_LITTLE_ENDIAN)
    # Unicorn has no ARM7TDMI model. TI925T is an ARMv4T/Thumb-1 model; wider
    # encodings and ARM-state transitions are trapped explicitly below too.
    uc.ctl_set_cpu_model(arm_const.UC_CPU_ARM_TI925T)
    rom_prot = unicorn.UC_PROT_READ | unicorn.UC_PROT_EXEC
    uc.mem_map(spec.rom_base, len(image), rom_prot)
    uc.mem_write(spec.rom_base, bytes(image))
    uc.mem_map(spec.stack_base, spec.stack_size, unicorn.UC_PROT_READ | unicorn.UC_PROT_WRITE)
    for region in spec.ram_regions:
        uc.mem_map(region.address, region.size, unicorn.UC_PROT_READ | unicorn.UC_PROT_WRITE)
        if region.initial:
            uc.mem_write(region.address, region.initial)

    # `until` terminates before this unmapped address is fetched. A function
    # that fails to branch through LR never receives successful completion.
    for index in range(13):
        # Distinct sentinels make accidental clobbers observable, including
        # callee-saved registers selected by the scenario.
        uc.reg_write(getattr(arm_const, f"UC_ARM_REG_R{index}"),
                     0xA5A50000 | index)
    for index, value in enumerate(spec.args):
        uc.reg_write(getattr(arm_const, f"UC_ARM_REG_R{index}"), value & 0xFFFFFFFF)
    stack_top = spec.stack_base + spec.stack_size - 8
    uc.reg_write(arm_const.UC_ARM_REG_SP, stack_top)
    uc.reg_write(arm_const.UC_ARM_REG_LR, RETURN_SENTINEL | 1)

    start = spec.rom_base + spec.entry_offset
    instruction_count = 0
    forbidden_pc: int | None = None
    forbidden_reason: str | None = None
    timeout_reached = False
    started = time.perf_counter_ns()
    allowed_end = (spec.entry_offset + len(spec.candidate_bytes) if overlay
                   else spec.entry_offset + (len(spec.candidate_bytes)
                                              if spec.original_function_size is None
                                              else spec.original_function_size))

    def count_instruction(_uc, _address, _size, _user_data):
        nonlocal instruction_count, forbidden_pc, forbidden_reason, timeout_reached
        if time.perf_counter_ns() - started >= spec.timeout_us * 1000:
            timeout_reached = True
            _uc.emu_stop()
            return
        offset = _address - spec.rom_base
        if not spec.entry_offset <= offset < allowed_end:
            forbidden_pc = _address
            forbidden_reason = "outside its leaf-function extent"
            _uc.emu_stop()
            return
        if (_uc.reg_read(arm_const.UC_ARM_REG_CPSR) & 0x20) == 0:
            forbidden_pc = _address
            forbidden_reason = "switched to ARM state"
            _uc.emu_stop()
            return
        halfword = int.from_bytes(_uc.mem_read(_address, 2), "little")
        if (halfword >> 11) in (0b11101, 0b11110, 0b11111):
            forbidden_pc = _address
            forbidden_reason = f"uses unsupported 32-bit Thumb encoding 0x{halfword:04x}"
            _uc.emu_stop()
            return
        if (halfword & 0xFF00) == 0xDF00:
            forbidden_pc = _address
            forbidden_reason = "uses SWI without BIOS emulation"
            _uc.emu_stop()
            return
        if (halfword & 0xFF87) == 0x4780:
            forbidden_pc = _address
            forbidden_reason = "uses BLX and leaves the leaf-function contract"
            _uc.emu_stop()
            return
        instruction_count += 1

    uc.hook_add(unicorn.UC_HOOK_CODE, count_instruction)
    try:
        uc.emu_start(start | 1, RETURN_SENTINEL, timeout=spec.timeout_us,
                     count=spec.instruction_budget)
    except unicorn.UcError as exc:
        raise RunnerError(
            f"{('candidate' if overlay else 'original')} execution failed at "
            f"PC 0x{uc.reg_read(arm_const.UC_ARM_REG_PC):08x}: {exc}"
        ) from exc

    elapsed_ns = time.perf_counter_ns() - started
    if forbidden_pc is not None:
        raise RunnerError(
            f"{('candidate' if overlay else 'original')} attempted execution outside "
            f"the supported contract ({forbidden_reason}) at PC 0x{forbidden_pc:08x}"
        )
    pc = uc.reg_read(arm_const.UC_ARM_REG_PC) & 0xFFFFFFFF
    completed = pc == RETURN_SENTINEL
    thumb_state = (uc.reg_read(arm_const.UC_ARM_REG_CPSR) & 0x20) != 0
    if completed and not thumb_state:
        raise RunnerError(f"{('candidate' if overlay else 'original')} returned in ARM state")
    if not completed:
        if timeout_reached or elapsed_ns >= spec.timeout_us * 1000:
            raise ExecutionTimedOut(
                f"{('candidate' if overlay else 'original')} timed out after "
                f"{elapsed_ns / 1000:.0f} us (limit={spec.timeout_us} us)"
            )
        raise InstructionBudgetExceeded(
            f"{('candidate' if overlay else 'original')} did not return to sentinel; "
            f"PC=0x{pc:08x}, budget={spec.instruction_budget} instructions"
        )

    registers = {
        name.lower(): uc.reg_read(_reg_id(name, arm_const)) & 0xFFFFFFFF
        for name in spec.compare_registers
    }
    watched: dict[str, str] = {}
    for watch in spec.watch_ranges:
        try:
            watched[watch.label] = bytes(uc.mem_read(watch.address, watch.size)).hex()
        except unicorn.UcError as exc:
            raise RunnerError(
                f"watch range {watch.label} at 0x{watch.address:08x} is unmapped"
            ) from exc
    return {
        "variant": "candidate" if overlay else "original",
        "entry": f"0x{start:08x}",
        "return_pc": f"0x{pc:08x}",
        "instructions_executed": instruction_count,
        "elapsed_ns": elapsed_ns,
        "registers": registers,
        "watched_memory_hex": watched,
    }


def run_differential(spec: RunSpec) -> dict[str, Any]:
    """Run original and overlaid bytes in independent, identically initialized CPUs."""
    rom = spec.rom_path.read_bytes()
    _validate_spec(spec, rom)
    original = _run(spec, rom, overlay=False)
    candidate = _run(spec, rom, overlay=True)
    differences: list[dict[str, Any]] = []
    for name in spec.compare_registers:
        key = name.lower()
        before, after = original["registers"][key], candidate["registers"][key]
        if before != after:
            differences.append({"kind": "register", "name": key,
                                "original": f"0x{before:08x}",
                                "candidate": f"0x{after:08x}"})
    for watch in spec.watch_ranges:
        before = original["watched_memory_hex"][watch.label]
        after = candidate["watched_memory_hex"][watch.label]
        if before != after:
            differences.append({"kind": "memory", "name": watch.label,
                                "address": f"0x{watch.address:08x}",
                                "size": watch.size,
                                "original_hex": before,
                                "candidate_hex": after})
    return {
        "base_rom_sha1": _sha1(rom),
        "candidate_overlay": {
            "rom_offset": f"0x{spec.overlay_offset:x}",
            "size": len(spec.candidate_bytes),
            "sha1": _sha1(spec.candidate_bytes),
        },
        "instruction_budget": spec.instruction_budget,
        "timeout_us": spec.timeout_us,
        "original": original,
        "candidate": candidate,
        "status": "DIFF" if differences else "MATCH",
        "differences": differences,
        "recovery_credit": False,
    }


def _number(value: Any) -> int:
    return int(value, 0) if isinstance(value, str) else int(value)


def _load_spec(path: Path) -> RunSpec:
    raw = json.loads(path.read_text())
    configured_sha1 = raw.get("expected_sha1", EXPECTED_USA_SHA1)
    if configured_sha1 != EXPECTED_USA_SHA1:
        raise RunnerError("CLI scenarios must validate against the expected USA ROM SHA-1")
    rom_base = _number(raw.get("rom_base", ROM_BASE))
    if rom_base != ROM_BASE:
        raise RunnerError("CLI scenarios must map the USA ROM at 0x08000000")
    return RunSpec(
        rom_path=(path.parent / raw["rom_path"]).resolve(),
        entry_offset=_number(raw["entry_offset"]),
        overlay_offset=_number(raw["overlay_offset"]),
        candidate_bytes=bytes.fromhex(raw["candidate_hex"]),
        original_function_size=(_number(raw["original_function_size"])
                                if "original_function_size" in raw else None),
        args=tuple(_number(value) for value in raw.get("args", [])),
        ram_regions=tuple(MemoryRegion(
            _number(item["address"]),
            _number(item["size"]),
            bytes.fromhex(item.get("initial_hex", "")), item.get("label", "ram"))
            for item in raw.get("ram_regions", [])),
        stack_base=_number(raw.get("stack_base", IWRAM_BASE)),
        stack_size=_number(raw.get("stack_size", IWRAM_SIZE)),
        instruction_budget=_number(raw.get("instruction_budget", 10000)),
        timeout_us=_number(raw.get("timeout_us", 250000)),
        watch_ranges=tuple(WatchRange(
            _number(item["address"]),
            _number(item["size"]),
            item["label"]) for item in raw.get("watch", [])),
        compare_registers=tuple(raw.get("compare_registers", ())),
        expected_sha1=configured_sha1,
        rom_base=rom_base,
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("scenario", type=Path, help="JSON scenario (see docs/THUMB_DIFFERENTIAL.md)")
    args = parser.parse_args()
    try:
        report = run_differential(_load_spec(args.scenario))
    except (RunnerError, OSError, ValueError, KeyError, json.JSONDecodeError) as exc:
        parser.exit(2, f"thumb_runner: {exc}\n")
    print(json.dumps(report, indent=2, sort_keys=True))
    return 1 if report["status"] == "DIFF" else 0


if __name__ == "__main__":
    raise SystemExit(main())
