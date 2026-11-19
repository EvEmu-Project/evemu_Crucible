#!/usr/bin/env bash
set -euo pipefail

# Build the repository-patched EVEDBTool from pinned upstream 0.0.6 source.
for command in git go; do
    if ! command -v "$command" >/dev/null 2>&1; then
        echo "ERROR: $command is required to build EVEDBTool." >&2
        exit 1
    fi
done

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

case "$(uname -m)" in
    x86_64) GOARCH=amd64 ;;
    aarch64|arm64) GOARCH=arm64 ;;
    *)
        echo "ERROR: Unsupported architecture: $(uname -m)" >&2
        exit 1
        ;;
esac

git clone --quiet --depth 1 --branch 0.0.6 https://github.com/EvEmu-Project/EVEDBTool.git "$WORK/EVEDBTool"
cp "$ROOT/tools/evedbtool/command_seed.go" "$WORK/EVEDBTool/command_seed.go"

(
    cd "$WORK/EVEDBTool"
    CGO_ENABLED=0 GOOS=linux GOARCH="$GOARCH" go build -trimpath \
        -ldflags "-s -w -X main.version=evemu-crucible" \
        -o "$ROOT/sql/evedbtool" .
)

chmod +x "$ROOT/sql/evedbtool"
echo "Patched EVEDBTool built at $ROOT/sql/evedbtool"
echo "Run it from the sql directory so it can find evedb.yaml and migrations."
