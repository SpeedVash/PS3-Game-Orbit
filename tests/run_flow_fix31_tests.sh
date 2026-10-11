#!/usr/bin/env bash
set -euo pipefail
export FIX31_FLOW_ONLY=1
exec bash "$(dirname "$0")/run_fix31_tests.sh"
