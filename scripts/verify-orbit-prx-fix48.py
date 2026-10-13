#!/usr/bin/env python3
"""Simulate two independently loaded PRX segments and validate its entry descriptors."""
from pathlib import Path
import struct
import sys

def verify(path):
    data=Path(path).read_bytes()
    assert data[:8]==b'\x7fELF\x02\x02\x01\x66'
    assert struct.unpack_from('>HH',data,16)==(0xffa4,21)
    headers=[struct.unpack_from('>IIQQQQQQ',data,64+i*56) for i in range(3)]
    assert [p[0] for p in headers]==[1,1,0x700000a4]
    assert headers[0][1]&7==5 and headers[1][1]&7==6
    memory=[]
    for p in headers[:2]:
        assert p[5]<=p[6] and p[2]+p[5]<=len(data)
        b=bytearray(p[6]);b[:p[5]]=data[p[2]:p[2]+p[5]];memory.append(b)
    reloc=headers[2];assert reloc[5]%24==0 and reloc[2]+reloc[5]==len(data)
    bases=(0x18000000,0x19000000)  # nonadjacent load addresses deliberately
    for at in range(reloc[2],reloc[2]+reloc[5],24):
        off,unused,value,dest,kind,relative=struct.unpack_from('>QHBBIQ',data,at)
        assert unused==0 and value<2 and dest<2 and relative<memory[value].__len__()
        address=bases[value]+relative
        size={1:4,4:2,5:2,6:2,38:8}[kind];assert off+size<=len(memory[dest])
        if kind==1:struct.pack_into('>I',memory[dest],off,address)
        elif kind==38:struct.pack_into('>Q',memory[dest],off,address)
        elif kind==4:struct.pack_into('>H',memory[dest],off,address&0xffff)
        elif kind==5:struct.pack_into('>H',memory[dest],off,(address>>16)&0xffff)
        else:struct.pack_into('>H',memory[dest],off,((address+0x8000)>>16)&0xffff)
    def pointer(address,size):
        for i,base in enumerate(bases):
            if base<=address and address-base+size<=len(memory[i]):return memory[i],address-base
        raise AssertionError('Bad loaded pointer: '+hex(address))
    def u32(address):
        b,at=pointer(address,4);return struct.unpack_from('>I',b,at)[0]
    module_off=headers[0][4]-headers[0][2]
    m=memory[0][module_off:module_off+52];assert len(m)==52
    assert m[4:32].split(b'\0')[0]==b'PS3GameOrbitService'
    toc,exp,end,imp,imp_end=struct.unpack_from('>IIIII',m,32)
    pointer(toc,1);assert not imp and not imp_end and end-exp==28
    b,at=pointer(exp,28)
    length,_,_,flags,count,variables,_=struct.unpack_from('>BBHHHHH',b,at)
    assert length==28 and flags==0x8000 and count==2 and variables==0
    name,nids,stubs,_=struct.unpack_from('>IIII',b,at+12);assert name==0
    assert [u32(nids+i*4) for i in range(2)]==[0xab779874,0xbc9a0086]
    for i in range(2):
        descriptor=u32(stubs+i*4);code,entry_toc=u32(descriptor),u32(descriptor+4)
        assert bases[0]<=code<bases[0]+headers[0][5] and entry_toc==toc
    print('PASS: PRX big-endian PPC64; every relocation simulated with nonadjacent segments; module/export/OPD pointers and TOC valid; no VSH imports')
    return data

if __name__=='__main__':
    verify(sys.argv[1])
    if len(sys.argv)>2:
        self=Path(sys.argv[2]).read_bytes();assert self[:4]==b'SCE\0'
        assert b'\x10\x70\x00\x00\x52\x00\x00\x01' in self[:1024], 'SPRX auth ID required'
        print('PASS: signed resident SPRX SELF header and VSH-plugin auth ID')
