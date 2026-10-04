#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
cd "$root"
python3 scripts/prepare-jfx.py
bash scripts/build-fix31.sh
mkdir -p dist/release
cp dist/B/PS3_GAME_ORBIT_FIX31.gnpdrm.pkg dist/release/PS3_GAME_ORBIT_v1.1_TESTE.gnpdrm.pkg
(cd dist/release && sha256sum PS3_GAME_ORBIT_v1.1_TESTE.gnpdrm.pkg > SHA256SUMS.txt)
printf 'Built PS3 Game Orbit 1.1 test: dist/release/PS3_GAME_ORBIT_v1.1_TESTE.gnpdrm.pkg\n'
