#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
mkdir -p "$root/build-host"
flags=(-std=c++17 -O2 -Wall -Wextra -Werror -DPS3_GAME_ORBIT_FIX42=1)
binary="$root/build-host/test_manager_fix42"
if [[ "${FIX42_SANITIZE:-0}" = 1 ]];then
 flags+=(-g -fsanitize=address,undefined -fno-omit-frame-pointer)
 binary="$root/build-host/test_manager_fix42_asan"
fi
g++ "${flags[@]}" -I"$root/include" "$root/tests/test_manager_fix42.cpp" "$root/src/file_manager_fix42.cpp" "$root/src/runtime_diag.cpp" -Wl,--wrap=rename,--wrap=statvfs -o "$binary"
"$binary"
