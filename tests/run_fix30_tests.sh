#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
mkdir -p "$root/build-host/orbit-visual"
g++ -std=c++17 -O2 -Wall -Wextra -Werror -DPS3_SP_LOADER_FIX28=1 -DPS3_SP_LOADER_FIX29=1 -DPS3_GAME_ORBIT_FIX30=1 -I"$root/include" \
 "$root/tests/test_orbit_fix30.cpp" "$root/src/orbit_ui_fix30.cpp" "$root/src/library_hud_fix28.cpp" \
 "$root/src/library_browser_fix28.cpp" "$root/src/app_controller.cpp" "$root/src/inspect_case_fix25.cpp" \
 "$root/src/safe_boot.cpp" "$root/src/coverflow_state.cpp" "$root/src/render_plan.cpp" \
 "$root/src/rsx_stage1.cpp" "$root/src/cover_orientation_fix28.cpp" "$root/src/cover_image.cpp" "$root/src/image_decode.cpp" \
 -lpng -ljpeg -lz -o "$root/build-host/test_orbit_fix30"
"$root/build-host/test_orbit_fix30" "$root" "$root/build-host/orbit-visual"
