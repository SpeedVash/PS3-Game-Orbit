#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
mkdir -p "$root/build-host"
flags=(-std=c++17 -O2 -Wall -Wextra -Werror -DPS3_SP_LOADER_FIX28=1 -DPS3_SP_LOADER_FIX29=1)
for i in $(seq 30 41);do flags+=("-DPS3_GAME_ORBIT_FIX$i=1");done
if [[ "${FIX41_SANITIZE:-0}" = 1 ]];then flags+=(-g -fsanitize=address,undefined -fno-omit-frame-pointer);fi
files=();for source in file_manager_fix41 runtime_diag preferences_fix29 file_store_fix38 library_browser_fix28 game_order_fix40 app_controller inspect_case_fix25 safe_boot coverflow_state orbit_flow_fix31 render_plan library_pair_fix28 cover_orientation_fix28 case_style_fix41;do files+=("$root/src/$source.cpp");done
g++ "${flags[@]}" -I"$root/include" "$root/tests/test_features_fix41.cpp" "${files[@]}" -Wl,--wrap=rename -o "$root/build-host/test_features_fix41"
"$root/build-host/test_features_fix41"
