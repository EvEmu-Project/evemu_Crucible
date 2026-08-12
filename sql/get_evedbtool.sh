#!/bin/bash

set -Eeuo pipefail

readonly EVEDBTOOL_VERSION="0.0.6"
readonly EVEDBTOOL_BASE_URL="https://github.com/EvEmu-Project/EVEDBTool/releases/download"
readonly X86_64_SHA256="a78b9ae56660a092bccb4dd82e4d3857a2a6bd7cfd2273c89899011bf4a05d10"
readonly AARCH64_SHA256="d091161e0dcc51f4bea8e4106a0959b4d8f5c02e519d86fc93ece8b20f90361a"

fail() {
    printf 'get_evedbtool: %s\n' "$1" >&2
    exit 1
}

command -v curl >/dev/null 2>&1 || fail "curl is required"
command -v sha256sum >/dev/null 2>&1 || fail "sha256sum is required"

case "$(uname -m)" in
    aarch64)
        asset="evedb_aarch64"
        expected_sha256="$AARCH64_SHA256"
        ;;
    x86_64)
        asset="evedbtool"
        expected_sha256="$X86_64_SHA256"
        ;;
    *)
        fail "unsupported architecture: $(uname -m)"
        ;;
esac

download_url="${EVEDBTOOL_BASE_URL}/${EVEDBTOOL_VERSION}/${asset}"
temporary_file="$(mktemp "evedbtool.tmp.XXXXXX")"
trap 'rm -f "$temporary_file"' EXIT

curl \
    --fail \
    --silent \
    --show-error \
    --location \
    --proto '=https' \
    --tlsv1.2 \
    --output "$temporary_file" \
    "$download_url"

actual_sha256="$(sha256sum "$temporary_file" | awk '{print $1}')"
[[ "$actual_sha256" == "$expected_sha256" ]] || \
    fail "download checksum mismatch"

chmod 0755 "$temporary_file"
mv -f "$temporary_file" evedbtool
trap - EXIT
