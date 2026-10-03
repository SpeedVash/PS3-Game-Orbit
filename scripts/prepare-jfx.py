#!/usr/bin/env python3
"""Prepare the separately acquired JFX model without publishing editable data."""
import argparse
import hashlib
from pathlib import Path
import subprocess
import sys
from zipfile import ZipFile, is_zipfile

ROOT = Path(__file__).resolve().parent.parent
OBJ = ROOT / 'assets/jfx_bluray/Bluray_Tris.obj'
INC = ROOT / 'src/jfx_case_data_fix29.inc'
OBJ_SHA = '4263705eb3e9801ee10d173094865fb50c5b52dfd52d2f1160390a09e3034c55'
INC_SHA = '9ae460c688c269393668cdd5072cfc5365594832410e72f70c18721501289956'
PAGE = 'https://www.turbosquid.com/3d-models/free-dvd-bluray-case-3d-model/724523'
MAX_BYTES = 4 * 1024 * 1024

def die(message):
    print(message, file=sys.stderr)
    raise SystemExit(2)

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--source', type=Path, help='Your JFX_DVDBlu.zip or original Bluray_Tris.obj')
args = parser.parse_args()
if args.source:
    source = args.source.expanduser().resolve()
    if not source.is_file():
        die('Source must be a regular ZIP or OBJ file.')
    if is_zipfile(source):
        with ZipFile(source) as archive:
            entries = [entry for entry in archive.infolist()
                       if not entry.is_dir() and entry.filename.replace('\\', '/').endswith('/BlurayCase/Models/Bluray_Tris.obj')]
            if len(entries) != 1 or entries[0].file_size > MAX_BYTES:
                die('ZIP must contain one original BlurayCase/Models/Bluray_Tris.obj of at most 4 MiB.')
            with archive.open(entries[0]) as stream:
                data = stream.read(MAX_BYTES + 1)
    else:
        if source.stat().st_size > MAX_BYTES:
            die('OBJ exceeds the 4 MiB limit.')
        data = source.read_bytes()
    if len(data) > MAX_BYTES or hashlib.sha256(data).hexdigest() != OBJ_SHA:
        die('Source hash differs from the approved original. No project file was changed.')
    OBJ.parent.mkdir(parents=True, exist_ok=True)
    OBJ.write_bytes(data)
if not OBJ.is_file():
    die('Get JFX_DVDBlu from ' + PAGE + '\nThen run: python3 scripts/prepare-jfx.py --source /path/JFX_DVDBlu.zip')
if hashlib.sha256(OBJ.read_bytes()).hexdigest() != OBJ_SHA:
    die('Local OBJ hash differs from the approved original.')
if not INC.is_file() or hashlib.sha256(INC.read_bytes()).hexdigest() != INC_SHA:
    subprocess.run([sys.executable, str(ROOT / 'scripts/import-jfx-fix29.py')], check=True, cwd=ROOT)
if hashlib.sha256(INC.read_bytes()).hexdigest() != INC_SHA:
    die('Generated model differs from the approved native mesh.')
subprocess.run([sys.executable, str(ROOT / 'scripts/verify-jfx-fix29.py')], check=True, cwd=ROOT)
print('PASS: approved model prepared locally; editable data must remain outside the public repository.')
