#!/usr/bin/env bash
set -euo pipefail
PSL="${PSL1GHT:-${PS3DEV:-}}"
if [ -z "$PSL" ]; then echo '[FAIL] PSL1GHT/PS3DEV is not set' >&2; exit 2; fi
INC="$PSL/ppu/include"
CMD="$INC/rsx/commands_inc.h"
PROG="$INC/rsx/rsx_program.h"
PNG="$INC/pngdec/pngdec.h"
JPG="$INC/jpgdec/jpgdec.h"
RULES="$PSL/ppu_rules"
fail=0
ok(){ printf '[OK]   %s\n' "$*"; }
bad(){ printf '[FAIL] %s\n' "$*"; fail=1; }
need_file(){ [ -f "$1" ] && ok "$(basename "$1") present" || bad "missing $1"; }
for f in "$CMD" "$PROG" "$PNG" "$JPG" "$RULES"; do need_file "$f"; done
[ "$fail" -eq 0 ] || exit 3
check(){ local file="$1" pattern="$2" label="$3"; grep -Eq "$pattern" "$file" && ok "$label" || bad "$label"; }
check "$CMD" 'SetFragmentProgramParameter.*u32 offset,u32 location' 'fragment constant API has offset + location'
check "$CMD" 'BindVertexArrayAttrib.*u16 frequency,u32 offset,u8 stride,u8 elems,u8 dtype,u8 location' 'vertex attrib API has frequency + offset'
check "$CMD" 'DrawIndexArray.*u8 type,u32 offset,u32 count,u8 data_type,u8 location' 'indexed draw API matches renderer'
check "$PROG" 'rsxVertexProgramGetAttrib.*const char \*name' 'vertex attribute lookup API present'
check "$PROG" 'u32[[:space:]]+index' 'program attribute descriptor exposes index'
check "$PROG" 'rsxVertexProgramGetConst.*const char \*name' 'vertex constant lookup API present'
check "$PROG" 'rsxFragmentProgramGetConst.*const char \*name' 'fragment constant lookup API present'
check "$PNG" 'pngLoadFromBuffer.*u32 size,pngData \*out' 'PNG buffer decoder API present'
check "$JPG" 'jpgLoadFromBuffer.*u32 size,jpgData \*out' 'JPEG buffer decoder API present'
check "$RULES" '%\.vpo: %\.vcg' 'ppu_rules VCG -> VPO rule present'
check "$RULES" '%\.fpo: %\.fcg' 'ppu_rules FCG -> FPO rule present'
check "$RULES" 'SELF_NPDRM' 'ppu_rules NPDRM SELF packaging path present'
check "$RULES" 'PACKAGE_FINALIZE' 'ppu_rules package finalizer path present'

# Compile-probe the exact attribute contract used by the V13 renderer.
PPUGXX="${PS3DEV:-$PSL}/ppu/bin/ppu-g++"
if [ -x "$PPUGXX" ]; then
  tmp="$(mktemp -d)"
  trap 'rm -rf "$tmp"' EXIT
  cat > "$tmp/rsx_attr_probe.cpp" <<'CPP'
#include <rsx/rsx.h>
static int probe(rsxVertexProgram* vp) {
    const rsxProgramAttrib* a = rsxVertexProgramGetAttrib(vp, "position");
    return a ? static_cast<int>(a->index) : -1;
}
CPP
  probe_log="$tmp/rsx_attr_probe.log"
  if "$PPUGXX" -std=gnu++11 -I"$INC" -c "$tmp/rsx_attr_probe.cpp" -o "$tmp/rsx_attr_probe.o" >"$probe_log" 2>&1; then
    ok 'vertex attribute lookup returns descriptor with usable index'
  else
    bad 'vertex attribute descriptor compile probe failed'
    echo '----- ppu-g++ probe diagnostics -----' >&2
    cat "$probe_log" >&2
    echo '-------------------------------------' >&2
  fi
else
  bad "ppu-g++ unavailable for RSX attribute compile probe: $PPUGXX"
fi

if [ "$fail" -ne 0 ]; then
  echo 'BLOCKED: installed PSL1GHT API differs from the V13 audited interface.' >&2
  exit 4
fi
echo 'READY: installed PSL1GHT API matches the interfaces used by PS3_SP_LOADER V13.'
