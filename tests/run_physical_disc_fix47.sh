#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
cd "$root"
mkdir -p build-host/disc47
python3 - <<'PY'
from pathlib import Path
import struct
from PIL import Image
root=Path('build-host/disc47')
def sfo(values):
    keys=bytearray();data=bytearray();records=[]
    for key,value in values.items():
        ko=len(keys);keys.extend(key.encode()+b'\0');vo=len(data);raw=value.encode()+b'\0';data.extend(raw)
        records.append(struct.pack('<HHIII',ko,0x204,len(raw),len(raw),vo))
    start=20+16*len(records)
    return struct.pack('<4sIIII',b'\x00PSF',0x101,start,start+len(keys),len(records))+b''.join(records)+keys+data
for folder,id,title,category in [('bd','BLES00001','Jogo original A','DG'),('bd_b','BLUS00002','Jogo original B','DG'),
        ('bad_id','not-an-id','Inválido','DG'),('update','BLES00001','Atualização','HG'),
        ('front','BCES00003','Somente ícone','DG'),('neutral','BLJS00004','Sem artes','DG')]:
    p=root/folder/'PS3_GAME';p.mkdir(parents=True,exist_ok=True)
    (p/'PARAM.SFO').write_bytes(sfo({'TITLE_ID':id,'TITLE':title,'CATEGORY':category}))
(root/'bad_sfo/PS3_GAME').mkdir(parents=True,exist_ok=True);(root/'bad_sfo/PS3_GAME/PARAM.SFO').write_bytes(b'not a PARAM.SFO')
(root/'ps1').mkdir(parents=True,exist_ok=True)
Image.new('RGB',(320,176),(80,170,200)).save(root/'front/PS3_GAME/ICON0.PNG')
covers=root/'covers';covers.mkdir(exist_ok=True)
Image.new('RGB',(1000,550),(100,80,180)).save(covers/'BLES00001.jpg')
Image.new('RGBA',(1000,550),(80,180,100,255)).save(covers/'BLES00001_INSIDE.png')
Image.new('RGBA',(512,512),(180,80,100,255)).save(covers/'BLES00001_DISC.png')
PY
flags=(-DPS3_SP_LOADER_FIX28=1 -DPS3_SP_LOADER_FIX29=1)
for number in {30..47};do flags+=("-DPS3_GAME_ORBIT_FIX$number=1");done
sources=(physical_disc_fix47 physical_disc_monitor_fix47 webman_mount_fix29 mount_operation_fix29 performance_fix35 runtime_diag metadata_utils cover_resolver preferences_fix29
    game_tools_fix35 file_store_fix38 game_order_fix40 library_browser_fix28 app_controller inspect_case_fix25
    safe_boot coverflow_state cover_cache cover_image v14_case_mesh full_cover_case_fix26 render_plan
    library_pair_fix28 cover_orientation_fix28 orbit_flow_fix31 case_style_fix41)
files=();for name in "${sources[@]}";do files+=("src/$name.cpp");done
g++ -std=c++17 -O2 -pthread -Wall -Wextra -Werror "${flags[@]}" -Iinclude tests/test_physical_disc_fix47.cpp "${files[@]}" -o build-host/test_physical_disc_fix47
timeout 15 build-host/test_physical_disc_fix47 "$root/build-host/disc47"
