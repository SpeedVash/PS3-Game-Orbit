#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
mkdir -p "$root/build-host"
sources=(library_browser_fix28 app_controller inspect_case_fix25 safe_boot coverflow_state render_plan
 orbit_flow_fix31 library_pair_fix28 layout_settings_fix31 preferences_fix29 cover_limits_fix31
 cover_cache cover_image cover_orientation_fix28 image_decode rsx_stage1 rsx_renderer_v10
 library_hud_fix28 orbit_ui_fix30 boot_visual runtime_diag math3d v14_case_mesh jfx_case_fix29
 case_render_fix25 full_cover_case_fix26)
files=();for source in "${sources[@]}";do files+=("$root/src/$source.cpp");done
extra=()
if [[ "${FIX31_FLOW_ONLY:-0}" = 1 ]];then
 extra+=(-DPS3_GAME_ORBIT_FLOW_ONLY=1)
 files=()
 for source in "${sources[@]}";do
  case "$source" in rsx_renderer_v10|jfx_case_fix29|case_render_fix25) continue;;esac
  files+=("$root/src/$source.cpp")
 done
fi
g++ -std=c++17 -O2 -Wall -Wextra -Werror -DPS3_SP_LOADER_FIX28=1 -DPS3_SP_LOADER_FIX29=1 \
 -DPS3_GAME_ORBIT_FIX30=1 -DPS3_GAME_ORBIT_FIX31=1 -I"$root/include" \
 "${extra[@]}" "$root/tests/test_orbit_fix31.cpp" "${files[@]}" -lpng -ljpeg -lz -o "$root/build-host/test_orbit_fix31"
"$root/build-host/test_orbit_fix31" "$root"
