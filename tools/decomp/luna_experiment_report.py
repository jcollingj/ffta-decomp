#!/usr/bin/env python3
"""Assess the Luna session experiment using the validated session registry.

Accepted-output and batch metrics include reviewed records only. Numeric sums
come from luna_sessions.report(); this module adds coverage so defaulted/missing
values remain visible instead of being mistaken for measured zeroes.
"""
import argparse
import json
import sys
from pathlib import Path

import luna_sessions


ROOT = Path(__file__).resolve().parents[2]
DEFAULT_REGISTRY = ROOT / "config/luna_sessions.json"
DEFAULT_DOCUMENT = ROOT / "docs/LUNA_EXPERIMENT.md"
BEGIN_MARKER = "<!-- BEGIN GENERATED LUNA ASSESSMENT -->"
END_MARKER = "<!-- END GENERATED LUNA ASSESSMENT -->"
COVERAGE_FIELDS = (
    "accepted_matching_functions",
    "accepted_matching_bytes",
    "accepted_matching_code_bytes",
    "accepted_matching_data_bytes",
    "revision_rounds",
    "parent_corrective_edits",
    "first_review_accepted",
    "parent_minutes",
    "agent_minutes",
)


def assess(registry):
    """Return validated base metrics plus reviewed-record field coverage."""
    result = luna_sessions.report(registry)
    reviewed = [s for s in registry["sessions"] if s["status"] == "reviewed"]

    def coverage(rows):
        return {
            field: {
                "recorded": sum(field in row and row[field] is not None for row in rows),
                "missing": sum(field not in row or row[field] is None for row in rows),
            }
            for field in COVERAGE_FIELDS
        }

    result["reviewed_field_coverage"] = coverage(reviewed)
    for batch in result["reviewed_batches_of_10"]:
        lo, hi = batch["session_ids"]
        rows = [s for s in reviewed if lo <= s["id"] <= hi]
        batch["field_coverage"] = coverage(rows)
    result["complete"] = result["reviewed"] == result["target_sessions"]
    return result


def _coverage_cell(entry):
    return f"{entry['recorded']} recorded / {entry['missing']} missing"


def _first_review_line(metrics):
    values = metrics["first_review_accepted"]
    known = values["true"] + values["false"]
    if not known:
        rate = "not available (0 known)"
    else:
        rate = f"{values['true'] / known:.1%} ({values['true']}/{known} known)"
    return (f"{values['true']} accepted, {values['false']} not accepted, "
            f"{values['unknown']} unknown; known-only rate {rate}")


