"""Diagnostic symbol-aligned comparisons; never relax the byte/layout gate."""
from compare import compare_bytes

ROM_BASE = 0x08000000


def text_symbols(nm_output):
    symbols = {}
    for line in nm_output.splitlines():
        fields = line.split()
        if len(fields) != 4 or fields[2] not in ('T', 't'):
            continue
        address, size = int(fields[0], 16) & ~1, int(fields[1], 16)
        if size and address >= ROM_BASE:
            symbols[fields[3]] = (address - ROM_BASE, size)
    return symbols


def merge_references(*groups):
    references = {}
    for group in groups:
        for reference in group:
            name = reference['name']
            previous = references.get(name)
            if previous and (previous['start'], previous['end']) != (reference['start'], reference['end']):
                raise ValueError(f'conflicting reference bounds for {name}')
            references[name] = reference
    return references


def compare_functions(base, candidate, load_offset, references, symbols):
    reports = []
    for name, (actual_start, size) in sorted(symbols.items(), key=lambda item: item[1][0]):
        reference = references.get(name)
        if reference is None:
            reports.append(dict(name=name, status='UNINVENTORIED', actual_start=actual_start, actual_size=size))
            continue
        start, end = reference['start'], reference['end']
        offset = actual_start - load_offset
        if not 0 <= start < end <= len(base):
            raise ValueError(f'invalid reference bounds for {name}')
        if not 0 <= offset < offset + size <= len(candidate):
            raise ValueError(f'candidate symbol outside emitted binary: {name}')
        report = compare_bytes(base[start:end], candidate[offset:offset + size], start)
        layout_matches = actual_start == start and size == end - start
        report.update(name=name, reference_start=start, actual_start=actual_start,
                      address_delta=actual_start-start, layout_matches=layout_matches,
                      status='MATCH' if report['matches'] and layout_matches else 'DIFF')
        reports.append(report)
    return reports
