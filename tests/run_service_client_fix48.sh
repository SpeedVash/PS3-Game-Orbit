#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
gzip -dc "$root/tests/fixtures/fix48/fixture.iso.gz" > "$work/fixture.iso"
g++ -std=c++17 -O2 -pthread -Wall -Wextra -Werror -DPS3_GAME_ORBIT_FIX48=1 -DPS3_GAME_ORBIT_FIX38=1 \
  -DORBIT48_RUNTIME_DIR="\"$work\"" -I"$root/include" "$root/tests/test_service_client_fix48.cpp" \
  "$root/src/orbit_service_client_fix48.cpp" "$root/src/iso_metadata_fix48.cpp" \
  "$root/src/ntfs_reader_fix48.cpp" "$root/src/ntfs_catalog_fix48.cpp" \
  "$root/src/raw_reader_fix48.cpp" "$root/src/metadata_utils.cpp" "$root/src/file_store_fix38.cpp" -o "$work/test"
"$work/test" "$work" "$work/fixture.iso"
