#!/usr/bin/env python3
"""Check artifact architecture, identity and frozen source; no hardware claims."""
import hashlib
from pathlib import Path
import re
import struct
import sys

folder = Path(sys.argv[1])
root = Path(__file__).resolve().parent.parent
expected = {
    'src/full_cover_case_fix26.cpp': 'f816ee39ba225ea099ddef0b967d143ecbafabd5a966f995cd012ecb6854e59b',
    'include/full_cover_case_fix26.h': '2736ce08659646e5002383419b300e39f622c92d8034aa2aa6ae8099edb619e4',
    'docs/historical/inspect_case_fix25_FIX30.cpp': '75736ceea6b8c6c8e512a22cccdee3c20e10af947c0e20f98569c3a5243198ac',
    'docs/historical/inspect_case_fix25_FIX30.h': 'd27f7e6efb89bbbf120b3525f8f4cedb9756ecce0d9a1f944518612a70aa0e07',
    'src/rsx_command_stream_fix25.cpp': '155555fd89fe888719768dc959dca8264025b19cab0cdd48a3e41059e7c05e6d',
    'include/rsx_command_stream_fix25.h': '39a0b02d16336243e413f31dd0db14ee8c557185f534ab773e749ccd49e3d454',
    'src/render_plan.cpp': 'eb1ffcd3f4dc4ff7db4ce7a383aba42e3d223434381909ad47df1ed4039888c2',
    'include/case_render_fix25.h': '8d2460f3a36086aa76406a71cba53def3bbb9aaeb319f5cf3bab2610ba9a26ae',
    'src/case_render_fix25.cpp': 'afa49ea06c76f103a9b6cd48e867dd84b083bd9040367eafbe0fac14b245e457',
    'pkgfiles/USRDIR/FULL_COVER_FIX26_CONTINUA.png': 'a350d4ab352a1aaae8c193821e8f6b313d27b19343e875bf144878b2ac0f6ac8',
    'src/v14_case_mesh.cpp': '4d36d20082196439b90aae771dfa146af36154e7e06abebbac6b92935d721c39',
    'docs/historical/v14_case_mesh_FIX28.h': '2159274c550f7240d854f64da0a94fc9b81b29a98cb7e9b9363e44f883a1cfa6',
    'shaders/v14_case.vpo': 'a5a141a629c5811793d4edb6373e1f6e4a8f738c5eff04d609db945595fb3434',
    'shaders/v14_case.fpo': 'c520d2b87206e89b04493367b10e14d675100571e441d90e502a6df01bcb07f3',
}
for path, digest in expected.items():
    assert hashlib.sha256((root/path).read_bytes()).hexdigest() == digest, path
print('PASS: historical V14, FIX26 wrap, archived shaders, archived 1.0 inspection/FIFO controllers, legacy pose and fixture preserved')
assert hashlib.sha256((root/'docs/historical/rsx_present_FIX29.cpp').read_bytes()).hexdigest() == 'b426f4c11fc0a2ef1712ef4de66c1415f88c95fa9578871c891f8ce87a34769b'
print('PASS: Historical presentation preserved; current bitmap fill uses the same bounded flip sequence')
elf = (folder/'PS3_GAME_ORBIT_FIX31.elf').read_bytes()
assert elf[:6] == b'\x7fELF\x02\x02', 'Expected ELF64 big endian'
assert struct.unpack_from('>H', elf, 18)[0] == 21, 'Expected PowerPC64'
for name in ('EBOOT.BIN', 'PS3_GAME_ORBIT_FIX31.self'):
    assert (folder/name).read_bytes()[:4] == b'SCE\0', name
print('PASS: PowerPC64 big endian ELF and both SELF headers')
sfo = (folder/'PARAM.SFO').read_bytes()
assert sfo[:4] == b'\0PSF'
key_start, data_start, count = struct.unpack_from('<III', sfo, 8)
values = {}
for i in range(count):
    keyoff, fmt, length, capacity, offset = struct.unpack_from('<HHIII', sfo, 20+i*16)
    assert length <= capacity
    name = sfo[key_start+keyoff:].split(b'\0', 1)[0].decode()
    raw = sfo[data_start+offset:data_start+offset+length]
    values[name] = raw.rstrip(b'\0').decode() if fmt == 0x204 else raw
assert values['TITLE_ID'] == 'PGORBT301', values
assert values['TITLE'] == 'PS3 Game Orbit', values
assert values['APP_VER'] == '01.01', values
assert values['CATEGORY'] == 'HG', values
pkg = (folder/'PS3_GAME_ORBIT_FIX31.gnpdrm.pkg').read_bytes()
assert pkg[:4] == b'\x7fPKG'
assert struct.unpack_from('>Q', pkg, 24)[0] == len(pkg), 'Package length mismatch'
assert pkg[48:96].rstrip(b'\0') == b'UP0001-PGORBT301_00-0000000000000000'
assert struct.unpack_from('>H', pkg, 4)[0] == 0x8000, 'Expected finalized retail PKG flag'
print('PASS: PARAM.SFO PGORBT301/01.01 and finalized PKG identity/length')
package_tree=root/'build/pkg'
for path,size in (('ICON0.PNG',(320,176)),('PIC1.PNG',(1920,1080)),('USRDIR/ORBIT_SPLASH.png',(1280,720))):
    data=(package_tree/path).read_bytes()
    assert data[:8]==bytes([137,80,78,71,13,10,26,10])
    assert struct.unpack_from('>II',data,16)==size,path
assert (package_tree/'USRDIR/NOTO_SANS_OFL.txt').read_bytes()==(root/'assets/fonts/OFL.txt').read_bytes()
assert (package_tree/'USRDIR/EBOOT.BIN').read_bytes()==(folder/'EBOOT.BIN').read_bytes()
assert (package_tree/'ICON0.PNG').read_bytes()==(root/'pkgfiles/ICON0.PNG').read_bytes()
print('PASS: staged icon, wave PIC1, branded startup, font license and final EBOOT match source/delivery')
symbols = (folder/'NATIVE_OBJECT_SYMBOLS.txt').read_text()
for forbidden in ('gcmSetWaitFlip', 'rsxFinish', 'rsxSetWaitLabel'):
    assert not re.search(r'\b' + forbidden + r'\b', symbols), forbidden
for required in ('jpgLoadFromBuffer', 'gcmSetFlip', 'gcmGetFlipStatus', 'rsxSetReferenceCommand', 'rsxSetWriteBackendLabel', 'sysGetSystemTime', 'rsxLoadTexture', 'poll', 'setsockopt', 'send', 'recv', 'pngLoadFromBuffer', 'rsxSetJumpCommand', 'rsxSetCullFace', 'rsxSetFrontFace'):
    assert re.search(r'\b' + required + r'\b', symbols), required
print('PASS: native object references use flip polling and REF/backend-label writes; no forbidden wait calls')
print('LIMIT: compilation and package checks do not prove TV visibility or prevent driver/hardware stalls')
