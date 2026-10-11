#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
: "${PS3DEV:?Set PS3DEV, e.g. /usr/local/ps3dev}"
: "${PSL1GHT:=$PS3DEV}"
export PSL1GHT
export PATH="$PS3DEV/bin:$PS3DEV/ppu/bin:$PS3DEV/spu/bin:$PATH"
./scripts/preflight_v13.sh
make clean
make -j"${JOBS:-2}" self
printf '\nBuilt SELF: %s/ps3_sp_loader.self\n' "$ROOT"
