#!/usr/bin/env python3
"""Own small ISO9660 virtual-disc template; it contains no game files."""
import argparse
from pathlib import Path
import struct
def record(lba,name):
    size=(33+len(name)+1)&~1
    b=bytearray(size);b[0]=size
    b[2:10]=struct.pack('<I',lba)+struct.pack('>I',lba)
    b[10:18]=struct.pack('<I',2048)+struct.pack('>I',2048)
    b[18:25]=bytes([126,10,13,0,0,0,0]);b[25]=2;b[28:32]=b'\1\0\0\1'
    b[32]=len(name);b[33:33+len(name)]=name
    return b
def template():
    b=bytearray(131072)
    b[3]=2;struct.pack_into('>I',b,0x14,63)
    b[0x800:0x80c]=b'PlayStation3';b[0x810:0x81a]=b'ORBT-00000';b[0x81a:0x830]=b' '*22
    at=16*2048;b[at]=1;b[at+1:at+6]=b'CD001';b[at+6]=1
    b[at+8:at+72]=b' '*64;b[at+40:at+49]=b'PS3VOLUME'
    b[at+80:at+88]=struct.pack('<I',64)+struct.pack('>I',64)
    b[at+120:at+124]=b'\1\0\0\1';b[at+124:at+128]=b'\1\0\0\1'
    b[at+128:at+132]=struct.pack('<H',2048)+struct.pack('>H',2048)
    b[at+132:at+140]=struct.pack('<I',10)+struct.pack('>I',10)
    struct.pack_into('<I',b,at+140,18);struct.pack_into('>I',b,at+148,19)
    root=record(20,b'\0');b[at+156:at+156+len(root)]=root
    b[at+190:at+318]=b' '*128;b[at+881]=1
    end=17*2048;b[end]=255;b[end+1:end+6]=b'CD001';b[end+6]=1
    b[18*2048:18*2048+10]=b'\1\0'+struct.pack('<I',20)+struct.pack('<H',1)+b'\0\0'
    b[19*2048:19*2048+10]=b'\1\0'+struct.pack('>I',20)+struct.pack('>H',1)+b'\0\0'
    directory=record(20,b'\0')+record(20,b'\1')
    b[20*2048:20*2048+len(directory)]=directory
    return bytes(b)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('output');a=p.parse_args()
    Path(a.output).parent.mkdir(parents=True,exist_ok=True);Path(a.output).write_bytes(template())
