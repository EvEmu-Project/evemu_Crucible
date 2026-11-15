#!/usr/bin/env python3
"""Build an OFFLINE EVEmu Crucible agent portrait cache.

This is an explicit, one-time operator utility. The game server itself never
downloads station-agent portraits. CCP's historic portraits may be absent; in
that case the server uses the separately bundled, clearly generic JPEG.
Only official images.evetech.net JPEG responses are accepted.
"""
from __future__ import annotations

import argparse
import gzip
import hashlib
import json
import re
import sys
import time
from pathlib import Path
from urllib.error import HTTPError, URLError
from urllib.parse import urlparse
from urllib.request import HTTPRedirectHandler, Request, build_opener

MIN_AGENT_ID = 3_000_000
MAX_AGENT_ID = 3_999_999
ALLOWED_SIZES = (64, 128, 256, 512)
BASE_URL = "https://images.evetech.net"
ROW_ID_PATTERN = re.compile(r"\(\s*(3\d{6})\s*,")
LOOSE_ID_PATTERN = re.compile(r"(?<!\d)(3\d{6})(?!\d)")


def source_and_cache_roots(script_file: Path) -> tuple[Path, Path]:
    source = script_file.resolve().parents[2]
    # Installed archive: stage/share/evemu/tools/agent_portraits/...
    if source.name == "evemu" and source.parent.name == "share":
        return source, source.parent.parent
    return source, source


def agent_ids_from_dump(path: Path) -> list[int]:
    """Read the packaged Crucible agent table without requiring MariaDB."""
    with gzip.open(path, "rt", encoding="utf-8", errors="replace") as file:
        text = file.read()
    ids = {int(match.group(1)) for match in ROW_ID_PATTERN.finditer(text)}
    # Support old dump layouts whose rows are separated differently.
    if not ids:
        ids = {int(match.group(1)) for match in LOOSE_ID_PATTERN.finditer(text)}
    ids = {id_ for id_ in ids if MIN_AGENT_ID <= id_ <= MAX_AGENT_ID}
    if not ids:
        raise ValueError("No Crucible NPC agent IDs found in " + str(path))
    return sorted(ids)


def agent_ids_from_text(path: Path) -> list[int]:
    ids = set()
    for line in path.read_text(encoding="utf-8-sig").splitlines():
        line = line.split("#", 1)[0].strip()
        if not line:
            continue
        value = int(line)
        if not MIN_AGENT_ID <= value <= MAX_AGENT_ID:
            raise ValueError("Not a legacy NPC agent ID: " + str(value))
        ids.add(value)
    return sorted(ids)


def is_jpeg(data: bytes) -> bool:
    return len(data) >= 128 and data.startswith(b"\xff\xd8") and data.endswith(b"\xff\xd9")


class OfficialCDNOnly(HTTPRedirectHandler):
    def redirect_request(self, req, fp, code, msg, headers, newurl):
        url = urlparse(newurl)
        if url.scheme != "https" or url.hostname != "images.evetech.net":
            raise ValueError("Refusing portrait redirect outside official HTTPS CDN")
        return super().redirect_request(req, fp, code, msg, headers, newurl)


def fetch_portrait(opener, agent_id: int, size: int, timeout: float) -> bytes:
    url = f"{BASE_URL}/characters/{agent_id}/portrait?size={size}"
    request = Request(url, headers={
        "User-Agent": "EVEmu-Crucible-Offline-Agent-Cache/1.0 (one-time operator sync)",
        "Accept": "image/jpeg",
    })
    with opener.open(request, timeout=timeout) as response:
        data = response.read(2_000_001)
        if len(data) > 2_000_000:
            raise ValueError("Oversized image response: " + str(agent_id))
        if "jpeg" not in response.headers.get("Content-Type", "").lower() or not is_jpeg(data):
            raise ValueError("Not a valid JPEG portrait: " + str(agent_id))
        return data


def cached_portrait(directory: Path, agent_id: int) -> Path | None:
    for size in (512, 256, 128, 64, 50, 40, 32):
        p = directory / f"{agent_id}_{size}.jpg"
        if p.is_file() and is_jpeg(p.read_bytes()):
            return p
    return None


