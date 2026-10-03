#!/usr/bin/env python3
"""Generate native ranges from the approved original JFX Blu-ray OBJ."""
from pathlib import Path
import json, math, hashlib

ROOT=Path(__file__).resolve().parent.parent
source=ROOT/'assets/jfx_bluray/Bluray_Tris.obj'
assert hashlib.sha256(source.read_bytes()).hexdigest()=='4263705eb3e9801ee10d173094865fb50c5b52dfd52d2f1160390a09e3034c55'
positions=[];normals=[];uvs=[];faces={};material=None
for line in source.read_text().splitlines():
    a=line.split()
    if not a:continue
    if a[0]=='v':positions.append(tuple(map(float,a[1:4])))
    elif a[0]=='vn':normals.append(tuple(map(float,a[1:4])))
    elif a[0]=='vt':uvs.append(tuple(map(float,a[1:3])))
    elif a[0]=='usemtl':material=a[1];faces.setdefault(material,[])
    elif a[0]=='f':
        assert len(a)==4
        faces[material].append([tuple(int(n)-1 for n in c.split('/')) for c in a[1:]])
lo=[min(p[i] for p in positions) for i in range(3)]
hi=[max(p[i] for p in positions) for i in range(3)]
center=[(lo[i]+hi[i])/2 for i in range(3)];scale=170/(hi[1]-lo[1])
paper={'front':[],'back':[],'spine':[]}
for face in faces['cover']:
    us=[uvs[c[1]][0] for c in face]
    name='back' if max(us)<=.4648+1e-6 else 'front' if min(us)>=.5332-1e-6 else 'spine'
    paper[name].append(face)
assert all(paper.values())
ranges=[('plastic','ShellEdge','ClearPlastic',faces['case'],[1,1,1,.14]),
        ('front','CoverFront','Paper',paper['front'],[.82,.82,.82,1]),
        ('back','CoverBack','Paper',paper['back'],[.82,.82,.82,1]),
        ('spine','CoverSpine','Paper',paper['spine'],[.82,.82,.82,1]),
        ('logo','ShellEdge','Logo',faces['logo'],[.75,.75,.75,1])]
def fmt(x):
    s=format(x,'.9g');return s+('f' if '.' in s or 'e' in s else '.0f')
output=['// Generated from the approved original OBJ. Do not hand-edit.']
parts=[];total=0
for name,surface,material,triangles,color in ranges:
    verts=[];indices=[];lookup={}
    for face in triangles:
        for pi,ti,ni in face:
            key=(pi,ti,normals[ni])
            if key not in lookup:
                lookup[key]=len(verts)
                p=[(positions[pi][i]-center[i])*scale for i in range(3)]
                n=normals[ni];length=math.sqrt(sum(x*x for x in n))
                n=[x/length for x in n] if length else [0,0,1]
                u,v=uvs[ti]
                if name in paper:u=(u-.0469)/(.951-.0469);v=1-(v-.0299)/(.4936-.0299)
                verts.append(p+n+[u,v])
            indices.append(lookup[key])
    assert max(indices)<65536
    output.append('static const V14Vertex jfx_'+name+'_vertices[]={')
    output.extend('    {'+','.join(fmt(x) for x in v)+'},' for v in verts)
    output.append('};\nstatic const std::uint16_t jfx_'+name+'_indices[]={')
    output.extend('    '+','.join(map(str,indices[i:i+24]))+',' for i in range(0,len(indices),24))
    output.append('};')
    parts.append({'name':name,'surface':surface,'material':material,'color':color,'vertices':len(verts),'triangles':len(triangles)})
    total+=len(triangles)
assert total==5597
output.append('V14CaseMesh JfxCaseFix29::build(){\n    V14CaseMesh mesh;mesh.jfx=true;')
for p in parts:
    n=p['name']
    output+=['    { V14MeshPart p;p.surface=V14Surface::'+p['surface']+';',
       '      p.material=V14MeshPart::Material::'+p['material']+';p.textured='+('true' if n in paper else 'false')+';',
       '      p.color={{'+','.join(fmt(x) for x in p['color'])+'}};',
       '      p.vertices.assign(std::begin(jfx_'+n+'_vertices),std::end(jfx_'+n+'_vertices));',
       '      p.indices.assign(std::begin(jfx_'+n+'_indices),std::end(jfx_'+n+'_indices));',
       '      mesh.parts.push_back(std::move(p)); }']
output+=['    return mesh;\n}']
path=ROOT/'src/jfx_case_data_fix29.inc';path.write_text('\n'.join(output)+'\n')
manifest={'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'source_geometry_preserved':True,
    'triangles':total,'native_parts':parts,'center':center,'uniform_scale':scale,
    'dimensions_mm':[(hi[i]-lo[i])*scale for i in range(3)],
    'native_shader':'Archived V14 shader: alpha blending, no physical refraction/specular PBR.',
    'native_plastic_rgba':[1,1,1,.14],'draws_per_case':6,'generated_sha256':hashlib.sha256(path.read_bytes()).hexdigest()}
(ROOT/'assets/jfx_bluray/NATIVE_IMPORT_FIX29.json').write_text(json.dumps(manifest,indent=2)+'\n')
print('PASS: original JFX OBJ imported; '+str(total)+' triangles, '+str(len(parts))+' GPU ranges, 6 draws per case')
