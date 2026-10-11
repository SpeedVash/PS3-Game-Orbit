#!/usr/bin/env python3
"""Run actual native HUD methods with observable RSX stubs; optionally export pixels/UVs."""
from pathlib import Path
import hashlib
import re
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parent.parent
source = (root/'src/rsx_renderer_v10.cpp').read_text()

def function(text, signature):
    start = text.index(signature)
    end = text.index('{', start) + 1
    depth = 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]

flags = ['-D__PSL1GHT__=1','-DPS3_SP_LOADER_FIX28=1','-DPS3_SP_LOADER_FIX29=1',*[f'-DPS3_GAME_ORBIT_FIX{i}=1' for i in range(30,44)]]
native = subprocess.run(['g++','-E','-P','-x','c++',*flags,'-'],
                        input=re.sub(r'^\s*#include[^\n]*', '', source, flags=re.M),
                        text=True,capture_output=True,check=True).stdout
body = '\n'.join(function(native, signature) for signature in (
    'bool RsxRendererV10::set_library_hud(',
    'bool RsxRendererV10::prepare_file_manager_geometry_ps3(',
    'bool RsxRendererV10::set_file_manager_hud(',
    'bool RsxRendererV10::draw_library_hud_ps3(',
    'void RsxRendererV10::shutdown(',
))
# Host cannot execute PowerPC sync. All remaining production method bodies are unchanged.
body = body.replace('__asm__ volatile("sync" ::: "memory");', 'publish_geometry();')
constants = sorted(set(re.findall(r'\bGCM_[A-Z0-9_]+\b', body)))
defines = ''.join(f'constexpr u32 {name}={0 if name == "GCM_FALSE" else 1 if name == "GCM_TRUE" else i+2};\n'
                  for i, name in enumerate(constants))
