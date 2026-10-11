#!/usr/bin/env bash
set -euo pipefail
if [ "$#" -ne 3 ]; then
  echo "usage: $0 <vertex|fragment> <input.vcg|input.fcg> <output.vpo|output.fpo>" >&2
  exit 2
fi
stage="$1"; src="$2"; out="$3"
mode="${SHADER_PIPELINE:-auto}"
case "$stage" in vertex) profile=vp40; flag=-v ;; fragment) profile=fp40; flag=-f ;; *) echo "invalid shader stage: $stage" >&2; exit 2;; esac

command -v cgcomp >/dev/null 2>&1 || { echo "ERROR: cgcomp not found" >&2; exit 3; }

# Recent PSL1GHT projects sometimes use an explicit Cg->ARB step before cgcomp.
# Prefer it only when cgc is installed; otherwise use the official direct PSL1GHT path.
if [ "$mode" = "asm" ] || { [ "$mode" = "auto" ] && command -v cgc >/dev/null 2>&1; }; then
  command -v cgc >/dev/null 2>&1 || { echo "ERROR: SHADER_PIPELINE=asm requires cgc" >&2; exit 3; }
  tmp="${out}.asm"
  trap 'rm -f "$tmp"' EXIT
  echo "shader: cgc $profile -> cgcomp $stage"
  cgc -profile "$profile" "$src" -o "$tmp"
  cgcomp "$flag" -a "$tmp" "$out"
else
  echo "shader: direct cgcomp $stage"
  cgcomp "$flag" -Wcg,-strict "$src" "$out"
fi

test -s "$out" || { echo "ERROR: shader output is empty: $out" >&2; exit 4; }
