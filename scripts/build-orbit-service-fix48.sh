#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
: "${PS3DEV:?PS3DEV required}"
: "${PSL1GHT:?PSL1GHT required}"
export PATH="$PS3DEV/bin:$PS3DEV/ppu/bin:$PATH"
[[ "$(ppu-gcc -dumpversion)" = 7.5.0 ]] || { echo 'FIX48 requires GCC 7.5.0'; exit 1; }
cd "$root"
mkdir -p build/service pkgfiles/USRDIR/empty
if ! command -v make_self_sprx >/dev/null; then
  : "${PSL1GHT_SOURCE:?Set PSL1GHT_SOURCE to the pinned SDK source to build make_self_sprx}"
  cc -O2 -DSPRX "$PSL1GHT_SOURCE/tools/geohot/make_self.c" -lgmp -lcrypto -lz -o build/service/make_self_sprx
  signer="$root/build/service/make_self_sprx"
else
  signer=$(command -v make_self_sprx)
fi
opts=(-std=gnu99 -O2 -mcpu=cell -m64 -fPIC -mminimal-toc -ffreestanding -fno-builtin
  -fno-stack-protector -fno-asynchronous-unwind-tables -Wall -Wextra -Werror
  -Iinclude -I"$PSL1GHT/ppu/include")
for input in service fan_policy raw_reader; do
  ppu-gcc "${opts[@]}" -c "plugin/$input.c" -o "build/service/$input.o"
done
ppu-gcc -m64 -mcpu=cell -c plugin/module.S -o build/service/module.o
ppu-ld --emit-relocs -T plugin/orbit_prx.ld -Map build/service/orbit_service.map \
  build/service/service.o build/service/fan_policy.o build/service/raw_reader.o build/service/module.o \
  -o build/service/orbit_service.linked.elf
python3 scripts/make-orbit-prx-fix48.py build/service/orbit_service.linked.elf build/service/orbit_service.prx
"$signer" build/service/orbit_service.prx pkgfiles/USRDIR/orbit_service.sprx
python3 scripts/verify-orbit-prx-fix48.py build/service/orbit_service.prx pkgfiles/USRDIR/orbit_service.sprx
