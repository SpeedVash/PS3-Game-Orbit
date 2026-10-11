#!/bin/bash
set -eu
root=$(cd "$(dirname "$0")/.." && pwd)
mkdir -p "$root/build-host"
flags=(-std=c++17 -O2 -Wall -Wextra -Werror -DPS3_SP_LOADER_FIX28=1 -DPS3_SP_LOADER_FIX29=1)
for i in $(seq 30 40); do flags+=("-DPS3_GAME_ORBIT_FIX$i=1"); done
if [[ "${FIX40_SANITIZE:-0}" = 1 ]]; then flags+=(-g -fsanitize=address,undefined -fno-omit-frame-pointer); fi
files=(); for source in game_order_fix40 file_manager_fix40 file_store_fix38 library_browser_fix28 app_controller inspect_case_fix25 safe_boot coverflow_state orbit_flow_fix31 render_plan library_pair_fix28 cover_orientation_fix28; do files+=("$root/src/$source.cpp"); done
g++ "${flags[@]}" -I"$root/include" "$root/tests/test_features_fix40.cpp" "${files[@]}" -Wl,--wrap=rename -o "$root/build-host/test_features_fix40"
"$root/build-host/test_features_fix40"
