#!/usr/bin/env python3
"""Exercise current native background draw functions with observable RSX API stubs."""
from pathlib import Path
import hashlib
import re
import subprocess
import tempfile

root=Path(__file__).resolve().parent.parent
renderer=(root/'src/rsx_renderer_v10.cpp').read_text()
waves=(root/'src/background_renderer_fix36.cpp').read_text()
def function(source,signature):
    start=source.index(signature);end=source.index('{',start)+1;depth=1
    while depth:
        depth+=(source[end]=='{')-(source[end]=='}');end+=1
    return source[start:end]
body=function(renderer,'bool RsxRendererV10::draw_orbit_background_ps3(')+'\n'+function(waves,'bool RsxRendererV10::draw_orbit_waves_ps3(')
constants=sorted(set(re.findall(r'\bGCM_[A-Z0-9_]+\b',body)))
defines=''.join(f'constexpr u32 {name}={0 if name=="GCM_FALSE" else 1 if name=="GCM_TRUE" else i+2};\n' for i,name in enumerate(constants))
prefix=r'''
#include <cassert>
#include <algorithm>
#include <array>
#include <cmath>
#define __PSL1GHT__ 1
#define private public
#include "rsx_renderer_v10.h"
#undef private
using u8=std::uint8_t;using u32=std::uint32_t;
CONSTANTS
struct gcmContextData{u32* current;u32* end;};
struct rsxVertexProgram{};struct rsxFragmentProgram{};struct rsxProgramConst{int id;};
static u32 commands[16384];static gcmContextData ctx{commands,commands+16383};
static unsigned draws,bindings,reloads,bound_texture,matrix_uploads;
static bool depth,write,cull,blend,animated;
static float pending_alpha,active_alpha;
static rsxVertexProgram vp;static rsxFragmentProgram fp;
static rsxProgramConst mvp{1},model{2},uv{3},color{4},use{5},alpha{6};
static void emit(gcmContextData* c){assert(c->current+4<c->end);c->current+=4;}
static void rsxSetDepthTestEnable(gcmContextData* c,u32 value){depth=value;emit(c);}
static void rsxSetDepthWriteEnable(gcmContextData* c,u32 value){write=value;emit(c);}
static void rsxSetCullFaceEnable(gcmContextData* c,u32 value){cull=value;emit(c);}
static void rsxSetBlendEnable(gcmContextData* c,u32 value){blend=value;emit(c);}
static void rsxSetVertexProgramParameter(gcmContextData* c,const rsxVertexProgram*,const rsxProgramConst* p,const float* value){
 if(p->id==1 || p->id==2){for(int i=0;i<16;++i)assert(value[i]==(i%5==0?1.f:0.f));++matrix_uploads;}
 if(p->id==3)assert(value[0]==1 && value[1]==1 && value[2]==0 && value[3]==0);
 emit(c);
}
static void rsxSetFragmentProgramParameter(gcmContextData* c,const rsxFragmentProgram*,const rsxProgramConst* p,const float* value,u32,u32){
 if(p->id==5)assert(value[0]==1);
 if(p->id==6)pending_alpha=value[0];
 emit(c);
}
static void rsxLoadFragmentProgramLocation(gcmContextData* c,const rsxFragmentProgram*,u32,u32){active_alpha=pending_alpha;++reloads;emit(c);}
static void rsxBindVertexArrayAttrib(gcmContextData* c,int,int,u32,u8 stride,int,u32,u32){assert(stride==sizeof(V14Vertex));++bindings;emit(c);}
static void rsxDrawIndexArray(gcmContextData* c,u32 type,u32 offset,u32 count,u32 index_type,u32 location){
 assert(type==GCM_TYPE_TRIANGLES && index_type==GCM_INDEX_TYPE_16B && location==GCM_LOCATION_RSX);
 assert(!depth && !write && !cull && matrix_uploads==2);
 if(draws==0){assert(count==6 && offset==100 && !blend && active_alpha==1);assert(bound_texture==(animated?20u:10u));}
 else {const unsigned layer=3-draws;assert(blend && bound_texture==30 && count==OrbitBackgroundFix36::IndicesPerRibbon);
  assert(offset==200+layer*OrbitBackgroundFix36::IndicesPerRibbon*2);
  assert(std::abs(active_alpha-(layer==0?1.f:layer==1?.47f:.22f))<.0001f);}
 ++draws;emit(c);
}
void RsxRendererV10::setup_texture_ps3(const GpuTextureStage1& tex){assert(tex.uploaded);bound_texture=tex.gpu_offset;emit(&ctx);}
'''.replace('CONSTANTS',defines)
suffix=r'''
int main(){
 RsxStage1 stage;stage.context_=&ctx;RsxRendererV10 r;r.stage1_=&stage;
 r.vertex_program_=&vp;r.fragment_program_=&fp;r.vp_mvp_=&mvp;r.vp_model_=&model;r.vp_uv_transform_=&uv;
 r.fp_base_color_=&color;r.fp_use_texture_=&use;r.fp_alpha_=&alpha;
 r.background_texture_.uploaded=r.animated_base_texture_.uploaded=r.wave_texture_.uploaded=true;
 r.background_texture_.gpu_offset=10;r.animated_base_texture_.gpu_offset=20;r.wave_texture_.gpu_offset=30;
 r.background_mesh_.vertices=r.background_mesh_.indices=commands;r.background_mesh_.index_offset=100;r.background_mesh_.index_count=6;
 r.wave_mesh_.vertices=r.wave_mesh_.indices=commands;r.wave_mesh_.index_offset=200;
 for(bool enabled:{true,false}){
  ctx={commands,commands+16383};draws=bindings=reloads=matrix_uploads=0;depth=write=cull=true;blend=false;
  animated=r.animated_background_=enabled;r.last_stats_.wave_draw_calls=0;
  assert(r.draw_orbit_background_ps3());assert(draws==(enabled?4u:1u) && reloads==draws && bindings==(enabled?6u:3u));
  assert(r.last_stats_.wave_draw_calls==(enabled?3u:0u) && !blend);
 }
 ctx={commands,commands+100};draws=0;r.animated_background_=true;assert(!r.draw_orbit_waves_ps3() && draws==0);
 ctx={commands,commands+16383};r.wave_mesh_.vertices=nullptr;assert(!r.draw_orbit_waves_ps3() && draws==0);
 r.animated_background_=false;assert(r.draw_orbit_waves_ps3() && ctx.current==commands && draws==0);
 puts("PASS: actual FIX36 native draw uses identity matrices, no depth/cull, correct static/animated textures, three ordered alpha-blended indexed ribbons and fragment reloads");
 puts("PASS: disabled waves emit no GPU commands; insufficient command room/missing geometry reject before wave draws");
 puts("LIMIT: API simulation does not measure native FIFO words, RSX sampling, TV output or console performance");
}
'''
print('BACKGROUND_SOURCE_SHA256: '+hashlib.sha256(waves.encode()).hexdigest(),flush=True)
with tempfile.TemporaryDirectory(prefix='orbit36-background-') as directory:
    cpp=Path(directory)/'test.cpp';binary=Path(directory)/'test';cpp.write_text(prefix+body+suffix)
    subprocess.run(['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror',*[f'-DPS3_GAME_ORBIT_FIX{i}=1' for i in range(30,37)],'-I'+str(root/'include'),str(cpp),str(root/'src/math3d.cpp'),'-o',str(binary)],check=True,timeout=40)
    subprocess.run([str(binary)],check=True,timeout=15)
