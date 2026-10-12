#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
cd "$root"
mkdir -p build-host
python3 - <<'PY'
from PIL import Image
Image.new('RGBA',(147,275),(180,80,30,255)).save('tests/assets/cache36_vertical.png')
image=Image.new('RGBA',(127,131));image.putdata([(x%256,y%256,(x+y)%256,(x*3+y*5)%256) for y in range(131) for x in range(127)]);image.save('build-host/disc45.png')
Image.new('RGBA',(1600,1200),(180,80,30,170)).save('build-host/large45.png')
image=Image.new('RGB',(147,275),(130,70,20));exif=Image.Exif();exif[274]=6;image.save('build-host/photo45.jpg',exif=exif)
PY
sources=(library_browser_fix28 app_controller inspect_case_fix25 safe_boot coverflow_state render_plan
 orbit_flow_fix31 library_pair_fix28 layout_settings_fix31 preferences_fix29 cover_limits_fix31
 cover_cache cover_image cover_orientation_fix28 image_decode rsx_stage1 rsx_renderer_v10
 library_hud_fix28 orbit_ui_fix30 boot_visual runtime_diag math3d v14_case_mesh jfx_case_fix29
 case_render_fix25 full_cover_case_fix26 case_animation_fix32 disc_texture_fix32 gpu_cover_cache_fix32
 frame_validation_fix32 inspection_art_fix33 inspection_art_cache_fix34 cover_resolver performance_fix35
 game_tools_fix35 rename_keyboard_fix35 prefetch_policy_fix36 background_settings_fix36 orbit_background_fix36
 background_renderer_fix36 texture_pool_fix36 usb_artwork_fix36 orbit_settings_fix37 artwork_resize_fix37
 cache_policy_fix37 cache_policy_fix44 case_model_fix37 file_store_fix38 disc_artwork_fix38 game_order_fix40
 file_manager_fix40 file_manager_fix41 file_manager_fix42 case_style_fix41 prepared_artwork_fix45 artwork_loader_fix45 artwork_upload_fix45 artwork_stream_fix45)
files=();extra=();flags=(-DPS3_SP_LOADER_FIX28=1 -DPS3_SP_LOADER_FIX29=1)
for number in {30..45};do flags+=("-DPS3_GAME_ORBIT_FIX$number=1");done
if [[ "${FIX45_PUBLIC_ONLY:-0}" = 1 ]];then extra+=(-DPS3_GAME_ORBIT_FLOW_ONLY=1);fi
for source in "${sources[@]}";do
 [[ "${FIX45_PUBLIC_ONLY:-0}" = 1 && "$source" = jfx_case_fix29 ]] && continue
 files+=("$root/src/$source.cpp")
done
sanitizer=();binary="$root/build-host/test_artwork_fix45"
if [[ "${FIX45_SANITIZE:-0}" = 1 ]];then
 sanitizer=(-g -fsanitize=address,undefined -fno-omit-frame-pointer)
 binary+=_asan
fi
if [[ "${FIX45_THREAD_SANITIZE:-0}" = 1 ]];then
 sanitizer=(-g -fsanitize=thread -fno-omit-frame-pointer)
 binary+=_tsan
fi
g++ -std=c++17 -O2 -pthread "${sanitizer[@]}" -Wall -Wextra -Werror "${flags[@]}" -I"$root/include" "${extra[@]}" "$root/tests/test_artwork_fix45.cpp" "${files[@]}" -lpng -ljpeg -lz -o "$binary"
timeout 60 "$binary" "$root"
