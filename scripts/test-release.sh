#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
cd "$root"
chmod +x scripts/*.sh tests/*.sh
python3 scripts/prepare-jfx.py
bash tests/run_host_tests.sh
bash tests/run_fix29_tests.sh
bash tests/run_fix30_tests.sh
bash tests/run_fix35_tests.sh
bash tests/run_fix36_tests.sh
bash tests/run_fix37_tests.sh
bash tests/run_fix38_tests.sh
bash tests/run_fix39_tests.sh
python3 scripts/check-presentation-fix20.py
python3 scripts/check-renderer-fix28.py
python3 scripts/check-library-loop-fix35.py
python3 scripts/check-library-loop-fix36.py
python3 scripts/check-library-loop-fix37.py
python3 scripts/check-library-loop-fix38.py
python3 scripts/check-library-loop-fix39.py
python3 scripts/check-background-fix36.py
python3 scripts/check-renderer-jfx-fix35.py
python3 scripts/check-renderer-jfx-fix37.py
python3 scripts/check-renderer-jfx-fix38.py
python3 scripts/check-renderer-jfx-fix39.py
python3 scripts/check-command-stream-fix25.py

python3 scripts/check-services-fix35.py

FIX40_PUBLIC_ONLY=1 bash tests/run_fix40_tests.sh
bash tests/run_features_fix40.sh
python3 scripts/check-library-loop-fix40.py

FIX41_PUBLIC_ONLY=1 bash tests/run_fix41_tests.sh
bash tests/run_features_fix41.sh
python3 scripts/check-file-manager-native-fix41.py
python3 scripts/check-library-loop-fix41.py

FIX42_PUBLIC_ONLY=1 bash tests/run_fix42_tests.sh
bash tests/run_manager_fix42.sh
python3 scripts/check-file-manager-native-fix42.py
python3 scripts/check-library-loop-fix42.py

FIX43_PUBLIC_ONLY=1 bash tests/run_fix43_tests.sh
python3 scripts/check-manager-hud-fix43.py
