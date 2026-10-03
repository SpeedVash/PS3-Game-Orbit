#!/usr/bin/env python3
"""Bake licensed antialiased Noto Sans Light glyphs and normalize XMB assets.

The generated icon is a project asset; resizing here is format preparation.
No font library or TrueType rasterizer is needed on the PS3.
"""
from pathlib import Path
from PIL import Image, ImageFont
import hashlib, json

ROOT=Path(__file__).resolve().parent.parent
font_path=ROOT/'assets/fonts/NotoSans-Variable.ttf'
codes=list(range(32,256))+list(range(256,384))+list(range(880,1024))+list(range(1024,1120))+[0x2013,0x2014,0x2018,0x2019,0x201c,0x201d,0x2026]
glyphs=[];pixels=bytearray()
for size in (18,20,28):
    font=ImageFont.truetype(str(font_path),size)
    values=[300 if a['name']==b'Weight' else a['default'] for a in font.get_variation_axes()]
    font.set_variation_by_axes(values)
    for code in codes:
        mask,offset=font.getmask2(chr(code),mode='L',anchor='ls')
        offset=(offset[0],offset[1]-font.getbbox('H',anchor='ls')[1])
        w,h=mask.size;advance=round(font.getlength(chr(code)))
        glyphs.append((size,code,len(pixels),w,h,*offset,advance))
        pixels.extend(bytes(mask))
out=ROOT/'src/orbit_font_fix30.inc'
with out.open('w') as f:
    f.write('// Generated from Noto Sans, weight 300. SIL OFL; assets/fonts/OFL.txt.\n')
    f.write('struct OrbitGlyph {unsigned short size,code;unsigned offset;unsigned short width,height;short x,y,advance;};\n')
    f.write('static constexpr OrbitGlyph OrbitGlyphs[]={\n')
    for g in glyphs:f.write('{'+','.join(map(str,g))+'},\n')
    f.write('};\nstatic constexpr unsigned char OrbitGlyphPixels[]={\n')
    for i in range(0,len(pixels),48):f.write(','.join(map(str,pixels[i:i+48]))+',\n')
    f.write('};\n')
source=Image.open(ROOT/'assets/branding/PS3_GAME_ORBIT_ICON_ORIGINAL.png').convert('RGB')
source.resize((320,176),Image.Resampling.LANCZOS).save(ROOT/'pkgfiles/ICON0.PNG')
source.resize((1280,720),Image.Resampling.LANCZOS).save(ROOT/'pkgfiles/USRDIR/ORBIT_SPLASH.png')
(ROOT/'assets/fonts/FONT_INFO.json').write_text(json.dumps({'font':'Noto Sans','weight':300,'sizes':[18,20,28],'source':'https://github.com/google/fonts/tree/main/ofl/notosans','source_sha256':hashlib.sha256(font_path.read_bytes()).hexdigest(),'glyphs':len(glyphs),'mask_bytes':len(pixels),'generated_sha256':hashlib.sha256(out.read_bytes()).hexdigest(),'license':'OFL.txt','runtime':'baked grayscale coverage; no TrueType dependency'},indent=2))
print('PASS: Noto Sans Light atlas;',len(glyphs),'glyphs;',len(pixels),'coverage bytes; ICON0 320x176 and splash 1280x720')