prefix = r'''
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#define __PSL1GHT__ 1
#define private public
#include "rsx_renderer_v10.h"
#undef private
#include "orbit_ui_fix30.h"
#include "performance_fix35.h"
#include "case_style_fix41.h"
using u8=std::uint8_t;using u32=std::uint32_t;
CONSTANTS
struct gcmContextData{u32* current;u32* end;};
struct rsxVertexProgram{};struct rsxFragmentProgram{};struct rsxProgramConst{int id;};
static u32 commands[16384];static gcmContextData ctx{commands,commands+16383};
static std::map<void*,std::pair<std::size_t,u32>> allocations;
static unsigned alloc_calls,map_calls,publishes,uploads,draws;
static unsigned fail_alloc,fail_map;
static bool fail_upload,depth,write,cull,blend;
static u32 next_offset=0x1000,bound_vertex,draw_offset,draw_count;
static rsxVertexProgram vp;static rsxFragmentProgram fp;
static rsxProgramConst mvp{1},model{2},uv{3},color{4},use{5},alpha{6};
static DecodedImageRGBA displayed;
static std::vector<HudRectFix35> uploaded_regions;
static void publish_geometry(){++publishes;}
static void* rsxMemalign(std::size_t alignment,std::size_t bytes){
 ++alloc_calls;if(alloc_calls==fail_alloc)return nullptr;
 void* p=nullptr;assert(posix_memalign(&p,alignment,bytes)==0);
 allocations.emplace(p,std::make_pair(bytes,next_offset));next_offset+=0x1000;return p;
}
static void rsxFree(void* p){assert(allocations.erase(p)==1);free(p);}
static int rsxAddressToOffset(void* p,u32* offset){
 ++map_calls;if(map_calls==fail_map)return -1;
 *offset=allocations.at(p).second;return 0;
}
static void emit(gcmContextData* c){assert(c->current+4<c->end);c->current+=4;}
static void rsxSetDepthTestEnable(gcmContextData* c,u32 v){depth=v;emit(c);}
static void rsxSetDepthWriteEnable(gcmContextData* c,u32 v){write=v;emit(c);}
static void rsxSetCullFaceEnable(gcmContextData* c,u32 v){cull=v;emit(c);}
static void rsxSetBlendEnable(gcmContextData* c,u32 v){blend=v;emit(c);}
static void rsxSetVertexProgramParameter(gcmContextData* c,const rsxVertexProgram*,const rsxProgramConst* p,const float* v){
 if(p->id==1 || p->id==2)for(int i=0;i<16;++i)assert(v[i]==(i%5==0?1.f:0.f));
 if(p->id==3)assert(v[0]==1 && v[1]==1 && v[2]==0 && v[3]==0);
 emit(c);
}
static void rsxSetFragmentProgramParameter(gcmContextData* c,const rsxFragmentProgram*,const rsxProgramConst* p,const float* v,u32,u32){
 if(p->id==4)for(int i=0;i<4;++i)assert(v[i]==1);
 if(p->id==5 || p->id==6)assert(v[0]==1);
 emit(c);
}
static void rsxLoadFragmentProgramLocation(gcmContextData* c,const rsxFragmentProgram*,u32,u32){emit(c);}
static void rsxBindVertexArrayAttrib(gcmContextData* c,int attr,int,u32 offset,u8 stride,int size,u32 type,u32 location){
 assert(stride==sizeof(V14Vertex) && type==GCM_VERTEX_DATA_TYPE_F32 && location==GCM_LOCATION_RSX);
 if(attr==0){assert(size==3);bound_vertex=offset;}
 if(attr==1)assert(offset==bound_vertex+offsetof(V14Vertex,nx) && size==3);
 if(attr==2)assert(offset==bound_vertex+offsetof(V14Vertex,u) && size==2);
 emit(c);
}
static void rsxDrawIndexArray(gcmContextData* c,u32 type,u32 offset,u32 count,u32 index_type,u32 location){
 assert(type==GCM_TYPE_TRIANGLES && index_type==GCM_INDEX_TYPE_16B && location==GCM_LOCATION_RSX);
 assert(!depth && !write && !cull && blend);draw_offset=offset;draw_count=count;++draws;emit(c);
}
namespace OrbitPerformanceFix35 {std::uint64_t now_us(){return 1;}void record(Kind,std::uint64_t){}}
namespace GameOrderFix40 {const char* name(GameSortFix40){return "A até Z";}}
void RsxRendererV10::setup_texture_ps3(const GpuTextureStage1& tex){assert(tex.uploaded && tex.width==1280 && tex.height==512);emit(&ctx);}
void RsxRendererV10::clear_cover_textures(){assert(covers_.empty());}
void RsxRendererV10::release_geometry_ps3(){assert(gpu_mesh_.empty());}
void RsxStage1::release_cover(GpuTextureStage1& tex){tex={};}
bool RsxStage1::update_overlay_regions(const DecodedImageRGBA& image,GpuTextureStage1& out,const std::vector<HudRectFix35>& regions){
 ++uploads;if(fail_upload){fail_upload=false;last_error_="injected upload failure";return false;}
 assert(image.valid() && image.width==1280 && image.height==512);
 if(!displayed.valid())displayed=image;
 for(const auto& rect:regions){
  assert(rect.x>=0 && rect.y>=0 && rect.width>0 && rect.height>0);
  assert(rect.x+rect.width<=image.width && rect.y+rect.height<=image.height);
  for(int y=rect.y;y<rect.y+rect.height;++y){
   const auto offset=std::size_t(y)*image.pitch+rect.x*4;
   std::copy_n(image.rgba.data()+offset,rect.width*4,displayed.rgba.data()+offset);
  }
 }
 uploaded_regions=regions;out.uploaded=true;out.width=1280;out.height=512;out.pitch=5120;return true;
}
'''.replace('CONSTANTS', defines)
suffix = r'''
static void draw(RsxRendererV10& r,bool manager){
 ctx={commands,commands+16383};const unsigned before=draws;assert(r.draw_library_hud_ps3());assert(draws==before+1);
 const auto& mesh=manager?r.file_hud_mesh_:r.hud_mesh_;
 assert(bound_vertex==mesh.vertex_offset && draw_offset==mesh.index_offset && draw_count==(manager?6u:18u));
 if(manager){
  assert(mesh.vertex_count==4 && mesh.index_count==6);
  const auto* p=static_cast<const V14Vertex*>(mesh.vertices);
  const int xs[4]={0,1280,1280,0},ys[4]={104,104,616,616};
  const float us[4]={0,1,1,0},vs[4]={0,0,1,1};
  for(int i=0;i<4;++i){
   assert(std::abs((p[i].x+1)*640-xs[i])<.001f && std::abs((1-p[i].y)*360-ys[i])<.001f);
   assert(p[i].u==us[i] && p[i].v==vs[i]);
  }
  const auto* indices=static_cast<const std::uint16_t*>(mesh.indices);
  const std::uint16_t expected[6]={0,1,2,0,2,3};assert(std::equal(indices,indices+6,expected));
  // Ordered source rows must remain ordered on screen, at both display scales.
  for(float scale:{1.f,1.5f})for(int row=0;row<=512;++row){
   const float screen=(1-(p[0].y+(p[3].y-p[0].y)*row/512.f))*360*scale;
   assert(std::abs(screen-(104+row)*scale)<.002f);
  }
 }
}
static void export_mesh(std::ostream& out,const V14Vertex* vertices,std::size_t count){
 out<<'[';for(std::size_t i=0;i<count;++i){if(i)out<<',';const auto& p=vertices[i];out<<'['<<p.x<<','<<p.y<<','<<p.u<<','<<p.v<<']';}out<<']';
}
int main(int argc,char** argv){
 RsxStage1 stage;stage.context_=&ctx;RsxRendererV10 r;r.stage1_=&stage;r.ready_=true;
 r.vertex_program_=&vp;r.fragment_program_=&fp;r.vp_mvp_=&mvp;r.vp_model_=&model;r.vp_uv_transform_=&uv;
 r.fp_base_color_=&color;r.fp_use_texture_=&use;r.fp_alpha_=&alpha;
 r.attr_position_=0;r.attr_normal_=1;r.attr_uv_=2;
 LibraryHudLinesFix28 lines{{"PS3 Game Orbit","GAME","library","1 / 2","CLASSIC","OK"}};
 assert(r.set_library_hud(lines));draw(r,false);assert(allocations.size()==2);
 const auto* original=static_cast<const unsigned char*>(r.hud_mesh_.vertices);
 const std::vector<unsigned char> saved(original,original+12*sizeof(V14Vertex));
 FileManagerFix40::View view;view.active=true;view.revision=1;view.mode=FileManagerFix40::Mode::Browse;
 view.panels[0].path="/dev_hdd0/PS3ISO";view.panels[1].path="/dev_usb000/PS3ISO";
 view.panels[0].entries={{"..",true,false,0,false,false},{"Jogo Exemplo.iso",false,true,4294967296ULL,true,false}};
 view.panels[1].entries={{"..",true,false,0,false,false},{"Backup",true,false,0,false,false}};
 view.status="Escolha a origem e o destino.";view.marked_count=1;view.selected_items=1;
 view.selection_panel=0;view.selection_path=view.panels[0].path;view.panels[0].selected=1;
 for(unsigned relative:{1u,2u}){
  fail_alloc=alloc_calls+relative;assert(!r.set_file_manager_hud(view));fail_alloc=0;
  assert(allocations.size()==2 && !r.file_hud_active_ && !r.file_hud_mesh_.vertices);draw(r,false);
 }
 for(unsigned relative:{1u,2u}){
  fail_map=map_calls+relative;assert(!r.set_file_manager_hud(view));fail_map=0;
  assert(allocations.size()==2 && !r.file_hud_active_ && !r.file_hud_mesh_.vertices);draw(r,false);
 }
 fail_upload=true;assert(!r.set_file_manager_hud(view));assert(!r.file_hud_active_ && r.file_hud_revision_==0);draw(r,false);
 assert(r.set_file_manager_hud(view));draw(r,true);assert(allocations.size()==4);
 assert(uploaded_regions.size()==1 && uploaded_regions[0].y==0 && uploaded_regions[0].height==512);
 assert(displayed.rgba==OrbitUiFix30::file_manager_hud(view).rgba);
 const unsigned stable_alloc=alloc_calls,stable_upload=uploads,stable_publish=publishes;
 assert(r.set_file_manager_hud(view));assert(alloc_calls==stable_alloc && uploads==stable_upload && publishes==stable_publish);
 if(argc==2){
  const auto image=OrbitUiFix30::file_manager_hud(view);
  std::ofstream rgba(std::string(argv[1])+"/manager.rgba",std::ios::binary);rgba.write(reinterpret_cast<const char*>(image.rgba.data()),image.rgba.size());
  std::ofstream geometry(std::string(argv[1])+"/manager-geometry.json");geometry<<"{\"before\":";
  export_mesh(geometry,static_cast<const V14Vertex*>(r.hud_mesh_.vertices),12);geometry<<",\"after\":";
  export_mesh(geometry,static_cast<const V14Vertex*>(r.file_hud_mesh_.vertices),4);geometry<<'}';
 }
 for(auto mode:{FileManagerFix40::Mode::Notice,FileManagerFix40::Mode::Browse,FileManagerFix40::Mode::Actions,
    FileManagerFix40::Mode::Devices,FileManagerFix40::Mode::Details,FileManagerFix40::Mode::Planning,
    FileManagerFix40::Mode::Confirm,FileManagerFix40::Mode::Overwrite,FileManagerFix40::Mode::Copy,
    FileManagerFix40::Mode::Summary,FileManagerFix40::Mode::ExitConfirm}){
  view.mode=mode;++view.revision;assert(r.set_file_manager_hud(view));draw(r,true);
  assert(uploaded_regions.size()==1 && uploaded_regions[0].y==48 && uploaded_regions[0].height==464);
  assert(displayed.rgba==OrbitUiFix30::file_manager_hud(view).rgba);
  const auto revision=r.file_hud_revision_;++view.revision;fail_upload=true;
  assert(!r.set_file_manager_hud(view) && r.file_hud_active_ && r.file_hud_revision_==revision);draw(r,true);
  assert(r.set_file_manager_hud(view));draw(r,true);
  assert(r.set_library_hud(lines));assert(!r.file_hud_active_);draw(r,false);
  assert(displayed.rgba==OrbitUiFix30::hud(lines).rgba);
  assert(std::equal(saved.begin(),saved.end(),static_cast<const unsigned char*>(r.hud_mesh_.vertices)));
  assert(r.set_file_manager_hud(view));draw(r,true);
 }
 assert(alloc_calls==stable_alloc && allocations.size()==4);
 const auto before=draws;ctx={commands,commands+100};assert(!r.draw_library_hud_ps3() && draws==before);
 ctx={commands+100,commands};assert(!r.draw_library_hud_ps3() && draws==before);
 ctx={commands,commands+16383};void* indices=r.file_hud_mesh_.indices;r.file_hud_mesh_.indices=nullptr;
 assert(!r.draw_library_hud_ps3() && draws==before);r.file_hud_mesh_.indices=indices;
 r.shutdown();assert(allocations.empty() && !r.file_hud_mesh_.vertices && !r.file_hud_mesh_.indices && !r.ready_);
 r.shutdown();assert(allocations.empty());
 puts("PASS: actual native HUD switches library 18-index atlas to continuous 6-index manager; ordered rows at 720p/1080p scale");
 puts("PASS: all 11 manager modes, dirty uploads, revision caching and library restoration preserve bitmap and library geometry");
 puts("PASS: allocation/mapping/upload failures, repeated transitions, command limits and native shutdown; no leaked geometry");
 puts("LIMIT: host RSX simulation does not prove TV output; physical PS3 test required");
}
'''
print('MANAGER_RENDERER_SOURCE_SHA256: '+hashlib.sha256(source.encode()).hexdigest(), flush=True)
with tempfile.TemporaryDirectory(prefix='orbit43-manager-hud-') as directory:
    cpp=Path(directory)/'test.cpp'
    binary=Path(directory)/'test'
    cpp.write_text(prefix+body+suffix)
    subprocess.run(['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror',
                    '-DPS3_SP_LOADER_FIX28=1','-DPS3_SP_LOADER_FIX29=1',
                    *[f'-DPS3_GAME_ORBIT_FIX{i}=1' for i in range(30,44)],
                    '-I'+str(root/'include'),str(cpp),str(root/'src/math3d.cpp'),
                    str(root/'src/orbit_ui_fix30.cpp'),str(root/'src/library_hud_fix28.cpp'),
                    str(root/'src/case_style_fix41.cpp'),'-o',str(binary)],check=True,timeout=60)
    arguments=[]
    if len(sys.argv)==2:
        output=Path(sys.argv[1]).resolve();output.mkdir(parents=True,exist_ok=True);arguments=[str(output)]
    subprocess.run([str(binary),*arguments],check=True,timeout=30)
