#!/usr/bin/env python3
"""A01 isolated EVEmu staging harness (stdlib only; Docker Compose v2 required).

All SQL is executed with `docker compose exec db` against a per-run project;
there is no arbitrary host, port, DSN or credential option. Never use live data.
"""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
from decimal import Decimal
import json
from pathlib import Path
import re
import secrets
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
COMPOSE = ROOT / "docker-compose.a01.yml"
STATE_DIR = ROOT / ".a01"
SESSION = STATE_DIR / "session.json"
PASSWORD = "evemu-a01-local-only"  # exclusively isolated Docker DB, no host port
PROJECT_RE = re.compile(r"^evemu-a01-[a-z0-9][a-z0-9-]{0,36}$")
FIXTURE_CHARACTERS = (90010001, 90010002)
FIXTURE_CORPORATIONS = (98010001, 98010002)
SCOPES = ("characters", "corporation_wallets", "wallet_journal", "items", "offers", "market_orders", "contracts")


class StagingError(RuntimeError):
    pass


def validate_project(value: str) -> str:
    if not PROJECT_RE.fullmatch(value):
        raise StagingError("Compose project must be evemu-a01-<lowercase alphanumeric/hyphen>.")
    return value


def session() -> dict:
    if not SESSION.is_file():
        raise StagingError("No A01 session. Run `python tools/staging/a01.py init` first.")
    saved = json.loads(SESSION.read_text(encoding="utf-8"))
    validate_project(saved["project"])
    return saved


def run(cmd: list[str], *, stdin: str | None = None, capture: bool = False) -> str:
    proc = subprocess.run(cmd, input=stdin, text=True, capture_output=capture, check=False, cwd=ROOT)
    if proc.returncode != 0:
        detail = (proc.stderr or proc.stdout or "").strip() if capture else "See Docker output"
        raise StagingError(f"Command failed (exit {proc.returncode}): {cmd[0]} — {detail[:1000]}")
    return proc.stdout if capture else ""


def compose_cmd(project: str, *args: str, with_server: bool = False) -> list[str]:
    validate_project(project)
    # -p prevents accidental use of the repository's default/prod Compose project.
    cmd = ["docker", "compose", "-p", project, "-f", str(COMPOSE)]
    if with_server:
        cmd += ["--profile", "client"]
    return cmd + list(args)


def db_query(project: str, sql: str) -> list[str]:
    # Not a host database connection. No --host override accepted anywhere.
    cmd = compose_cmd(project, "exec", "-T", "-e", f"MYSQL_PWD={PASSWORD}",
                      "db", "mariadb", "-u", "evemu", "-D", "evemu", "--batch",
                      "--raw", "--skip-column-names", "--default-character-set=utf8mb4")
    result = run(cmd, stdin=sql, capture=True)
    return result.splitlines()


def ensure_ids(values: tuple[int, int], label: str) -> tuple[int, int]:
    if len(values) != 2 or values[0] == values[1] or not all(0 < x < 2**32 for x in values):
        raise StagingError(f"{label} must contain two different positive 32-bit IDs.")
    return values


def select_queries(prefix: str, chars: tuple[int, int], corps: tuple[int, int]) -> dict[str, str]:
    """Select only a finite allowlist of numeric columns from approved tables."""
    if prefix not in ("", "a01_"):
        raise StagingError("Invalid snapshot table prefix")
    charids = ",".join(map(str, ensure_ids(chars, "characters")))
    corpids = ",".join(map(str, ensure_ids(corps, "corporations")))
    def q(table: str, key: str, fields: str, where: str) -> str:
        return (f"SELECT CAST({key} AS CHAR), CONCAT_WS('|', {fields}) "
                f"FROM {prefix}{table} WHERE {where} ORDER BY {key};")
    return {
        "characters": q("chrCharacters", "characterID", "CAST(balance AS CHAR), CAST(corporationID AS CHAR)", f"characterID IN ({charids})"),
        "corporation_wallets": q("crpWalletDivisons", "corporationID",
            ", ".join(f"CAST(balance{n} AS CHAR)" for n in range(1, 8)), f"corporationID IN ({corpids})"),
        "wallet_journal": q("jnlCharacters", "transactionID",
            "CAST(ownerID AS CHAR), CAST(entryTypeID AS CHAR), CAST(referenceID AS CHAR), CAST(amount AS CHAR), CAST(balance AS CHAR)", f"ownerID IN ({charids})"),
        "items": q("entity", "itemID",
            "CAST(ownerID AS CHAR), CAST(locationID AS CHAR), CAST(typeID AS CHAR), CAST(flag AS CHAR), CAST(quantity AS CHAR)", f"ownerID IN ({charids})"),
        "offers": q("agtOffers", "offerID",
            "CAST(characterID AS CHAR), CAST(agentID AS CHAR), CAST(missionID AS CHAR), CAST(stateID AS CHAR), CAST(rewardISK AS CHAR), CAST(dateCompleted AS CHAR)", f"characterID IN ({charids})"),
        "market_orders": q("mktOrders", "orderID",
            "CAST(ownerID AS CHAR), CAST(typeID AS CHAR), CAST(price AS CHAR), CAST(volRemaining AS CHAR)", f"ownerID IN ({charids})"),
        "contracts": q("ctrContracts", "contractId",
            "CAST(issuerID AS CHAR), CAST(acceptorID AS CHAR), CAST(status AS CHAR), CAST(price AS CHAR), CAST(reward AS CHAR), CAST(collateral AS CHAR), CAST(crateID AS CHAR)",
            f"issuerID IN ({charids}) OR acceptorID IN ({charids})"),
    }


