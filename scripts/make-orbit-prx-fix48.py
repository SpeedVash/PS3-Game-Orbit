#!/usr/bin/env python3
"""Build a freestanding PRX from an emit-relocs PPC64 ELF, retaining every load relocation."""
import argparse
import json
from pathlib import Path
import struct

def convert(input_path, output_path):
    b = bytearray(Path(input_path).read_bytes())
    assert b[:6] == b'\x7fELF\x02\x02', 'PPC64 big endian ELF required'
    assert struct.unpack_from('>H', b, 18)[0] == 21
    phoff = struct.unpack_from('>Q', b, 32)[0]
    shoff = struct.unpack_from('>Q', b, 40)[0]
    phsize, phnum, shsize, shnum, shnames = struct.unpack_from('>HHHHH', b, 54)
    assert phsize == 56 and shsize == 64
    sections = [struct.unpack_from('>IIQQQQIIQQ', b, shoff+i*64) for i in range(shnum)]
    names = sections[shnames]
    strings = b[names[4]:names[4]+names[5]]
    section_names = [strings[x[0]:].split(b'\0', 1)[0].decode() for x in sections]
    segments = []
    for i in range(phnum):
        p = struct.unpack_from('>IIQQQQQQ', b, phoff+i*56)
        if p[0] == 1:
            segments.append(p)
    assert len(segments) == 2, 'Exactly code and data LOAD segments required'
    def locate(address, width=1):
        for i,p in enumerate(segments):
            if address >= p[3] and address-p[3]+width <= p[6]:
                return i, address-p[3]
        raise AssertionError('Address outside module segments: '+hex(address))
    def file_offset(address, width=1):
        i, off = locate(address, width)
        assert off+width <= segments[i][5]
        return segments[i][2]+off
    tables = {}
    for i,s in enumerate(sections):
        if s[1] == 2:
            strtab = sections[s[6]]
            text = b[strtab[4]:strtab[4]+strtab[5]]
            table = []
            for at in range(s[4],s[4]+s[5],24):
                symbol = struct.unpack_from('>IBBHQQ', b, at)
                label = text[symbol[0]:].split(b'\0',1)[0].decode()
                table.append((label,symbol))
            tables[i] = table
    symbols = {name:s for table in tables.values() for name,s in table if name}
    module = symbols['orbit_module_info'][4]
    assert locate(module,52)[0] == 0
    assert len(b[file_offset(module)+4:file_offset(module)+32].split(b'\0',1)[0]) <= 27
    absolute = {1:4,4:2,5:2,6:2,38:8}
    fixed = {0,10,44,45,46,47,48,49,50,63,64}  # resolved branches and segment-local TOC offsets
    relocation_map = {}
    skipped = {}
    for s in sections:
        if s[1] != 4 or not (sections[s[7]][2]&2):
            continue
        table = tables[s[6]]
        for at in range(s[4],s[4]+s[5],24):
            address,info,addend = struct.unpack_from('>QQq',b,at)
            kind = info & 0xffffffff
            index = info >> 32
            label,symbol = table[index]
            if kind == 51:  # R_PPC64_TOC is an absolute runtime TOC pointer, not a relative instruction.
                target = struct.unpack_from('>Q',b,file_offset(address,8))[0]
                destination,off = locate(address,8)
                value,relative = locate(target)
                relocation_map[(destination,off)] = (off,0,value,destination,38,relative)
                continue
            if kind in fixed:
                skipped[kind] = skipped.get(kind,0)+1
                continue
            assert kind in absolute, ('Unhandled PRX relocation',kind,label,hex(address))
            assert symbol[3] != 0, ('Undefined symbol',label)
            target = symbol[4]+addend
            destination,off = locate(address,absolute[kind])
            value,relative = locate(target)
            relocation_map[(destination,off)] = (off,0,value,destination,kind,relative)
    # The public PPU ABI uses the final 8 bytes of each 24-byte OPD for its 32-bit descriptor.
    if '.opd' in section_names:
        s = sections[section_names.index('.opd')]
        assert s[5]%24 == 0
        for address in range(s[3],s[3]+s[5],24):
            off = file_offset(address,24)
            function,toc,_ = struct.unpack_from('>QQQ',b,off)
            struct.pack_into('>II',b,off+16,function,toc)
            for delta,target in [(16,function),(20,toc)]:
                destination,relative = locate(address+delta,4)
                value,value_offset = locate(target)
                relocation_map[(destination,relative)] = (relative,0,value,destination,1,value_offset)
    relocations = b''.join(struct.pack('>QHBBIQ',*r) for _,r in sorted(relocation_map.items()))
    # New compact ELF has no sections or debug data, and exactly two LOAD + one PRX reloc header.
    header_size = 64+3*56
    offset = (header_size+15)&~15
    payloads = []
    headers = []
    for i,p in enumerate(segments):
        data = bytes(b[p[2]:p[2]+p[5]])
        physical = offset+(module-p[3]) if i==0 else 0
        headers.append((1,p[1],offset,p[3],physical,len(data),p[6],16))
        payloads.append((offset,data));offset=(offset+len(data)+15)&~15
    headers.append((0x700000a4,0,offset,0,0,len(relocations),0,16))
    payloads.append((offset,relocations))
    out = bytearray(offset+len(relocations))
    out[:64] = b[:64]
    out[7] = 102
    struct.pack_into('>H',out,16,0xffa4)
    struct.pack_into('>Q',out,24,0)
    struct.pack_into('>Q',out,32,64)
    struct.pack_into('>Q',out,40,0)
    struct.pack_into('>HHHHH',out,54,56,3,0,0,0)
    for i,p in enumerate(headers):
        struct.pack_into('>IIQQQQQQ',out,64+i*56,*p)
    for off,data in payloads:
        out[off:off+len(data)] = data
    Path(output_path).write_bytes(out)
    return {'module_info':hex(module),'load_segments':2,'relocations':len(relocation_map),
        'fixed_relocations':skipped,'imports':0,'size':len(out),'memory':sum(p[6] for p in segments)}

if __name__ == '__main__':
    p = argparse.ArgumentParser()
    p.add_argument('input');p.add_argument('output')
    a = p.parse_args()
    print(json.dumps(convert(a.input,a.output),indent=2))
