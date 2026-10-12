#!/usr/bin/env python3
"""Record an already spawned Luna session and its independent review.

This tool never spawns agents. Registration appends exactly one distinct live
agent identity. A registration timestamp is not a spawn timestamp or effort
measurement. Review counts are supplied explicitly and are never inferred.
"""
import argparse
import json
import os
import tempfile
from datetime import datetime, timezone
from pathlib import Path

from luna_sessions import DEFAULT_REGISTRY, validate


def _timestamp(value=None):
    if value is None:
        return datetime.now(timezone.utc).isoformat()
    if not isinstance(value, str):
        raise ValueError("timestamp must be a timezone-aware ISO timestamp")
    try:
        parsed = datetime.fromisoformat(value.replace("Z", "+00:00"))
    except ValueError as exc:
        raise ValueError("timestamp must be a timezone-aware ISO timestamp") from exc
    if parsed.tzinfo is None or parsed.utcoffset() is None:
        raise ValueError("timestamp must be a timezone-aware ISO timestamp")
    return parsed.isoformat()


def _read(path):
    try:
        registry = json.loads(path.read_text())
    except (OSError, json.JSONDecodeError) as exc:
        raise ValueError(f"cannot read registry: {exc}") from exc
    return validate(registry)


def _atomic_write(path, registry):
    payload = (json.dumps(registry, indent=2) + "\n").encode()
    mode = path.stat().st_mode & 0o777
    fd, temp_name = tempfile.mkstemp(prefix=f".{path.name}.", suffix=".tmp", dir=path.parent)
    try:
        with os.fdopen(fd, "wb") as stream:
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        os.chmod(temp_name, mode)
        os.replace(temp_name, path)
    except BaseException:
        try:
            os.unlink(temp_name)
        except FileNotFoundError:
            pass
        raise


def register_spawned_session(path, *, agent, model, task, spawned_utc=None, registered_utc=None):
    """Append a distinct agent known to have spawned successfully."""
    path = Path(path)
    registry = _read(path)
    for name, value in (("agent", agent), ("model", model), ("task", task)):
        if not isinstance(value, str) or not value.strip():
            raise ValueError(f"{name} must be a nonempty string")
    identity = agent.strip()
    if any(s.get("agent", "").strip() == identity for s in registry["sessions"]):
        raise ValueError(f"duplicate agent identity: {identity}")
    session_id = len(registry["sessions"]) + 1
    if session_id > registry["target_sessions"]:
        raise ValueError("session target is already full")
    if any(s["id"] == session_id for s in registry["sessions"]):
        raise ValueError(f"next session id {session_id} already exists")
    registered = _timestamp(registered_utc)
    record = {
        "id": session_id,
        "agent": identity,
        "model": model.strip(),
        "task": task.strip(),
        "status": "running",
        "registered_utc": registered,
        "registration_timestamp_kind": "registration",
        "parent_minutes": None,
        "agent_minutes": None,
        "accepted_matching_functions": 0,
        "accepted_matching_bytes": 0,
        "accepted_matching_code_bytes": 0,
        "accepted_matching_data_bytes": 0,
    }
    if spawned_utc is not None:
        record["started_utc"] = _timestamp(spawned_utc)
        record["registration_timestamp_kind"] = "registration_with_supplied_spawn_time"
    else:
        record["timing_note"] = "Registration timestamp; exact spawn timestamp unavailable. Do not use for agent duration."
    registry["sessions"].append(record)
    validate(registry)
    _atomic_write(path, registry)
    return record


def mark_reviewed(path, *, session_id, evidence, accepted_functions, accepted_code_bytes,
                  accepted_data_bytes, first_review_accepted=None, reviewed_utc=None):
    """Mark a started session reviewed using explicit evidence and accepted counts."""
    path = Path(path)
    registry = _read(path)
    if isinstance(session_id, bool) or not isinstance(session_id, int) or session_id < 1:
        raise ValueError("session_id must be a positive integer")
    if not isinstance(evidence, str) or not evidence.strip():
        raise ValueError("review evidence must be nonempty")
    counts = {
        "accepted_matching_functions": accepted_functions,
        "accepted_matching_code_bytes": accepted_code_bytes,
        "accepted_matching_data_bytes": accepted_data_bytes,
    }
    for field, value in counts.items():
        if isinstance(value, bool) or not isinstance(value, int) or value < 0:
            raise ValueError(f"{field} must be a nonnegative integer")
    if first_review_accepted is not None and not isinstance(first_review_accepted, bool):
        raise ValueError("first_review_accepted must be true, false, or omitted")
    record = next((s for s in registry["sessions"] if s["id"] == session_id), None)
    if record is None:
        raise ValueError(f"unknown session id: {session_id}")
    if record["status"] not in {"running", "review_pending"}:
        raise ValueError(f"session {session_id} is not awaiting review")
    record.update(counts)
    record["accepted_matching_bytes"] = accepted_code_bytes + accepted_data_bytes
    record["review_evidence"] = evidence.strip()
    record["reviewed_utc"] = _timestamp(reviewed_utc)
    record["status"] = "reviewed"
    if first_review_accepted is not None:
        record["first_review_accepted"] = first_review_accepted
    validate(registry)
    _atomic_write(path, registry)
    return record


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("registry", nargs="?", type=Path, default=DEFAULT_REGISTRY)
    commands = parser.add_subparsers(dest="command", required=True)
    add = commands.add_parser("register", help="register an already spawned distinct agent")
    add.add_argument("--agent", required=True)
    add.add_argument("--model", required=True)
    add.add_argument("--task", required=True)
    add.add_argument("--spawned-utc", help="actual spawn time, if known (timezone-aware ISO timestamp)")
    review = commands.add_parser("review", help="record independent review evidence and accepted counts")
    review.add_argument("--id", type=int, required=True)
    review.add_argument("--evidence", required=True)
    review.add_argument("--accepted-functions", type=int, required=True)
    review.add_argument("--accepted-code-bytes", type=int, required=True)
    review.add_argument("--accepted-data-bytes", type=int, required=True)
    review.add_argument("--first-review-accepted", choices=("true", "false"))
    args = parser.parse_args(argv)
    try:
        if args.command == "register":
            result = register_spawned_session(args.registry, agent=args.agent, model=args.model,
                                              task=args.task, spawned_utc=args.spawned_utc)
        else:
            first = None if args.first_review_accepted is None else args.first_review_accepted == "true"
            result = mark_reviewed(args.registry, session_id=args.id, evidence=args.evidence,
                                   accepted_functions=args.accepted_functions,
                                   accepted_code_bytes=args.accepted_code_bytes,
                                   accepted_data_bytes=args.accepted_data_bytes,
                                   first_review_accepted=first)
    except (OSError, ValueError) as exc:
        parser.error(str(exc))
    print(json.dumps(result, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