def snapshot(project: str, mode: str, chars: tuple[int, int], corps: tuple[int, int], *, build_sha: str = "") -> dict:
    prefix = "a01_" if mode == "fixture" else ""
    queries = select_queries(prefix, chars, corps)
    groups: dict[str, dict[str, str]] = {}
    for scope, statement in queries.items():
        rows: dict[str, str] = {}
        for line in db_query(project, statement):
            parts = line.split("\t", 1)
            if len(parts) != 2:
                raise StagingError(f"Malformed {scope} row (expected exactly two TSV columns).")
            key, value = parts
            if key in rows:
                raise StagingError(f"Duplicate {scope} primary key {key}: query is unsafe/inconsistent.")
            rows[key] = value
        groups[scope] = rows
    return {
        "format": "evemu-a01-v1", "mode": mode, "project": project,
        "captured_utc": datetime.now(timezone.utc).isoformat(), "build_sha": build_sha,
        "character_ids": list(chars), "corporation_ids": list(corps), "rows": groups,
    }


def diff(before: dict, after: dict) -> dict:
    for fld in ("format", "mode", "project", "character_ids", "corporation_ids"):
        if before.get(fld) != after.get(fld):
            raise StagingError(f"Cannot compare mismatched snapshots: {fld}")
    result: dict[str, dict] = {}
    for scope in SCOPES:
        old = before["rows"].get(scope, {})
        new = after["rows"].get(scope, {})
        result[scope] = {
            "added": {k: new[k] for k in sorted(new.keys() - old.keys())},
            "removed": {k: old[k] for k in sorted(old.keys() - new.keys())},
            "changed": {k: {"before": old[k], "after": new[k]}
                        for k in sorted(old.keys() & new.keys()) if old[k] != new[k]},
            "before_count": len(old), "after_count": len(new),
        }
    return result


def wallet_delta(before: dict, after: dict, character: int) -> Decimal:
    key = str(character)
    try:
        old = before["rows"]["characters"][key].split("|", 1)[0]
        new = after["rows"]["characters"][key].split("|", 1)[0]
    except KeyError as exc:
        raise StagingError(f"Character {character} missing from snapshot; create test characters first") from exc
    return Decimal(new) - Decimal(old)


def journal_added(before: dict, after: dict, character: int) -> int:
    changes = diff(before, after)["wallet_journal"]
    return sum(1 for value in changes["added"].values()
               if value.split("|", 1)[0] == str(character))