def save_atomic(path: Path, data: bytes):
    path.parent.mkdir(parents=True, exist_ok=True)
    temp = path.with_name(path.name + ".tmp")
    try:
        temp.write_bytes(data)
        temp.replace(path)
    finally:
        if temp.exists():
            temp.unlink()


def main(argv: list[str] | None = None) -> int:
    source_root, install_root = source_and_cache_roots(Path(__file__))
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sql-dump", type=Path,
                        default=source_root / "sql/base/agtAgents.sql.gz",
                        help="Crucible SQL agent archive (used without a running database)")
    parser.add_argument("--ids-file", type=Path, help="Optional newline-separated agent IDs")
    parser.add_argument("--output", type=Path,
                        default=install_root / "image_cache/Character",
                        help="Image server's configured image_cache/Character directory")
    parser.add_argument("--size", type=int, choices=ALLOWED_SIZES, default=256,
                        help="CDN JPEG size to store; 256px works for Crucible 50px requests")
    parser.add_argument("--limit", type=int, default=0,
                        help="Limit agent count for a trial run (default: all)")
    parser.add_argument("--delay", type=float, default=0.25,
                        help="Seconds between CDN requests (default: 0.25)")
    parser.add_argument("--timeout", type=float, default=10,
                        help="Seconds per CDN request")
    parser.add_argument("--max-errors", type=int, default=30,
                        help="Stop after this many network/invalid-image errors")
    parser.add_argument("--verify-only", action="store_true",
                        help="Count cached portraits without making network requests")
    args = parser.parse_args(argv)
    if args.limit < 0 or args.delay < 0 or args.timeout <= 0 or args.max_errors < 1:
        parser.error("Invalid limit, delay, timeout, or max-errors")
    try:
        ids = agent_ids_from_text(args.ids_file) if args.ids_file else agent_ids_from_dump(args.sql_dump)
    except (OSError, ValueError) as exc:
        parser.error(str(exc))
    if args.limit:
        ids = ids[:args.limit]
    args.output.mkdir(parents=True, exist_ok=True)
    report = {"total_agents": len(ids), "local_existing": 0, "downloaded": 0,
              "unavailable": 0, "errors": 0, "no_network_runtime": True}
    opener = build_opener(OfficialCDNOnly)
    generic_digest = None
    if not args.verify_only:
        try:
            generic = fetch_portrait(opener, 1, args.size, args.timeout)
            generic_digest = hashlib.sha256(generic).digest()
        except (HTTPError, URLError, OSError, ValueError) as exc:
            print("Unable to identify CCP default portrait; skipping generic-image check:", exc,
                  file=sys.stderr)

    for index, agent_id in enumerate(ids, 1):
        if cached_portrait(args.output, agent_id):
            report["local_existing"] += 1
            continue
        if args.verify_only:
            report["unavailable"] += 1
            continue
        try:
            data = fetch_portrait(opener, agent_id, args.size, args.timeout)
            if generic_digest is not None and hashlib.sha256(data).digest() == generic_digest:
                report["unavailable"] += 1
                continue
            save_atomic(args.output / f"{agent_id}_{args.size}.jpg", data)
            report["downloaded"] += 1
        except HTTPError as exc:
            if exc.code in (404, 410):
                report["unavailable"] += 1
            else:
                report["errors"] += 1
                print(f"{agent_id}: HTTP {exc.code}", file=sys.stderr)
        except (URLError, OSError, ValueError) as exc:
            report["errors"] += 1
            print(f"{agent_id}: {exc}", file=sys.stderr)
        if index % 100 == 0:
            print(f"Processed {index}/{len(ids)} agents; cached {report['local_existing'] + report['downloaded']}")
        if report["errors"] >= args.max_errors:
            print("Too many errors; stopping instead of overwhelming the image service.",
                  file=sys.stderr)
            break
        time.sleep(args.delay)

    report["missing_agent_portraits_use_local_generic_fallback"] = True
    print(json.dumps(report, indent=2))
    return 1 if report["errors"] >= args.max_errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
