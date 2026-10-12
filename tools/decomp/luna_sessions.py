#!/usr/bin/env python3
"""Validate and report the Luna session registry.

Registry schema (versionless for backwards compatibility): top-level
``target_sessions`` (positive integer), optional ``count_definition`` (string),
and ``sessions`` (array). Each session has a unique positive integer ``id``;
``status`` is planned, running, review_pending, or reviewed. ``historical`` is
an optional boolean. Started records require nonempty ``agent``, ``model``, and ``task`` strings;
agent identities must be unique.
Reviewed records require nonempty ``review_commit`` or ``review_evidence``.
``accepted_matching_functions``, ``accepted_matching_bytes``,
``revision_rounds``, and ``parent_corrective_edits`` are nonnegative integers
(default zero). ``parent_minutes`` and ``agent_minutes`` are nonnegative
numbers or null (default null). Null means unknown, never zero.
Optional code/data byte subtotals classify ``accepted_matching_bytes`` and
their sum may not exceed it; any remainder is reported as unclassified.
``first_review_accepted`` is true, false, or null/absent (unknown).
Valid timezone-aware ``started_utc`` and ``finished_utc`` are reported as
elapsed wall time only when both exist; this never fills ``agent_minutes``.

IDs describe actual sessions only; they must be consecutive from 1. Planned
records are allowed as explicit placeholders, but are excluded from started
and reviewed counts. Historical records use the same evidence rules; null
timings preserve the absence of historical measurements. Reports never infer
person-effort from elapsed or concurrent time.
"""
import argparse
import json
import math
from pathlib import Path
from datetime import datetime


ROOT = Path(__file__).resolve().parents[2]
DEFAULT_REGISTRY = ROOT / "config/luna_sessions.json"
STATUSES = {"planned", "running", "review_pending", "reviewed"}
COUNTERS = ("accepted_matching_functions", "accepted_matching_bytes",
            "revision_rounds", "parent_corrective_edits")
CLASSIFIED_BYTES = ("accepted_matching_code_bytes", "accepted_matching_data_bytes")
TIMINGS = ("parent_minutes", "agent_minutes")


def _nonnegative_number(value, field, session_id):
    if (isinstance(value, bool) or not isinstance(value, (int, float))
            or not math.isfinite(value) or value < 0):
        raise ValueError(f"session {session_id}: {field} must be a nonnegative number or null")


def validate(registry):
    if not isinstance(registry, dict):
        raise ValueError("registry must be an object")
    target = registry.get("target_sessions")
    if isinstance(target, bool) or not isinstance(target, int) or target < 1:
        raise ValueError("target_sessions must be a positive integer")
    sessions = registry.get("sessions")
    if not isinstance(sessions, list):
        raise ValueError("sessions must be an array")
    if len(sessions) > target:
        raise ValueError(f"registry has {len(sessions)} sessions, above target {target}")

    ids = []
    agent_ids = []
    for record in sessions:
        if not isinstance(record, dict):
            raise ValueError("each session must be an object")
        sid = record.get("id")
        if isinstance(sid, bool) or not isinstance(sid, int) or sid < 1:
            raise ValueError("session id must be a positive integer")
        ids.append(sid)
        status = record.get("status")
        if not isinstance(status, str) or status not in STATUSES:
            raise ValueError(f"session {sid}: unknown status {status!r}")
        for field in ("agent", "model", "task"):
            if field in record and not isinstance(record[field], str):
                raise ValueError(f"session {sid}: {field} must be a string")
        if status != "planned":
            for field in ("agent", "model", "task"):
                if not isinstance(record.get(field), str) or not record[field].strip():
                    raise ValueError(f"session {sid}: started records require nonempty {field}")
            agent_ids.append(record["agent"].strip())
        if "historical" in record and not isinstance(record["historical"], bool):
            raise ValueError(f"session {sid}: historical must be boolean")
        if status == "reviewed" and not any(
                isinstance(record.get(field), str) and record[field].strip()
                for field in ("review_commit", "review_evidence")):
            raise ValueError(f"session {sid}: reviewed status requires review evidence")
        for field in COUNTERS:
            value = record.get(field, 0)
            if isinstance(value, bool) or not isinstance(value, int) or value < 0:
                raise ValueError(f"session {sid}: {field} must be a nonnegative integer")
        for field in CLASSIFIED_BYTES:
            value = record.get(field, 0)
            if isinstance(value, bool) or not isinstance(value, int) or value < 0:
                raise ValueError(f"session {sid}: {field} must be a nonnegative integer")
        if sum(record.get(field, 0) for field in CLASSIFIED_BYTES) > record.get("accepted_matching_bytes", 0):
            raise ValueError(f"session {sid}: classified accepted bytes exceed accepted_matching_bytes")
        if "first_review_accepted" in record and record["first_review_accepted"] is not None and not isinstance(record["first_review_accepted"], bool):
            raise ValueError(f"session {sid}: first_review_accepted must be boolean or null")
        for field in TIMINGS:
            value = record.get(field)
            if value is not None:
                _nonnegative_number(value, field, sid)
        parsed = {}
        for field in ("started_utc", "finished_utc", "registered_utc", "reviewed_utc"):
            if field in record:
                value = record[field]
                try:
                    timestamp = datetime.fromisoformat(value.replace("Z", "+00:00")) if isinstance(value, str) else None
                except ValueError:
                    timestamp = None
                if timestamp is None or timestamp.tzinfo is None or timestamp.utcoffset() is None:
                    raise ValueError(f"session {sid}: {field} must be a valid timezone-aware timestamp")
                parsed[field] = timestamp
        if "started_utc" in parsed and "finished_utc" in parsed and parsed["finished_utc"] < parsed["started_utc"]:
            raise ValueError(f"session {sid}: finished_utc precedes started_utc")
    if len(set(ids)) != len(ids):
        raise ValueError("duplicate session id")
    if len(set(agent_ids)) != len(agent_ids):
        raise ValueError("duplicate agent identity across started sessions")
    if sorted(ids) != list(range(1, len(ids) + 1)):
        raise ValueError("session ids must be consecutive from 1")
    return registry


