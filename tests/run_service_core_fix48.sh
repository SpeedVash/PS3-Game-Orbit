#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
opts='-O2 -Wall -Wextra -Werror'
if [[ "${FIX48_SANITIZE:-0}" = 1 ]];then opts+=' -g -fsanitize=address,undefined -fno-omit-frame-pointer';fi
gcc -std=c99 $opts -I"$root/include" -c "$root/plugin/fan_policy.c" -o "$work/fan.o"
gcc -std=c99 $opts -I"$root/include" -c "$root/plugin/raw_reader.c" -o "$work/raw.o"
g++ -std=c++17 $opts -DPS3_GAME_ORBIT_FIX48=1 -I"$root/include" \
  "$root/tests/test_service_core_fix48.cpp" "$root/src/ntfs_reader_fix48.cpp" \
  "$root/src/iso_metadata_fix48.cpp" "$work/fan.o" "$work/raw.o" -o "$work/tests"
"$work/tests"