def render_markdown(result):
    """Render stable Markdown from assess() output."""
    m = result["reviewed_metrics"]
    lines = [
        f"**Progress:** {result['started']} started, {result['reviewed']} reviewed, "
        f"{result['planned']} planned; target {result['target_sessions']}. "
        f"Experiment status: {'complete' if result['complete'] else 'incomplete'}.",
        "",
        "Only reviewed records contribute accepted-output totals and batch results. "
        "Sums below are recorded values; the existing registry reporter treats absent "
        "integer counters as zero, so coverage is shown alongside totals.",
        "",
        "| Reviewed accepted output | Recorded total |",
        "|---|---:|",
        f"| Matching functions | {m['accepted_matching_functions']} |",
        f"| Matching bytes | {m['accepted_matching_bytes']} |",
        f"| Code bytes | {m['accepted_matching_code_bytes']} |",
        f"| Data bytes | {m['accepted_matching_data_bytes']} |",
        f"| Unclassified accepted bytes | {m['unclassified_accepted_bytes']} |",
        f"| Reviewed records with nonzero accepted functions or bytes | {m['reviewed_accepted_outputs']} |",
        "",
        f"**First review:** {_first_review_line(m)}. This includes plan, code, and integration feedback; "
        "it is not a clean coding-accuracy measure.",
        "",
        "| Reviewed metric field | Coverage |",
        "|---|---:|",
    ]
    for field in COVERAGE_FIELDS:
        lines.append(f"| `{field}` | {_coverage_cell(result['reviewed_field_coverage'][field])} |")
    lines.extend([
        "",
        "| Reviewed intervention metric | Recorded total | Coverage |",
        "|---|---:|---:|",
        f"| Revision rounds | {m['revision_rounds']} | {_coverage_cell(result['reviewed_field_coverage']['revision_rounds'])} |",
        f"| Parent corrective edits | {m['parent_corrective_edits']} | {_coverage_cell(result['reviewed_field_coverage']['parent_corrective_edits'])} |",
        "",
        "| Effort/timing measure | Recorded sum | Known records | Unknown records |",
        "|---|---:|---:|---:|",
    ])
    for field, label in (("parent_minutes", "Parent effort minutes"),
                         ("agent_minutes", "Agent effort minutes"),
                         ("elapsed_minutes", "Elapsed wall minutes")):
        item = m[field]
        lines.append(f"| {label} | {item['known_minutes']:g} | {item['known_records']} | {item['unknown_records']} |")
    lines.extend([
        "",
        "Elapsed wall time is not person effort; registration timestamps are not spawn times. "
        "Concurrent elapsed time cannot establish effort share. Cost/token data are unavailable. "
        "No speedup or 90% effort-share claim is supported by this registry.",
        "",
        "### Reviewed batches of ten",
        "",
        "Accepted output and intervention totals below use reviewed records in each session-ID batch. "
        "Metric coverage distinguishes explicit zero from missing data.",
        "",
        "| Sessions | Reviewed | Functions | Code bytes | Data bytes | Unclassified bytes | First review T/F/? (known rate) | Revisions | Coverage | Parent edits | Coverage |",
        "|---:|---:|---:|---:|---:|---:|---|---:|---|---:|---|",
    ])
    for batch in result["reviewed_batches_of_10"]:
        lo, hi = batch["session_ids"]
        if not batch["reviewed"]:
            continue
        fc = batch["field_coverage"]
        first = batch["first_review_accepted"]
        known = first["true"] + first["false"]
        rate = f"{first['true'] / known:.1%} ({first['true']}/{known})" if known else "n/a (0 known)"
        lines.append(
            f"| {lo}–{hi} | {batch['reviewed']} | {batch['accepted_matching_functions']} | "
            f"{batch['accepted_matching_code_bytes']} | {batch['accepted_matching_data_bytes']} | "
            f"{batch['unclassified_accepted_bytes']} | "
            f"{first['true']}/{first['false']}/{first['unknown']} ({rate}) | "
            f"{batch['revision_rounds']} | {_coverage_cell(fc['revision_rounds'])} | "
            f"{batch['parent_corrective_edits']} | {_coverage_cell(fc['parent_corrective_edits'])} |"
        )
    lines.extend([
        "",
        "Reviewed testing/tool sessions with zero accepted matching functions and bytes may still have "
        "provided useful evidence, but receive no recovered-code/data credit in these totals.",
    ])
    return "\n".join(lines) + "\n"


def replace_generated_section(document, generated):
    """Replace exactly one marked section, preserving all text outside it."""
    if document.count(BEGIN_MARKER) != 1 or document.count(END_MARKER) != 1:
        raise ValueError("document must contain exactly one generated-section marker pair")
    start = document.index(BEGIN_MARKER) + len(BEGIN_MARKER)
    end = document.index(END_MARKER)
    if end < start:
        raise ValueError("generated-section markers are out of order")
    return document[:start] + "\n\n" + generated.rstrip() + "\n\n" + document[end:]


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("registry", nargs="?", type=Path, default=DEFAULT_REGISTRY)
    parser.add_argument("--json", action="store_true", help="print machine-readable assessment JSON")
    parser.add_argument("--refresh-doc", action="store_true",
                        help=f"replace only the marked assessment section in {DEFAULT_DOCUMENT}")
    parser.add_argument("--document", type=Path, default=DEFAULT_DOCUMENT,
                        help="document to update with --refresh-doc")
    args = parser.parse_args(argv)
    try:
        registry = json.loads(args.registry.read_text())
        result = assess(registry)
        rendered = render_markdown(result)
        if args.refresh_doc:
            document = args.document.read_text()
            updated = replace_generated_section(document, rendered)
            args.document.write_text(updated)
            print(f"Updated generated assessment in {args.document}")
        elif args.json:
            print(json.dumps(result, indent=2, sort_keys=True))
        else:
            print(rendered, end="")
    except (OSError, json.JSONDecodeError, ValueError) as exc:
        parser.error(str(exc))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
