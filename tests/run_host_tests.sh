#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
mkdir -p "$ROOT/build-host"

g++ -std=c++17 -O2 -Wall -Wextra -I"$ROOT/include" \
  "$ROOT/tests/test_runtime.cpp" \
  "$ROOT/src/coverflow_state.cpp" "$ROOT/src/app_controller.cpp" \
  "$ROOT/src/render_plan.cpp" "$ROOT/src/metadata_utils.cpp" "$ROOT/src/cover_image.cpp" \
  -o "$ROOT/build-host/test_runtime"
"$ROOT/build-host/test_runtime"

g++ -std=c++17 -O2 -Wall -Wextra -I"$ROOT/include" \
  "$ROOT/tests/test_v04.cpp" "$ROOT/src/cover_image.cpp" "$ROOT/src/image_decode.cpp" \
  "$ROOT/src/rsx_stage1.cpp" "$ROOT/src/cover_orientation_fix28.cpp" "$ROOT/src/v14_case_mesh.cpp" \
  -lpng -ljpeg -o "$ROOT/build-host/test_v04"
"$ROOT/build-host/test_v04" "$ROOT"

g++ -std=c++17 -O2 -Wall -Wextra -I"$ROOT/include" \
  "$ROOT/tests/test_v13.cpp" "$ROOT/src/case_render_fix25.cpp" "$ROOT/src/coverflow_state.cpp" "$ROOT/src/render_plan.cpp" \
  "$ROOT/src/v14_case_mesh.cpp" "$ROOT/src/math3d.cpp" "$ROOT/src/rsx_renderer_v10.cpp" "$ROOT/src/jfx_case_fix29.cpp" "$ROOT/src/library_pair_fix28.cpp" "$ROOT/src/library_hud_fix28.cpp" \
  "$ROOT/src/rsx_stage1.cpp" "$ROOT/src/cover_orientation_fix28.cpp" "$ROOT/src/image_decode.cpp" "$ROOT/src/cover_image.cpp" \
  "$ROOT/src/cover_cache.cpp" "$ROOT/src/boot_visual.cpp" "$ROOT/src/runtime_diag.cpp" \
  -lpng -ljpeg -o "$ROOT/build-host/test_v13"
"$ROOT/build-host/test_v13"

g++ -std=c++17 -O2 -Wall -Wextra -I"$ROOT/include" \
  "$ROOT/tests/test_v13_platform.cpp" "$ROOT/src/ps3_lifecycle.cpp" "$ROOT/src/runtime_diag.cpp" \
  -pthread -o "$ROOT/build-host/test_v13_platform"
"$ROOT/build-host/test_v13_platform"


g++ -std=c++17 -O2 -Wall -Wextra -I"$ROOT/include" \
  "$ROOT/tests/test_v13_safe_boot.cpp" "$ROOT/src/safe_boot.cpp" "$ROOT/src/coverflow_state.cpp" "$ROOT/src/render_plan.cpp" \
  -o "$ROOT/build-host/test_v13_safe_boot"
"$ROOT/build-host/test_v13_safe_boot"

g++ -std=c++17 -O2 -Wall -Wextra -Werror -DPS3_SP_LOADER_FIX28=1 -I"$ROOT/include" \
  "$ROOT/tests/test_fix28_case.cpp" "$ROOT/src/case_render_fix25.cpp" "$ROOT/src/inspect_case_fix25.cpp" "$ROOT/src/safe_boot.cpp" "$ROOT/src/coverflow_state.cpp" "$ROOT/src/render_plan.cpp" \
  "$ROOT/src/v14_case_mesh.cpp" "$ROOT/src/full_cover_case_fix26.cpp" "$ROOT/src/math3d.cpp" "$ROOT/src/rsx_renderer_v10.cpp" "$ROOT/src/jfx_case_fix29.cpp" "$ROOT/src/library_pair_fix28.cpp" "$ROOT/src/library_hud_fix28.cpp" \
  "$ROOT/src/rsx_stage1.cpp" "$ROOT/src/cover_orientation_fix28.cpp" "$ROOT/src/image_decode.cpp" "$ROOT/src/cover_image.cpp" \
  "$ROOT/src/cover_cache.cpp" "$ROOT/src/boot_visual.cpp" "$ROOT/src/runtime_diag.cpp" \
  -lpng -ljpeg -o "$ROOT/build-host/test_fix28_case"
"$ROOT/build-host/test_fix28_case" "$ROOT"

g++ -std=c++17 -O2 -Wall -Wextra -Werror -DPS3_SP_LOADER_FIX28=1 -I"$ROOT/include" \
  "$ROOT/tests/test_library_fix28.cpp" "$ROOT/src/library_scanner.cpp" "$ROOT/src/metadata_utils.cpp" "$ROOT/src/cover_resolver.cpp" \
  "$ROOT/src/library_browser_fix28.cpp" "$ROOT/src/library_hud_fix28.cpp" "$ROOT/src/app_controller.cpp" \
  "$ROOT/src/inspect_case_fix25.cpp" "$ROOT/src/safe_boot.cpp" "$ROOT/src/coverflow_state.cpp" "$ROOT/src/render_plan.cpp" \
  "$ROOT/src/v14_case_mesh.cpp" "$ROOT/src/full_cover_case_fix26.cpp" "$ROOT/src/case_render_fix25.cpp" "$ROOT/src/math3d.cpp" \
  "$ROOT/src/rsx_renderer_v10.cpp" "$ROOT/src/jfx_case_fix29.cpp" "$ROOT/src/library_pair_fix28.cpp" "$ROOT/src/rsx_stage1.cpp" "$ROOT/src/cover_orientation_fix28.cpp" "$ROOT/src/image_decode.cpp" "$ROOT/src/cover_image.cpp" \
  "$ROOT/src/cover_cache.cpp" "$ROOT/src/boot_visual.cpp" "$ROOT/src/runtime_diag.cpp" \
  -lpng -ljpeg -o "$ROOT/build-host/test_library_fix28"
"$ROOT/build-host/test_library_fix28" "$ROOT"

g++ -std=c++17 -O2 -Wall -Wextra -Werror -DPS3_SP_LOADER_FIX28=1 -I"$ROOT/include" \
  "$ROOT/tests/test_pair_orientation_fix28.cpp" "$ROOT/src/cover_orientation_fix28.cpp" "$ROOT/src/library_pair_fix28.cpp" \
  "$ROOT/src/library_browser_fix28.cpp" "$ROOT/src/library_hud_fix28.cpp" "$ROOT/src/app_controller.cpp" \
  "$ROOT/src/inspect_case_fix25.cpp" "$ROOT/src/safe_boot.cpp" "$ROOT/src/coverflow_state.cpp" "$ROOT/src/render_plan.cpp" \
  "$ROOT/src/v14_case_mesh.cpp" "$ROOT/src/full_cover_case_fix26.cpp" "$ROOT/src/case_render_fix25.cpp" "$ROOT/src/math3d.cpp" \
  "$ROOT/src/rsx_renderer_v10.cpp" "$ROOT/src/jfx_case_fix29.cpp" "$ROOT/src/rsx_stage1.cpp" "$ROOT/src/image_decode.cpp" "$ROOT/src/cover_image.cpp" \
  "$ROOT/src/cover_cache.cpp" "$ROOT/src/boot_visual.cpp" "$ROOT/src/runtime_diag.cpp" \
  -lpng -ljpeg -o "$ROOT/build-host/test_pair_orientation_fix28"
"$ROOT/build-host/test_pair_orientation_fix28" "$ROOT"

"$ROOT/scripts/verify_v14_frozen_v13.sh"
