#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
cd "$root"
python3 scripts/prepare-jfx.py
bash scripts/build-fix36.sh
mkdir -p dist/release
cp dist/B/PS3_GAME_ORBIT_FIX36.gnpdrm.pkg dist/release/PS3_GAME_ORBIT_v1.3.3_TESTE.gnpdrm.pkg
(cd dist/release && sha256sum PS3_GAME_ORBIT_v1.3.3_TESTE.gnpdrm.pkg > SHA256SUMS.txt)
printf 'Built PS3 Game Orbit 1.3.3 test: dist/release/PS3_GAME_ORBIT_v1.3.3_TESTE.gnpdrm.pkg\n'
