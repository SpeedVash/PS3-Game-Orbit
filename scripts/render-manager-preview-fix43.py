#!/usr/bin/env python3
"""Render manager pixels through the actual uploaded native HUD vertices and UVs."""
from pathlib import Path
import json
import subprocess
import sys
import tempfile
from PIL import Image

root=Path(__file__).resolve().parent.parent
output=root/'assets/previews/GERENCIADOR_1_5_3.png'
with tempfile.TemporaryDirectory(prefix='orbit43-preview-') as directory:
    folder=Path(directory)
    subprocess.run([sys.executable,str(root/'scripts/check-manager-hud-fix43.py'),directory],check=True)
    bitmap=Image.frombytes('RGBA',(1280,512),(folder/'manager.rgba').read_bytes())
    geometry=json.loads((folder/'manager-geometry.json').read_text())
    def project(vertices):
        screen=Image.new('RGBA',(1280,720),(16,19,23,255))
        for i in range(0,len(vertices),4):
            a,b,c,d=vertices[i:i+4]
            target=(round((a[0]+1)*640),round((1-a[1])*360),round((c[0]+1)*640),round((1-c[1])*360))
            source=(round(a[2]*1280),round(a[3]*512),round(c[2]*1280),round(c[3]*512))
            image=bitmap.crop(source).resize((target[2]-target[0],target[3]-target[1]),Image.Resampling.BILINEAR)
            screen.alpha_composite(image,target[:2])
        return screen.convert('RGB')
    output.parent.mkdir(parents=True,exist_ok=True)
    project(geometry['after']).save(output)
    if len(sys.argv)==2:
        project(geometry['before']).save(Path(sys.argv[1]).resolve())
print('PASS: preview uses actual manager bitmap and native uploaded mesh/UVs: '+str(output))
print('LIMIT: computer projection, not a physical PS3 screenshot')
