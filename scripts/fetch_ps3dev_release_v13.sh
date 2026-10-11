#!/usr/bin/env bash
set -euo pipefail
# Optional local helper. GitHub Actions is the recommended V13 build path.
ARCHIVE="${PS3DEV_RELEASE_ASSET:-ps3dev-linux-X64.tar.gz}"
DEST="${1:-$HOME/ps3dev-v13}"
TAG="${PS3DEV_RELEASE_TAG:-}"

if [[ -n "$TAG" ]]; then
  URL="https://github.com/ps3dev/ps3dev/releases/download/${TAG}/${ARCHIVE}"
else
  command -v curl >/dev/null 2>&1 || { echo 'ERROR: curl is required' >&2; exit 2; }
  command -v python3 >/dev/null 2>&1 || { echo 'ERROR: python3 is required for automatic release discovery' >&2; exit 2; }
  API_JSON="$(curl -fsSL 'https://api.github.com/repos/ps3dev/ps3dev/releases?per_page=30')"
  read -r TAG URL < <(printf '%s' "$API_JSON" | python3 -c '
import json,sys
releases=json.load(sys.stdin)
for r in releases:
    for a in r.get("assets",[]):
        if a.get("name")=="ps3dev-linux-X64.tar.gz":
            print(r.get("tag_name",""), a.get("browser_download_url",""))
            raise SystemExit
raise SystemExit(2)
') || { echo 'ERROR: no recent PS3DEV release contains ps3dev-linux-X64.tar.gz' >&2; exit 3; }
fi

mkdir -p "$DEST"
echo "PS3DEV release: $TAG"
echo "Downloading $URL"
curl -fL --retry 3 --retry-delay 2 "$URL" -o "$DEST/$ARCHIVE"
rm -rf "$DEST/extracted"
mkdir -p "$DEST/extracted"
tar -xzf "$DEST/$ARCHIVE" -C "$DEST/extracted"
TOOL="$(find "$DEST/extracted" -type f -path '*/ppu/bin/ppu-g++' -print -quit)"
[[ -n "$TOOL" ]] || { echo 'ERROR: ppu-g++ not found in archive' >&2; exit 4; }
SDK="$(dirname "$(dirname "$(dirname "$TOOL")")")"
cat <<EOM

Set for this terminal:
  export PS3DEV="$SDK"
  export PSL1GHT="\$PS3DEV"
  export PATH="\$PATH:\$PS3DEV/bin:\$PS3DEV/ppu/bin:\$PS3DEV/spu/bin"

Then run:
  ./scripts/preflight_v13.sh
  ./scripts/build_release_v13.sh
EOM
