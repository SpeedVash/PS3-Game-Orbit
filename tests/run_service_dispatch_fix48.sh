#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
gcc -std=c99 -O2 -Wall -Wextra -Werror -I"$root/include" \
  "$root/tests/test_service_dispatch_fix48.c" "$root/plugin/fan_policy.c" \
  "$root/plugin/raw_reader.c" -o "$work/test"
"$work/test"
