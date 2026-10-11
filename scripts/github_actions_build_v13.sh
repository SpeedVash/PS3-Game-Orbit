#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

export PS3DEV="${PS3DEV:-/usr/local/ps3dev}"
export PSL1GHT="${PSL1GHT:-$PS3DEV}"
export PATH="$PS3DEV/bin:$PS3DEV/ppu/bin:$PS3DEV/spu/bin:$PATH"
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH:-/usr/lib64:/usr/local/lib64:/usr/lib/x86_64-linux-gnu}"

printf 'PS3_SP_LOADER V13 - GitHub Actions build\n'
printf 'PS3DEV=%s\nPSL1GHT=%s\n' "$PS3DEV" "$PSL1GHT"
command -v ppu-g++
command -v cgcomp
command -v cgc
ppu-g++ --version | head -1

# V14 must never silently change.
./scripts/verify_v14_frozen_v13.sh

# Compile fresh shaders in CI, so the build proves that the shader pipeline works.
rm -f shaders/v14_case.vpo shaders/v14_case.fpo shaders/v14_case.vpo.o shaders/v14_case.fpo.o
export SHADER_PIPELINE=asm

./scripts/build_release_v13.sh

# Final package sanity checks.
for f in \
  PS3_SP_LOADER_V13.pkg \
  ps3_sp_loader.elf \
  ps3_sp_loader.self \
  ps3_sp_loader.gnpdrm.pkg \
  build/pkg/USRDIR/EBOOT.BIN \
  build/pkg/PARAM.SFO \
  shaders/v14_case.vpo \
  shaders/v14_case.fpo \
  BUILD_SHA256_V13.txt; do
  test -s "$f" || { echo "ERROR: missing final artifact: $f" >&2; exit 20; }
done

printf '\nV13 GitHub Actions PS3 build: OK\n'
