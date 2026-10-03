#!/usr/bin/env python3
"""Independent comparison of every native triangle and cover UV with the OBJ."""
import hashlib,json,math,re
from collections import Counter
from pathlib import Path
ROOT=Path(__file__).resolve().parent.parent
obj=ROOT/'assets/jfx_bluray/Bluray_Tris.obj';inc=ROOT/'src/jfx_case_data_fix29.inc'
positions=[];normals=[];uvs=[];faces={};group=None
for line in obj.read_text().splitlines():
    p=line.split()
    if not p:continue
    if p[0]=='v':positions.append(tuple(map(float,p[1:4])))
    elif p[0]=='vn':normals.append(tuple(map(float,p[1:4])))
    elif p[0]=='vt':uvs.append(tuple(map(float,p[1:3])))
    elif p[0]=='usemtl':group=p[1];faces.setdefault(group,[])
    elif p[0]=='f':faces[group].append([tuple(int(v)-1 for v in x.split('/')) for x in p[1:]])
lo=[min(p[i] for p in positions) for i in range(3)];hi=[max(p[i] for p in positions) for i in range(3)]
center=[(lo[i]+hi[i])/2 for i in range(3)];scale=170/(hi[1]-lo[1])
def source_vertex(c,cover):
    pi,ti,ni=c;p=[(positions[pi][i]-center[i])*scale for i in range(3)];n=normals[ni];l=math.sqrt(sum(x*x for x in n));n=[x/l for x in n] if l else [0,0,1]
    u,v=uvs[ti]
    if cover:u=(u-.0469)/(.951-.0469);v=1-(v-.0299)/(.4936-.0299)
    return p+n+[u,v]
def signature(tri):return tuple(tuple(round(x,5) for x in v) for v in tri)
expected={name:[] for name in ('plastic','front','back','spine','logo')}
for group,fs in faces.items():
    for f in fs:
        name='plastic' if group=='case' else 'logo' if group=='logo' else 'back' if max(uvs[c[1]][0] for c in f)<=.464801 else 'front' if min(uvs[c[1]][0] for c in f)>=.533199 else 'spine'
        expected[name].append([source_vertex(c,group=='cover') for c in f])
parts={};text=inc.read_text();vertices=0;triangles=0;max_error=0
for name,raw in re.findall(r'static const V14Vertex jfx_(\w+)_vertices\[\]=\{(.*?)\n\};',text,re.S):
    vs=[[float(x.replace('f','')) for x in row.split(',')] for row in re.findall(r'\{([^{}]+)\}',raw)]
    ids=list(map(int,re.search(r'jfx_'+name+r'_indices\[\]=\{(.*?)\n\};',text,re.S)[1].replace('\n','').replace(' ','').strip(',').split(',')))
    assert len(ids)%3==0 and max(ids)<len(vs)
    assert all(len(v)==8 and all(math.isfinite(x) for x in v) and abs(sum(x*x for x in v[3:6])-1)<2e-6 for v in vs)
    group='case' if name=='plastic' else 'logo' if name=='logo' else 'cover'
    assert len(ids)//3==len(expected[name])
    for i in range(0,len(ids),3):
        actual=[vs[n] for n in ids[i:i+3]];original=expected[name][i//3]
        error=max(abs(a-b) for av,bv in zip(actual,original) for a,b in zip(av,bv))
        max_error=max(max_error,error)
        assert error<1e-6,(name,i,error)
    parts[name]={'vertices':len(vs),'triangles':len(ids)//3};vertices+=len(vs);triangles+=len(ids)//3
    if group=='cover':assert all(-1e-7<=x<=1+1e-7 for v in vs for x in v[6:])
assert set(parts)==set(expected)
assert triangles==5597 and set(parts)=={'plastic','front','back','spine','logo'}
manifest=json.loads((ROOT/'assets/jfx_bluray/NATIVE_IMPORT_FIX29.json').read_text())
assert manifest['native_plastic_rgba']==[1,1,1,.14]
assert manifest['source_sha256']==hashlib.sha256(obj.read_bytes()).hexdigest()
assert manifest['generated_sha256']==hashlib.sha256(inc.read_bytes()).hexdigest()
report={'passed':True,'source_sha256':manifest['source_sha256'],'triangles':triangles,'render_vertices':vertices,'max_decimal_attribute_error':max_error,'ranges':parts,
        'verification':'Every source triangle matched in order, positions, normalized normals and transformed cover UVs; all attributes within 0.000001 mm / normalized attribute units.',
        'native_material':'Unpigmented RGB [1,1,1], alpha 0.14; opaque original logo; archived RSX shader. No PBR refraction.'}
(ROOT/'assets/jfx_bluray/VERIFICACAO_NATIVA_FIX29.json').write_text(json.dumps(report,indent=2)+'\n')
print('PASS: all 5597 native triangles, normals and original cover UVs match the approved OBJ; neutral plastic and original 3D logo')