def write_snapshot(data: dict, label: str) -> Path:
    if not re.fullmatch(r"[a-zA-Z0-9][a-zA-Z0-9_-]{0,50}", label):
        raise StagingError("Label must contain only letters, numbers, underscores or hyphens")
    run_dir = STATE_DIR / "runs"
    run_dir.mkdir(parents=True, exist_ok=True)
    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%S%fZ")
    path = run_dir / f"{stamp}-{label}.json"
    path.write_text(json.dumps(data, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return path


def init(args: argparse.Namespace) -> None:
    if SESSION.exists():
        raise StagingError(f"Session already exists: {SESSION}. Stop/destroy it before initializing a new one.")
    project = validate_project(args.project or "evemu-a01-" + secrets.token_hex(5))
    STATE_DIR.mkdir(parents=True, exist_ok=True)
    SESSION.write_text(json.dumps({"project": project, "created_utc": datetime.now(timezone.utc).isoformat()}, indent=2) + "\n", encoding="utf-8")
    print(f"A01 isolated session initialized: {project}")
    print(f"State: {SESSION} (Git-ignored; keep it private)")


def up(args: argparse.Namespace) -> None:
    project = session()["project"]
    run(compose_cmd(project, "up", "-d", "--wait", "db"))
    if args.server:
        # Dockerfile builds server and EVEDBTool; startup runs install in this
        # unique, fresh DB only. No host DB credentials are ever accepted.
        run(compose_cmd(project, "up", "-d", "--build", "server", with_server=True))
        print("Server container started; full SDE installation may still be running.")
        print("Use `logs` to verify server startup before connecting the Windows client.")
    print(f"A01 staging services started for {project}. Database is NOT exposed to host.")


def cmd_snapshot(args: argparse.Namespace) -> None:
    project = session()["project"]
    if args.mode == "fixture":
        chars, corps = FIXTURE_CHARACTERS, FIXTURE_CORPORATIONS
    else:
        if None in (args.character_a, args.character_b, args.corporation_a, args.corporation_b):
            raise StagingError("Game snapshots require --character-a/b and --corporation-a/b (two disposable test identities).")
        chars, corps = (args.character_a, args.character_b), (args.corporation_a, args.corporation_b)
    shot = snapshot(project, args.mode, chars, corps, build_sha=args.build_sha or "")
    path = write_snapshot(shot, args.label)
    print(f"Read-only {args.mode} snapshot: {path}")
    for key in SCOPES:
        print(f"  {key}: {len(shot['rows'][key])} rows")


def cmd_compare(args: argparse.Namespace) -> None:
    before = json.loads(Path(args.before).read_text(encoding="utf-8"))
    after = json.loads(Path(args.after).read_text(encoding="utf-8"))
    changes = diff(before, after)
    for scope, change in changes.items():
        print(f"{scope}: +{len(change['added'])} / -{len(change['removed'])} / ~{len(change['changed'])} "+
              f"({change['before_count']} -> {change['after_count']})")
    for cid in before["character_ids"]:
        print(f"character {cid} ISK delta: {wallet_delta(before, after, cid)}")
    if args.expect_a_isk is not None:
        if wallet_delta(before, after, before["character_ids"][0]) != Decimal(args.expect_a_isk):
            raise StagingError("Character A ISK delta does not match expected amount")
    if args.expect_b_isk is not None:
        if wallet_delta(before, after, before["character_ids"][1]) != Decimal(args.expect_b_isk):
            raise StagingError("Character B ISK delta does not match expected amount")
    for cid, expected in ((before["character_ids"][0], args.expect_a_journal_entries),
                          (before["character_ids"][1], args.expect_b_journal_entries)):
        if expected is not None and journal_added(before, after, cid) != expected:
            raise StagingError(f"Character {cid} new journal row count differs from expected {expected}")
    if args.fail_on_removal and any(g["removed"] for g in changes.values()):
        raise StagingError("Unexpected row removal detected")
    path = write_snapshot({"before": args.before, "after": args.after, "changes": changes}, "diff")
    print(f"Exact row diff (private): {path}")
    print("Note: a diff reveals DB changes; it cannot by itself prove exactly-once client RPC behavior.")


def cmd_selftest(args: argparse.Namespace) -> None:
    project = session()["project"]
    source = ROOT / "sql" / "staging"
    db_query(project, (source / "a01_fixture.sql").read_text(encoding="utf-8"))
    before = snapshot(project, "fixture", FIXTURE_CHARACTERS, FIXTURE_CORPORATIONS)
    db_query(project, (source / "a01_fixture_transition.sql").read_text(encoding="utf-8"))
    after = snapshot(project, "fixture", FIXTURE_CHARACTERS, FIXTURE_CORPORATIONS)
    changes = diff(before, after)
    expected = {"characters": 1, "items": 1, "offers": 1, "market_orders": 1, "contracts": 1, "corporation_wallets": 0, "wallet_journal": 0}
    for scope, number in expected.items():
        if len(changes[scope]["changed"]) != number or changes[scope]["added"] or changes[scope]["removed"]:
            raise StagingError(f"Unexpected synthetic fixture transition for {scope}")
    if wallet_delta(before, after, FIXTURE_CHARACTERS[0]) != Decimal("25000.00"):
        raise StagingError("Synthetic character A delta failed")
    if wallet_delta(before, after, FIXTURE_CHARACTERS[1]) != Decimal("0.00"):
        raise StagingError("Synthetic character B balance changed")
    if len(changes["wallet_journal"]["added"]) != 1 or changes["wallet_journal"]["removed"]:
        raise StagingError("Expected exactly one new wallet journal entry")
    if journal_added(before, after, FIXTURE_CHARACTERS[0]) != 1 or journal_added(before, after, FIXTURE_CHARACTERS[1]) != 0:
        raise StagingError("Wallet journal ownership mismatch or duplicate payout")
    if before["rows"]["items"]["140010003"] != after["rows"]["items"]["140010003"]:
        raise StagingError("Synthetic character B inventory mutated")
    if before["rows"]["offers"]["41010002"] != after["rows"]["offers"]["41010002"]:
        raise StagingError("Synthetic character B mission mutated")
    if before["rows"]["market_orders"]["51010002"] != after["rows"]["market_orders"]["51010002"]:
        raise StagingError("Synthetic character B market mutated")
    if before["rows"]["corporation_wallets"] != after["rows"]["corporation_wallets"]:
        raise StagingError("Synthetic corporation wallets mutated")
    bp = write_snapshot(before, "fixture-before")
    ap = write_snapshot(after, "fixture-after")
    dp = write_snapshot({"format":"evemu-a01-diff-v1","changes":changes}, "fixture-diff")
    print("PASS: real MariaDB fixture, two characters/two corporations, five row changes and one journal insertion.")
    print(f"PASS: 25,000 ISK expected delta; character B wallet/inventory unchanged; no deleted/duplicate rows.")
    print(f"Evidence: {bp}, {ap}, {dp}")
    print("LIMITATION: synthetic tables do NOT exercise actual EVEmu RPC or persistence code.")


def cmd_logs(args: argparse.Namespace) -> None:
    project = session()["project"]
    out = run(compose_cmd(project, "logs", "--no-color", "--tail", str(args.tail), "server", with_server=True), capture=True)
    STATE_DIR.mkdir(parents=True, exist_ok=True)
    path = STATE_DIR / "runs" / (datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")+"-server.log")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(out, encoding="utf-8")
    print(f"Private server log: {path}")


def cmd_status(args: argparse.Namespace) -> None:
    project = session()["project"]
    print(f"A01 project: {project}")
    run(compose_cmd(project, "ps", "-a", with_server=True))


def cmd_stop(args: argparse.Namespace) -> None:
    project = session()["project"]
    run(compose_cmd(project, "down", with_server=True))  # preserve staging DB volumes
    print("Stopped A01 containers; project-specific test database volume preserved.")


def cmd_destroy(args: argparse.Namespace) -> None:
    project = session()["project"]
    if args.confirm != project:
        raise StagingError(f"To delete ONLY this staging project's volumes, pass --confirm {project}")
    run(compose_cmd(project, "down", "--volumes", "--remove-orphans", with_server=True))
    SESSION.unlink()
    print(f"Destroyed disposable project {project} and its volumes; local .a01/runs evidence retained.")


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="action", required=True)
    init_p = sub.add_parser("init", help="create a random isolated Docker project")
    init_p.add_argument("--project", help="optional unique name: evemu-a01-<token>")
    up_p = sub.add_parser("up", help="start isolated DB; optionally build/run the actual server")
    up_p.add_argument("--server", action="store_true", help="build and launch real EVEmu staging server")
    snap_p = sub.add_parser("snapshot", help="read-only snapshot of fixture or real server tables")
    snap_p.add_argument("--mode", choices=("fixture", "game"), required=True)
    snap_p.add_argument("--label", required=True)
    for k in ("character-a", "character-b", "corporation-a", "corporation-b"):
        snap_p.add_argument("--" + k, type=int)
    snap_p.add_argument("--build-sha", help="the server build SHA from BUILD-INFO.txt")
    compare_p = sub.add_parser("compare", help="compare snapshots by exact item/offer/order/contract IDs")
    compare_p.add_argument("--before", required=True)
    compare_p.add_argument("--after", required=True)
    compare_p.add_argument("--expect-a-isk")
    compare_p.add_argument("--expect-b-isk")
    compare_p.add_argument("--expect-a-journal-entries", type=int)
    compare_p.add_argument("--expect-b-journal-entries", type=int)
    compare_p.add_argument("--fail-on-removal", action="store_true")
    sub.add_parser("selftest", help="destructive ONLY to six a01_* synthetic tables in own A01 DB")
    logp = sub.add_parser("logs", help="save private server logs for troubleshooting")
    logp.add_argument("--tail", type=int, default=500)
    sub.add_parser("status", help="show only this staging project's Docker containers")
    sub.add_parser("stop", help="stop staging containers, preserving volume")
    dest = sub.add_parser("destroy", help="remove only this A01 project's Docker volumes")
    dest.add_argument("--confirm", required=True, help="type the exact A01 project name")
    return p.parse_args()


def main() -> int:
    args = parse_args()
    try:
        {"init": init, "up": up, "snapshot": cmd_snapshot, "compare": cmd_compare,
         "selftest": cmd_selftest, "logs": cmd_logs, "status": cmd_status,
         "stop": cmd_stop, "destroy": cmd_destroy}[args.action](args)
    except (StagingError, OSError, ValueError, KeyError) as exc:
        print(f"A01 ERROR: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
