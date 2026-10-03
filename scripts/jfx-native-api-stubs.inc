
#include <cassert>
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cmath>
#include <string>
#include <vector>
#include <unordered_map>
#define __PSL1GHT__ 1
#define PS3_SP_LOADER_FIX28 1
#define private public
#include "rsx_renderer_v10.h"
#undef private
#include "diagnostic_views_fix24.h"
#include "case_render_fix25.h"
#include "full_cover_case_fix26.h"
#include "jfx_case_fix29.h"
#include "library_pair_fix28.h"
using u8=std::uint8_t; using u16=std::uint16_t;
using u32=std::uint32_t; using s32=std::int32_t; using f32=float;
struct gcmContextData { u32* begin;u32* current;u32* end; };
struct rsxVertexProgram {};
struct rsxFragmentProgram {};
struct rsxProgramConst { int id,count; };
struct gcmSurface {
    int colorFormat,colorTarget,colorLocation[4],colorOffset[4],colorPitch[4];
    int depthFormat,depthLocation,depthOffset,depthPitch,type,antiAlias,width,height,x,y;
};
namespace RuntimeDiag { void log(const char*,...) {} }
namespace RsxPresentFix20 {
    void log_display(const char*) {}
    void log_buffers(const char*,u32*,int,int,u32) {}
}
static u32 commands[10000];
static gcmContextData ctx{commands,commands,commands+10000};
static std::vector<u32> pixels(1280*720);
static rsxVertexProgram vp;static rsxFragmentProgram fp;
static rsxProgramConst mvp_param{0,4},model_param{1,4},uv_param{2,1};
static rsxProgramConst color_param{3,1},texture_param{4,1},alpha_param{5,1};
static std::array<float,16> expected_mvp,expected_model,neighbor_mvp,neighbor_model,uploaded_mvp,uploaded_model;
static unsigned case_draws=6,bound_offset;
static unsigned version,loaded_version,draws,loads,uploads;
static bool texture_bound,wait_ok=true;
static int texture_mode;
static bool depth_test,depth_write,blend,cull;
static u32 front_face,cull_mode;
static bool expected_clockwise;
static int flip_rc;
static unsigned flip_calls,padding_on_clear;
static float pending_use,active_use,pending_alpha;
static std::array<float,4> pending_color,pending_uv;
static unsigned plastic_draws,logo_draws,paper_draws;static bool far_seen,near_seen;
static void emit(gcmContextData* c) { assert(c->current<c->end-4); c->current+=4; }
static void gcmResetFlipStatus() {}
template<class... A> static void rsxSetSurface(gcmContextData* c,A...) { emit(c); }
template<class... A> static void rsxSetViewport(gcmContextData* c,A...) { emit(c); }
template<class... A> static void rsxSetScissor(gcmContextData* c,A...) { emit(c); }
template<class... A> static void rsxSetColorMask(gcmContextData* c,A...) { emit(c); }
static void rsxSetDepthTestEnable(gcmContextData* c,u32 v) { depth_test=v;emit(c); }
template<class... A> static void rsxSetDepthFunc(gcmContextData* c,A...) { emit(c); }
static void rsxSetDepthWriteEnable(gcmContextData* c,u32 v) { depth_write=v;emit(c); }
static void rsxSetCullFaceEnable(gcmContextData* c,u32 v) { cull=v;emit(c); }
static void rsxSetFrontFace(gcmContextData* c,u32 v) { front_face=v;emit(c); }
static void rsxSetCullFace(gcmContextData* c,u32 v) { cull_mode=v;emit(c); }
template<class... A> static void rsxSetShadeModel(gcmContextData* c,A...) { emit(c); }
static void rsxSetBlendEnable(gcmContextData* c,u32 v) { blend=v;emit(c); }
template<class... A> static void rsxSetBlendFunc(gcmContextData* c,A...) { emit(c); }
template<class... A> static void rsxSetBlendEquation(gcmContextData* c,A...) { emit(c); }
template<class... A> static void rsxSetClearColor(gcmContextData* c,A...) { emit(c); }
template<class... A> static void rsxSetClearDepthStencil(gcmContextData* c,A...) { emit(c); }
template<class... A> static void rsxClearSurface(gcmContextData* c,A...) { emit(c);c->current+=padding_on_clear; }
template<class... A> static void rsxLoadVertexProgram(gcmContextData* c,A...) { emit(c); }
static void rsxTextureControl(gcmContextData* c,u8,bool enabled,u32,u32,u32) {
    texture_bound=enabled;emit(c);
}
template<class... A> static void rsxBindVertexArrayAttrib(gcmContextData* c,A...) { emit(c); }
static void rsxSetVertexProgramParameter(gcmContextData* c,const rsxVertexProgram*,const rsxProgramConst* p,const float* values){
 if(p->id==0) std::copy_n(values,16,uploaded_mvp.begin());
 if(p->id==1) std::copy_n(values,16,uploaded_model.begin());
 if(p->id==2) std::copy_n(values,4,pending_uv.begin());
 emit(c);
}
static void rsxSetFragmentProgramParameter(gcmContextData* c,const rsxFragmentProgram*,
                                         const rsxProgramConst* p,const float* values,u32,u32) {
    if(p->id==3) std::copy_n(values,4,pending_color.begin());
    if(p->id==4) pending_use=values[0];
    if(p->id==5) pending_alpha=values[0];
    ++version;++uploads;emit(c);
}
static void rsxLoadFragmentProgramLocation(gcmContextData* c,const rsxFragmentProgram*,u32,u32) {
    assert(uploads==3);uploads=0;
    loaded_version=version;active_use=pending_use;++loads;emit(c);
}
static void rsxDrawIndexArray(gcmContextData* c,u32,u32,u32 count,u32,u32){
 assert(count>0 && loaded_version==version && texture_bound==(active_use==1));
 const bool neighbor=uploaded_model==neighbor_model;
 assert(std::fabs(pending_alpha-(neighbor ? .76f : 1))<1e-6f);
 if(draws<case_draws){
  assert(depth_test && cull && front_face==GCM_FRONTFACE_CCW);
  assert(uploaded_mvp==(neighbor ? neighbor_mvp : expected_mvp));
  if(count==2642*3){
   assert(draws>=8 && blend && !depth_write && active_use==0);
   assert((pending_color==std::array<float,4>{{1,1,1,.14f}}));
   if(cull_mode==GCM_CULL_FRONT){far_seen=true;} else {assert(cull_mode==GCM_CULL_BACK && far_seen);near_seen=true;}
   ++plastic_draws;
  }else if(count==2925*3){assert(!blend && depth_write && active_use==0 && cull_mode==GCM_CULL_BACK);++logo_draws;}
  else{
   assert(!blend && depth_write && cull_mode==GCM_CULL_BACK);
   const bool expected_texture=texture_mode==1 || (texture_mode==2 && count==6 && pending_uv[0]>1.5f);
   assert(active_use==(expected_texture ? 1.0f : 0.0f));
   if(expected_texture) assert(bound_offset==(neighbor ? 20u : 10u));
   if(texture_mode==1) assert((pending_uv==std::array<float,4>{{1,1,0,0}}));
   ++paper_draws;
  }
 }else {assert(draws==case_draws && count==12 && !depth_test && !depth_write && blend && !cull);}
 ++draws;emit(c);
}
static s32 gcmSetFlip(gcmContextData* c,u8) { ++flip_calls;emit(c);return flip_rc; }
static void rsxFlushBuffer(gcmContextData*) {}
void RsxRendererV10::setup_texture_ps3(const GpuTextureStage1& t) {
    assert(t.uploaded);bound_offset=t.gpu_offset;if(draws==case_draws) assert(t.width==1024 && t.height==192);texture_bound=true;emit(&ctx);
}
bool RsxRendererV10::wait_for_flip_polling_ps3(const char*,unsigned) {
    if(!wait_ok) last_error_="simulated timeout";
    return wait_ok;
}
