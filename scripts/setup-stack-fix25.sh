#!/usr/bin/env bash
set -euo pipefail
variant=${1:?B required}
stack_root=${2:?absolute stack directory required}
[[ "$stack_root" = /* ]] || { echo 'Stack path must be absolute'; exit 1; }
export PS3DEV="$stack_root/ps3dev"
export PSL1GHT="$PS3DEV"
export PATH="$PS3DEV/bin:$PS3DEV/ppu/bin:$PS3DEV/spu/bin:$PATH"
mkdir -p "$stack_root"
case "$variant" in
  B)
    mkdir -p "$PS3DEV/bin" "$PS3DEV/spu/bin"
    toolchain_commit=961fddac01337f18da08f4471d558a7a5b0d9af2
    sdk_commit=af9d3d964c8faa69abce4961a269f9582e05a33f
    git init "$stack_root/toolchain"
    git -C "$stack_root/toolchain" remote add origin https://github.com/ps3aqua/ps3toolchain.git
    git -C "$stack_root/toolchain" fetch --depth 1 origin "$toolchain_commit"
    git -C "$stack_root/toolchain" checkout --detach FETCH_HEAD
    # Public HTTPS GNU mirror; no change to compiler patches or versions.
    sed -i 's|https://ftp.gnu.org/gnu/|https://mirrors.kernel.org/gnu/|g' "$stack_root/toolchain"/scripts/*.sh
    # Honor the CPUs assigned to this runner; /proc/cpuinfo can expose the host.
    export FIX16_BUILD_JOBS="${FIX16_BUILD_JOBS:-$(nproc)}"
    python3 - "$stack_root/toolchain" <<'PY'
from pathlib import Path
import sys
root = Path(sys.argv[1])
for name in ('001-binutils-PPU.sh', '002-gcc-newlib-PPU.sh'):
    p = root / 'scripts' / name
    s = p.read_text()
    begin = s.index('PROCS="$(grep')
    end = s.index('${MAKE:-make}', begin)
    s = s[:begin] + 'PROCS="$FIX16_BUILD_JOBS"\n' + s[end:]
    if name.startswith('002-'):
        s = s.replace('if [ ! -f ${NEWLIB}.tar.gz ]',
                      'if [ ! -f newlib-${NEWLIB}.tar.gz ]')
        s = s.replace('  ./contrib/download_prerequisites',
            "  sed -i 's|ftp://gcc.gnu.org/pub/gcc/infrastructure/|https://gcc.gnu.org/pub/gcc/infrastructure/|g' contrib/download_prerequisites\n  ./contrib/download_prerequisites")
    p.write_text(s)
PY
    # Only PPU is needed by this probe: binutils, GCC/newlib, aliases.
    (cd "$stack_root/toolchain" && ./toolchain.sh 1 2 4)
    git init "$stack_root/sdk"
    git -C "$stack_root/sdk" remote add origin https://github.com/ps3aqua/PSL1GHT.git
    git -C "$stack_root/sdk" fetch --depth 1 origin "$sdk_commit"
    git -C "$stack_root/sdk" checkout --detach FETCH_HEAD
    make -C "$stack_root/sdk" install-ctrl
    make -C "$stack_root/sdk/ppu" -j"$FIX16_BUILD_JOBS"
    make -C "$stack_root/sdk/ppu" install
    # Build only ELF/SELF/PKG tools used here. No SPU, Cg, SDL or portlibs.
    for component in geohot sprxlinker generic ps3py; do
      CC=gcc LDSHARED='gcc -shared' make -C "$stack_root/sdk/tools/$component" -j"$FIX16_BUILD_JOBS"
      CC=gcc LDSHARED='gcc -shared' make -C "$stack_root/sdk/tools/$component" install
    done
    printf 'ps3aqua toolchain %s\nPSL1GHT %s\n' "$toolchain_commit" "$sdk_commit" > "$PS3DEV/build.txt"
    ;;
  *) echo 'FIX25 requires the ps3aqua B stack'; exit 1 ;;
esac
ppu-gcc --version
test -f "$PSL1GHT/ppu/lib/librsx.a"
test -f "$PSL1GHT/ppu/lib/libgcm_sys.a"
test -f "$PSL1GHT/ppu/lib/libsysutil.a"
# Marker is written only after the complete installation succeeds.
printf '%s\n' "$variant" > "$PS3DEV/FIX16_STACK_OK"
