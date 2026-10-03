#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
mkdir -p "$root/build-host"
python3 "$root/scripts/verify-jfx-fix29.py"
g++ -std=c++17 -O2 -Wall -Wextra -Werror -DPS3_SP_LOADER_FIX28=1 -DPS3_SP_LOADER_FIX29=1 -I"$root/include" \
 "$root/tests/test_jfx_fix29.cpp" "$root/src/jfx_case_fix29.cpp" "$root/src/rsx_renderer_v10.cpp" \
 "$root/src/library_pair_fix28.cpp" "$root/src/library_hud_fix28.cpp" "$root/src/case_render_fix25.cpp" "$root/src/full_cover_case_fix26.cpp" \
 "$root/src/preferences_fix29.cpp" "$root/src/library_browser_fix28.cpp" "$root/src/app_controller.cpp" "$root/src/inspect_case_fix25.cpp" \
 "$root/src/safe_boot.cpp" "$root/src/coverflow_state.cpp" "$root/src/render_plan.cpp" "$root/src/v14_case_mesh.cpp" "$root/src/math3d.cpp" \
 "$root/src/rsx_stage1.cpp" "$root/src/cover_orientation_fix28.cpp" "$root/src/image_decode.cpp" "$root/src/cover_image.cpp" \
 "$root/src/cover_cache.cpp" "$root/src/boot_visual.cpp" "$root/src/runtime_diag.cpp" \
 -lpng -ljpeg -o "$root/build-host/test_jfx_fix29"
"$root/build-host/test_jfx_fix29"
g++ -std=c++17 -O2 -Wall -Wextra -Werror -I"$root/include" "$root/tests/test_webman_fix29.cpp" \
 "$root/src/webman_mount_fix29.cpp" "$root/src/mount_operation_fix29.cpp" -pthread -o "$root/build-host/test_webman_fix29"
"$root/build-host/test_webman_fix29"