def report(registry):
    validate(registry)
    sessions = registry["sessions"]
    started = [s for s in sessions if s["status"] in {"running", "review_pending", "reviewed"}]
    reviewed = [s for s in sessions if s["status"] == "reviewed"]

    def metrics(rows):
        result = {key: sum(row.get(key, 0) for row in rows) for key in COUNTERS}
        for key in CLASSIFIED_BYTES:
            result[key] = sum(row.get(key, 0) for row in rows)
        result["unclassified_accepted_bytes"] = sum(
            row.get("accepted_matching_bytes", 0) - sum(row.get(key, 0) for key in CLASSIFIED_BYTES)
            for row in rows)
        review_values = [row.get("first_review_accepted") for row in rows]
        result["first_review_accepted"] = {
            "true": sum(value is True for value in review_values),
            "false": sum(value is False for value in review_values),
            "unknown": sum(value is None for value in review_values),
        }
        result["reviewed_accepted_outputs"] = sum(
            bool(row.get("accepted_matching_bytes", 0) or row.get("accepted_matching_functions", 0))
            for row in rows)
        elapsed = []
        for row in rows:
            if "started_utc" in row and "finished_utc" in row:
                start = datetime.fromisoformat(row["started_utc"].replace("Z", "+00:00"))
                finish = datetime.fromisoformat(row["finished_utc"].replace("Z", "+00:00"))
                elapsed.append((finish - start).total_seconds() / 60)
        result["elapsed_minutes"] = {"known_minutes": sum(elapsed), "known_records": len(elapsed),
                                     "unknown_records": len(rows) - len(elapsed)}
        for key in TIMINGS:
            known = [row[key] for row in rows if row.get(key) is not None]
            result[key] = {"known_minutes": sum(known), "known_records": len(known),
                           "unknown_records": len(rows) - len(known)}
        return result

    batches = []
    for start in range(1, registry["target_sessions"] + 1, 10):
        rows = [s for s in reviewed if start <= s["id"] < start + 10]
        batches.append({"session_ids": [start, min(start + 9, registry["target_sessions"])],
                        "reviewed": len(rows), **metrics(rows)})
    return {"target_sessions": registry["target_sessions"], "started": len(started),
            "reviewed": len(reviewed), "planned": sum(s["status"] == "planned" for s in sessions),
            "started_metrics": metrics(started), "reviewed_metrics": metrics(reviewed),
            "reviewed_batches_of_10": batches}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("registry", nargs="?", type=Path, default=DEFAULT_REGISTRY)
    parser.add_argument("--json", action="store_true", help="print machine-readable JSON")
    args = parser.parse_args(argv)
    try:
        result = report(json.loads(args.registry.read_text()))
    except (OSError, json.JSONDecodeError, ValueError) as exc:
        parser.error(str(exc))
    if args.json:
        print(json.dumps(result, indent=2))
    else:
        print(f"Sessions: {result['started']} started, {result['reviewed']} reviewed, "
              f"{result['planned']} planned (target {result['target_sessions']})")
        m = result["reviewed_metrics"]
        print(f"Accepted matching output: {m['accepted_matching_functions']} functions, "
              f"{m['accepted_matching_bytes']} bytes total; "
              f"{m['accepted_matching_code_bytes']} code, {m['accepted_matching_data_bytes']} data, "
              f"{m['unclassified_accepted_bytes']} unclassified")
        first = m["first_review_accepted"]
        print(f"First review: {first['true']} accepted, {first['false']} not accepted, {first['unknown']} unknown; "
              f"{m['reviewed_accepted_outputs']} reviewed accepted outputs")
        for role in (*TIMINGS, "elapsed_minutes"):
            t = m[role]
            print(f"{role}: {t['known_minutes']} known minutes across {t['known_records']} "
                  f"records; {t['unknown_records']} unknown")
        for b in result["reviewed_batches_of_10"]:
            if not b["reviewed"]:
                continue
            print(f"Reviewed batch {b['session_ids'][0]}–{b['session_ids'][1]}: {b['reviewed']} reviewed; "
                  f"{b['accepted_matching_code_bytes']} code bytes, {b['accepted_matching_data_bytes']} data bytes, "
                  f"{b['unclassified_accepted_bytes']} unclassified; "
                  f"first review T/F/? {b['first_review_accepted']['true']}/"
                  f"{b['first_review_accepted']['false']}/{b['first_review_accepted']['unknown']}; "
                  f"{b['parent_corrective_edits']} parent edits, {b['revision_rounds']} revisions")
        print("Parent minutes are effort; elapsed minutes are wall time and are never used as agent_minutes.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
