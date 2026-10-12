"""Immutable ELF reference snapshots for isolated probes.

The probe CLI may skip this module entirely when a candidate has no undefined
symbols. With dependencies, it snapshots the selected reference ELF into the
probe directory before parsing symbols, so later builds cannot change the
reference halfway through a probe. A minimal caller looks like::

    symbols, reference = load_reference_symbols(
        undefined, args.reference_elf, probe_directory)

Pass ``symbols`` to the existing typed-reference emitter and include
``reference`` in the comparison JSON. Thumb function symbols retain their odd
address and ARM function symbols retain their even address.
"""
import hashlib
from pathlib import Path
import subprocess


class ReferenceElfError(ValueError):
    """The selected reference ELF is unavailable or cannot be read."""


def parse_readelf_symbols(output):
    """Return defined symbol name -> (value, type) from ``readelf -sW`` text."""
    symbols = {}
    for line in output.splitlines():
        parts = line.split()
        if (len(parts) >= 8 and parts[0].endswith(':')
                and parts[0][:-1].isdigit() and parts[6] != 'UND'):
            try:
                value = int(parts[1], 16)
            except ValueError:
                continue
            symbols[parts[7]] = (value, parts[3])
    return symbols


def snapshot_reference_elf(reference_elf, probe_directory):
    """Copy an ELF once into ``probe_directory`` and return it with identity.

    The selected input must be a readable ELF file. The snapshot is a regular
    copy, never a symlink, and its SHA-256 describes the bytes used by later
    symbol parsing.
    """
    selected = Path(reference_elf).expanduser()
    try:
        source = selected.read_bytes()
    except OSError as error:
        raise ReferenceElfError(
            f"cannot read reference ELF {selected}: {error.strerror or error}") from error
    if len(source) < 4 or source[:4] != b'\x7fELF':
        raise ReferenceElfError(f"invalid reference ELF {selected}: missing ELF magic")

    destination_directory = Path(probe_directory)
    destination_directory.mkdir(parents=True, exist_ok=True)
    snapshot = destination_directory / 'reference.elf'
    try:
        snapshot.write_bytes(source)
    except OSError as error:
        raise ReferenceElfError(
            f"cannot snapshot reference ELF to {snapshot}: {error.strerror or error}") from error
    return snapshot, {
        'selected_reference_elf': str(selected.resolve()),
        'reference_elf_snapshot': str(snapshot.resolve()),
        'reference_elf_sha256': hashlib.sha256(source).hexdigest(),
    }


def load_reference_symbols(undefined, reference_elf, probe_directory,
                           readelf='arm-none-eabi-readelf', check_output=None):
    """Load symbols for undefined names, snapshotting only when names exist.

    ``check_output`` is injectable for tests; production callers use
    ``subprocess.check_output`` with an argv list (no shell parsing).
    """
    names = list(undefined)
    if not names:
        return {}, None

    snapshot, identity = snapshot_reference_elf(reference_elf, probe_directory)
    runner = check_output or subprocess.check_output
    command = [readelf, '-sW', str(snapshot)]
    try:
        output = runner(command, text=True)
    except (OSError, subprocess.CalledProcessError) as error:
        detail = getattr(error, 'stderr', None) or str(error)
        raise ReferenceElfError(
            f"cannot parse reference ELF snapshot {snapshot} with {readelf}: {detail}") from error
    return parse_readelf_symbols(output), identity
