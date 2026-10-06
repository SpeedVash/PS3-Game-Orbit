#!/usr/bin/env python3
"""Deterministic code-native Blu-ray reflective underside; no shader rebuild."""
from pathlib import Path
from PIL import Image
import math, colorsys, json, hashlib
root=Path(__file__).resolve().parent.parent
image=Image.new('RGBA',(512,512));px=image.load()
for y in range(512):
 for x in range(512):
  dx=(x-255.5)/256;dy=(y-255.5)/256;r=math.hypot(dx,dy);a=math.atan2(dy,dx)
  # Broad metallic highlights with a cool silver substrate and faint grooves.
  beam=abs(math.cos(a-.46))**16
  shadow=abs(math.sin(a+.30))**5
  groove=math.sin(r*1650)*1.3
  metal=118+94*beam-24*shadow+groove+16*math.exp(-((r-.87)/.20)**2)
  rgb=colorsys.hsv_to_rgb((a/(2*math.pi)+.56+r*.22)%1,1,1)
  sheen=13*(.35+.65*beam)*max(0,min(1,(r-.16)/.32))
  color=[metal-6+sheen*rgb[0],metal+1+sheen*rgb[1],metal+13+sheen*rgb[2]]
  if .133<=r<.225:color=[147+36*beam,153+37*beam,164+37*beam]
  if abs(r-.225)<.003 or abs(r-.985)<.003:color=[91,99,114]
  px[x,y]=tuple(max(0,min(255,round(c))) for c in color)+(255,)
path=root/'pkgfiles/USRDIR/ORBIT_DISC_BACK.png';image.save(path,optimize=True)
(root/'assets/branding/DISC_BACK_1_4_1.json').write_text(json.dumps({'asset':path.name,'size':[512,512],'type':'procedural cool-silver Blu-ray underside','runtime':'archived shader + texture, no real environment reflection','sha256':hashlib.sha256(path.read_bytes()).hexdigest()},indent=2)+'\n')
print('PASS: procedural reflective silver Blu-ray underside, 512x512 RGBA')
