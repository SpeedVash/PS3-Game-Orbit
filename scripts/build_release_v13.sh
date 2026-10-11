#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
./scripts/preflight_v13.sh
make clean
make -j"${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)}" self
make pkg

for f in ps3_sp_loader.elf ps3_sp_loader.self ps3_sp_loader.pkg ps3_sp_loader.gnpdrm.pkg; do
  [ -s "$f" ] || { echo "ERROR: expected build artifact missing: $f" >&2; exit 5; }
done
for f in build/pkg/USRDIR/EBOOT.BIN build/pkg/PARAM.SFO build/pkg/ICON0.PNG          build/pkg/USRDIR/FULL_COVER_TEST_275x147.png; do
  [ -s "$f" ] || { echo "ERROR: expected PKG staging file missing: $f" >&2; exit 6; }
done

# The finalized NPDRM package is the user-facing install package.
cp -f ps3_sp_loader.gnpdrm.pkg PS3_SP_LOADER_V13.pkg
sha256sum ps3_sp_loader.elf ps3_sp_loader.self build/pkg/USRDIR/EBOOT.BIN           ps3_sp_loader.pkg ps3_sp_loader.gnpdrm.pkg PS3_SP_LOADER_V13.pkg           > BUILD_SHA256_V13.txt

printf '\nBuild complete. Install package: PS3_SP_LOADER_V13.pkg\n'
cat BUILD_SHA256_V13.txt
