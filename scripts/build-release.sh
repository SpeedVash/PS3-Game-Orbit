#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
cd "$root"
python3 scripts/prepare-jfx.py
bash scripts/build-fix43.sh
mkdir -p dist/release
cp dist/B/PS3_GAME_ORBIT_FIX43.gnpdrm.pkg dist/release/PS3_GAME_ORBIT_v1.5.3.gnpdrm.pkg
(cd dist/release && sha256sum PS3_GAME_ORBIT_v1.5.3.gnpdrm.pkg > SHA256SUMS.txt)
printf 'Built PS3 Game Orbit 1.5.3: dist/release/PS3_GAME_ORBIT_v1.5.3.gnpdrm.pkg\n'
