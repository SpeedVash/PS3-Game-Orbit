#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
cd "$root"
python3 scripts/prepare-jfx.py
bash scripts/build-fix44.sh
mkdir -p dist/release
cp dist/B/PS3_GAME_ORBIT_FIX44.gnpdrm.pkg dist/release/PS3_GAME_ORBIT_v1.5.4.gnpdrm.pkg
(cd dist/release && sha256sum PS3_GAME_ORBIT_v1.5.4.gnpdrm.pkg > SHA256SUMS.txt)
printf 'Built PS3 Game Orbit 1.5.4: dist/release/PS3_GAME_ORBIT_v1.5.4.gnpdrm.pkg\n'
