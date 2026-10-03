#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CPP="$ROOT/src/v14_case_mesh.cpp"
HDR="$ROOT/docs/historical/v14_case_mesh_FIX28.h"
EXPECT_CPP="4d36d20082196439b90aae771dfa146af36154e7e06abebbac6b92935d721c39"
EXPECT_HDR="2159274c550f7240d854f64da0a94fc9b81b29a98cb7e9b9363e44f883a1cfa6"
actual_cpp="$(sha256sum "$CPP" | awk '{print $1}')"
actual_hdr="$(sha256sum "$HDR" | awk '{print $1}')"
status=0
printf 'V14 historical geometry check (JFX is the active model)\n'
if [ "$actual_cpp" = "$EXPECT_CPP" ]; then echo "[OK]   v14_case_mesh.cpp $actual_cpp"; else echo "[FAIL] v14_case_mesh.cpp $actual_cpp"; status=1; fi
if [ "$actual_hdr" = "$EXPECT_HDR" ]; then echo "[OK]   v14_case_mesh.h   $actual_hdr"; else echo "[FAIL] v14_case_mesh.h   $actual_hdr"; status=1; fi
if [ "$status" -ne 0 ]; then
  echo 'ERROR: historical V14 geometry changed. Restore the frozen files before building.' >&2
  exit 7
fi
