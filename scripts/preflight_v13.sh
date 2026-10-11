#!/usr/bin/env bash
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
fail=0
ok(){ printf '[OK]    %s\n' "$*"; }
miss(){ printf '[MISS]  %s\n' "$*"; fail=1; }
printf 'PS3_SP_LOADER V13 preflight\nProject: %s\nPS3DEV=%s\nPSL1GHT=%s\n' "$ROOT" "${PS3DEV:-<unset>}" "${PSL1GHT:-<unset>}"
if [ -z "${PS3DEV:-}" ]; then miss 'PS3DEV is not set'; else ok 'PS3DEV set'; fi
if [ -z "${PSL1GHT:-}" ]; then miss 'PSL1GHT is not set'; else ok 'PSL1GHT set'; fi
if [ -n "${PS3DEV:-}" ] && [ ! -d "$PS3DEV" ]; then miss "PS3DEV directory not found: $PS3DEV"; fi
if [ -n "${PSL1GHT:-}" ] && [ ! -f "$PSL1GHT/ppu_rules" ]; then miss 'PSL1GHT/ppu_rules not found'; fi
check_tool(){ if command -v "$1" >/dev/null 2>&1; then ok "$1 -> $(command -v "$1")"; else miss "$1"; fi; }
for t in ppu-g++ ppu-as ppu-strip sprxlinker make_self fself make_self_npdrm pkg package_finalize sfo bin2s; do check_tool "$t"; done
if command -v cgcomp >/dev/null 2>&1; then
  ok "cgcomp -> $(command -v cgcomp)"
  command -v cgc >/dev/null 2>&1 && ok "cgc -> $(command -v cgc)" || printf '[INFO]  cgc not found; direct cgcomp path will be used\n'
elif [ -s "$ROOT/shaders/v14_case.vpo" ] && [ -s "$ROOT/shaders/v14_case.fpo" ]; then
  ok 'precompiled VPO/FPO present; Cg compiler not required for this build'
else
  miss 'cgcomp missing and shaders/v14_case.vpo + .fpo are not precompiled'
fi
[ -f "$ROOT/pkgfiles/ICON0.PNG" ] && ok 'pkgfiles/ICON0.PNG present' || miss 'pkgfiles/ICON0.PNG missing'
[ -f "$ROOT/pkgfiles/USRDIR/FULL_COVER_TEST_275x147.png" ] && ok 'Safe Boot full cover present in pkgfiles/USRDIR' || miss 'Safe Boot full cover missing'
[ -f "$ROOT/shaders/v14_case.vcg" ] && ok 'vertex shader source present' || miss 'vertex shader source missing'
[ -f "$ROOT/shaders/v14_case.fcg" ] && ok 'fragment shader source present' || miss 'fragment shader source missing'
[ -x "$ROOT/scripts/compile_shader_v13.sh" ] && ok 'shader compiler wrapper present' || miss 'shader compiler wrapper missing'
if ! "$ROOT/scripts/verify_v14_frozen_v13.sh"; then fail=1; fi
if [ -n "${PSL1GHT:-}" ] && [ -d "${PSL1GHT}/ppu/include" ]; then
  if ! "$ROOT/scripts/audit_psl1ght_api_v13.sh"; then fail=1; fi
else
  miss 'cannot audit PSL1GHT headers until PSL1GHT points to the installed SDK'
fi
if [ "$fail" -eq 0 ]; then echo; echo 'READY: V13 toolchain + API + V14 preflight passed.'; exit 0; fi
echo; echo 'BLOCKED: fix the [MISS]/[FAIL] items before building PS3 artifacts.'; exit 2
