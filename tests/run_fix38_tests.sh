#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
mkdir -p "$root/build-host"
cd "$root"
python3 - <<'PY'
from PIL import Image
Image.new('RGBA',(1024,1024),(180,80,30,255)).save('tests/assets/cache32_1024.png')
Image.new('RGBA',(147,275),(180,80,30,255)).save('tests/assets/cache36_vertical.png')
PY
sources=(library_browser_fix28 app_controller inspect_case_fix25 safe_boot coverflow_state render_plan
 orbit_flow_fix31 library_pair_fix28 layout_settings_fix31 preferences_fix29 cover_limits_fix31
 cover_cache cover_image cover_orientation_fix28 image_decode rsx_stage1 rsx_renderer_v10
 library_hud_fix28 orbit_ui_fix30 boot_visual runtime_diag math3d v14_case_mesh jfx_case_fix29
 case_render_fix25 full_cover_case_fix26 case_animation_fix32 disc_texture_fix32 gpu_cover_cache_fix32 frame_validation_fix32 inspection_art_fix33 inspection_art_cache_fix34 cover_resolver performance_fix35 game_tools_fix35 rename_keyboard_fix35 prefetch_policy_fix36 background_settings_fix36 orbit_background_fix36 background_renderer_fix36 texture_pool_fix36 usb_artwork_fix36 orbit_settings_fix37 artwork_resize_fix37 cache_policy_fix37 case_model_fix37 file_store_fix38 disc_artwork_fix38)
files=();for source in "${sources[@]}";do files+=("$root/src/$source.cpp");done
extra=()
if [[ "${FIX38_PUBLIC_ONLY:-0}" = 1 ]];then
 extra+=(-DPS3_GAME_ORBIT_FLOW_ONLY=1)
 files=();for source in "${sources[@]}";do
  [[ "$source" = jfx_case_fix29 ]] && continue
  files+=("$root/src/$source.cpp")
 done
fi
sanitizer=();binary="$root/build-host/test_orbit_fix38"
if [[ "${FIX38_SANITIZE:-0}" = 1 ]];then
 sanitizer=(-g -fsanitize=address,undefined -fno-omit-frame-pointer)
 binary="$root/build-host/test_orbit_fix38_asan"
fi
g++ -std=c++17 -O2 "${sanitizer[@]}" -Wall -Wextra -Werror -DPS3_SP_LOADER_FIX28=1 -DPS3_SP_LOADER_FIX29=1 \
 -DPS3_GAME_ORBIT_FIX30=1 -DPS3_GAME_ORBIT_FIX31=1 -DPS3_GAME_ORBIT_FIX32=1 -DPS3_GAME_ORBIT_FIX33=1 -DPS3_GAME_ORBIT_FIX34=1 -DPS3_GAME_ORBIT_FIX35=1 -DPS3_GAME_ORBIT_FIX36=1 -DPS3_GAME_ORBIT_FIX37=1 -DPS3_GAME_ORBIT_FIX38=1 -I"$root/include" \
 "${extra[@]}" "$root/tests/test_orbit_fix38.cpp" "${files[@]}" -Wl,--wrap=rename -lpng -ljpeg -lz -o "$binary"
"$binary" "$root"
