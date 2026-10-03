#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
: "${PS3DEV:?PS3DEV required}"
: "${PSL1GHT:?PSL1GHT required}"
export PATH="$PS3DEV/bin:$PS3DEV/ppu/bin:$PATH"
[[ "$(realpath "$PS3DEV")" = "$(realpath "$PSL1GHT")" ]] || {
  echo 'Use one complete ps3aqua stack; PS3DEV and PSL1GHT must match'; exit 1;
}
[[ "$(ppu-g++ -dumpversion)" = 7.5.0 ]] || { echo 'FIX28 requires GCC 7.5.0'; exit 1; }
rg -q '961fddac01337f18da08f4471d558a7a5b0d9af2' "$PS3DEV/build.txt"
rg -q 'af9d3d964c8faa69abce4961a269f9582e05a33f' "$PS3DEV/build.txt"
cd "$root"
mkdir -p dist/B
./scripts/verify_v14_frozen_v13.sh
sha256sum -c shaders/FROZEN_SHADER_SHA256.txt
{
  printf 'PS3_SP_LOADER FIX28 DUAS CAPAS; ps3aqua B stack\n'
  ppu-g++ --version
  cat "$PS3DEV/build.txt"
  sha256sum "$PS3DEV"/ppu/lib/lib{rsx,gcm_sys,rt,net,io,sysutil,sysmodule,jpgdec,pngdec,lv2}.a
  sha256sum "$(ppu-g++ -print-file-name=libm.a)" "$(ppu-g++ -print-file-name=libc.a)" "$(ppu-g++ -print-file-name=libstdc++.a)"
} > dist/B/BUILD_ENV.txt
python3 - <<'PY'
from pathlib import Path
import hashlib
paths = [Path('Makefile'), Path('.github/workflows/build-ps3.yml')]
for folder in ('include', 'src', 'shaders', 'scripts', 'tests', 'pkgfiles'):
    paths += [p for p in Path(folder).rglob('*') if p.is_file() and p.suffix not in ('.o', '.d')]
Path('dist/B/SOURCE_SHA256.txt').write_text(''.join(
    hashlib.sha256(p.read_bytes()).hexdigest() + '  ' + p.as_posix() + '\n'
    for p in sorted(paths)))
PY
make clean VERBOSE=1 2>&1 | tee dist/B/BUILD_LOG.txt
make -j"${FIX28_BUILD_JOBS:-4}" pkg VERBOSE=1 2>&1 | tee -a dist/B/BUILD_LOG.txt
for ext in elf self gnpdrm.pkg; do
  cp "ps3_sp_loader_fix28_duas_capas.$ext" "dist/B/PS3_SP_LOADER_FIX28_DUAS_CAPAS.$ext"
done
cp build/pkg/USRDIR/EBOOT.BIN build/pkg/PARAM.SFO dist/B/
cp ps3_sp_loader_fix28_duas_capas.map dist/B/renderer.map
ppu-nm -u src/main.o src/rsx_present_fix20.o src/rsx_renderer_v10.o src/rsx_stage1.o src/image_decode.o src/rsx_command_stream_fix25.o > dist/B/NATIVE_OBJECT_SYMBOLS.txt
ppu-objdump -dr src/main.o src/rsx_present_fix20.o src/rsx_command_stream_fix25.o > dist/B/NATIVE_PRESENT_DISASSEMBLY.txt
python3 scripts/verify-build-fix28.py dist/B | tee dist/B/PACKAGE_CHECK.txt
python3 scripts/check-presentation-fix20.py | tee dist/B/HOST_PRESENTATION_CHECK.txt
python3 scripts/check-renderer-fix28.py | tee dist/B/HOST_RENDERER_CHECK.txt
python3 scripts/check-library-loop-fix28.py | tee dist/B/HOST_LIBRARY_LOOP_CHECK.txt
python3 scripts/check-command-stream-fix25.py | tee dist/B/HOST_COMMAND_STREAM_CHECK.txt
(cd dist/B && sha256sum ./*.elf ./*.self ./*.pkg EBOOT.BIN PARAM.SFO > SHA256SUMS.txt)
