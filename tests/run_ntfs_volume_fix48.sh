#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
gzip -dc "$root/tests/fixtures/fix48/fixture.ntfs.gz" > "$work/volume.ntfs"
gzip -dc "$root/tests/fixtures/fix48/fixture.iso.gz" > "$work/game.iso"
opts='-O2 -Wall -Wextra -Werror'
if [[ "${FIX48_SANITIZE:-0}" = 1 ]];then opts+=' -g -fsanitize=address,undefined -fno-omit-frame-pointer';fi
g++ -std=c++17 $opts -DPS3_GAME_ORBIT_FIX48=1 -I"$root/include" \
  "$root/tests/test_ntfs_volume_fix48.cpp" "$root/src/ntfs_reader_fix48.cpp" \
  "$root/src/iso_metadata_fix48.cpp" "$root/src/ntfs_catalog_fix48.cpp" \
  "$root/src/raw_reader_fix48.cpp" "$root/src/metadata_utils.cpp" -o "$work/test"
"$work/test" "$work/volume.ntfs" "$work/game.iso"
