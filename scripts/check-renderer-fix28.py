#!/usr/bin/env python3
"""Run the actual native draw_frame body with simulated SDK APIs.

Tests command ordering and values supplied to the SDK. This is not an RSX
emulator: GPU cache behavior, memory ordering and screen output need a PS3.
"""
import hashlib
from pathlib import Path
import re
import subprocess
import tempfile

root = Path(__file__).resolve().parent.parent
source = (root/'src/rsx_renderer_v10.cpp').read_text()

def function(signature):
    start = source.index(signature)
    pos = source.index('{',start)
    depth,end = 1,pos+1
    while depth:
        depth += (source[end]=='{')-(source[end]=='}')
        end += 1
    return source[start:end]

draw = function('bool RsxRendererV10::draw_frame_ps3(')
calibrate = function('unsigned RsxRendererV10::front_green_samples(')
hud = function('bool RsxRendererV10::draw_library_hud_ps3(')
helpers = '\n'.join(function(s) for s in (
    'V10UvTransform compute_v10_uv_transform(',
    'V10UvTransform compute_native_uv_transform(',
    'std::size_t native_packet_part(',
    'std::vector<std::size_t> native_submission_order(',
    'const GpuTextureStage1* RsxRendererV10::texture_for_game(',
    'static bool surface_uses_texture('))
constants = sorted(set(re.findall(r'\bGCM_[A-Z0-9_]+\b',draw+hud)))
prefix = r'''
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
static void rsxSetVertexProgramParameter(gcmContextData* c,const rsxVertexProgram*,
                                       const rsxProgramConst* p,const float* values) {
    if(p->id==0) { std::copy_n(values,16,uploaded_mvp.begin());assert(draws>=case_draws ? uploaded_mvp==mat4_identity().m : uploaded_mvp==expected_mvp || (case_draws==12 && uploaded_mvp==neighbor_mvp)); }
    if(p->id==1) { std::copy_n(values,16,uploaded_model.begin());assert(draws>=case_draws ? uploaded_model==mat4_identity().m : uploaded_model==expected_model || (case_draws==12 && uploaded_model==neighbor_model)); }
    if(p->id==2) {
        if((texture_mode!=1 && texture_mode<3 && draws==0) || (texture_mode==4 && draws==3)) {
            assert(std::fabs(values[0]-275.0f/130)<0.0001f && std::fabs(values[2]+145.0f/130)<0.0001f);
            assert(values[1]==1 && values[3]==0);
        } else if((texture_mode==1 && draws==1) || (texture_mode>=3 && (draws==1 || (texture_mode==3 && draws==4)))) {
            assert(values[0]==-1 && values[1]==1 && std::fabs(values[2]-130.0f/275)<0.0001f && values[3]==0);
        } else assert(values[0]==1 && values[1]==1 && values[2]==0 && values[3]==0);
    }
    emit(c);
}
static void rsxSetFragmentProgramParameter(gcmContextData* c,const rsxFragmentProgram*,
                                         const rsxProgramConst* p,const float* values,u32,u32) {
    if(p->id==4) pending_use=values[0];
    if(p->id==5) pending_alpha=values[0];
    ++version;++uploads;emit(c);
}
static void rsxLoadFragmentProgramLocation(gcmContextData* c,const rsxFragmentProgram*,u32,u32) {
    assert(uploads==3);uploads=0;
    loaded_version=version;active_use=pending_use;++loads;emit(c);
}
static void rsxDrawIndexArray(gcmContextData* c,u32,u32,u32 count,u32,u32) {
    assert(count>0 && loaded_version==version);
    assert(active_use==(draws>=case_draws || (texture_mode==1 && draws<3) || (texture_mode==2 && draws==0) || (texture_mode==3 && draws<6) || (texture_mode==4 && (draws<3 || draws==3)) ? 1.0f : 0.0f));
    assert(texture_bound==(active_use==1));
    const bool neighbor=case_draws==12 && draws<case_draws && uploaded_model==neighbor_model;
    assert(std::fabs(pending_alpha-(neighbor ? 0.76f : 1.0f))<0.0001f);
    if(case_draws==12 && draws<case_draws){
        assert(uploaded_mvp==(neighbor ? neighbor_mvp : expected_mvp));
        if(active_use==1) assert(bound_offset==(neighbor ? 20u : 10u));
    }
    if(draws<case_draws){
        assert(depth_test && cull && front_face==(expected_clockwise ? GCM_FRONTFACE_CW : GCM_FRONTFACE_CCW) && cull_mode==GCM_CULL_BACK);
        assert(blend==(draws>=case_draws/2) && depth_write==(draws<case_draws/2));
    }else{
        assert(draws==case_draws && count==12 && blend && !depth_test && !depth_write && !cull);
    }
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
'''
suffix = r'''
static void reset_commands() {
    ctx={commands,commands,commands+10000};
    draws=loads=version=loaded_version=uploads=0;
    flip_calls=padding_on_clear=0;
    texture_bound=false;wait_ok=true;flip_rc=0;cull=false;
}
int main() {
    RsxStage1 stage;stage.context_=&ctx;
    RsxRendererV10 renderer;renderer.stage1_=&stage;
    renderer.color_buffer_[0]=renderer.color_buffer_[1]=pixels.data();
    renderer.depth_buffer_=pixels.data();renderer.color_pitch_=5120;
    renderer.vertex_program_=&vp;renderer.fragment_program_=&fp;
    renderer.vp_mvp_=&mvp_param;renderer.vp_model_=&model_param;renderer.vp_uv_transform_=&uv_param;
    renderer.fp_base_color_=&color_param;renderer.fp_use_texture_=&texture_param;renderer.fp_alpha_=&alpha_param;
    const auto mesh=build_full_cover_case_fix26(5);
    V10FramePlan plan;
    const auto model=mat4_mul(mat4_translate(0,0,55),mat4_mul(mat4_rotate_x_deg(-5),mat4_rotate_y_deg(28)));
    const auto mvp=mat4_mul(mat4_mul(mat4_perspective(29,1280.0f/720,1,2000),
                                   mat4_look_at(0,2,425,0,0,0,0,1,0)),model);
    // Expected rows are constructed independently of mat4_shader_rows.
    for(int r=0;r<4;++r) for(int c=0;c<4;++c) {
        expected_mvp[r*4+c]=mvp.m[c*4+r];expected_model[r*4+c]=model.m[c*4+r];
    }
    for(const auto& part:mesh.parts) {
        V10DrawPacket p;p.game_index=0;p.surface=part.surface;p.mesh_textured=part.textured;p.model=model;p.mvp=mvp;
        plan.packets.push_back(p);
        RsxRendererV10::GpuMeshPart gp;
        gp.surface=part.surface;gp.vertices=gp.indices=pixels.data();
        gp.vertex_count=part.vertices.size();gp.index_count=part.indices.size();renderer.gpu_mesh_.push_back(gp);
    }
    reset_commands();texture_mode=0;
    assert(renderer.draw_frame_ps3(plan,mesh));
    assert(draws==6 && loads==6 && renderer.current_buffer_==1);
    assert(renderer.last_stats_.textured_draw_calls==0 && renderer.last_stats_.shell_draw_calls==6);
    puts("PASS: actual gray draw body uploads matrix rows and activates FP after all three constant updates on each draw");
    RsxRendererV10::GpuCoverRecord record;record.texture.uploaded=record.texture.full_cover=true;
    record.texture.gpu_offset=10;renderer.covers_[0]=record;
    reset_commands();texture_mode=1;
    assert(renderer.draw_frame_ps3(plan,mesh));
    assert(draws==6 && loads==6 && renderer.current_buffer_==0);
    assert(renderer.last_stats_.textured_draw_calls==3 && renderer.last_stats_.shell_draw_calls==3);
    puts("PASS: actual textured draw body draws opaque covers before translucent plastic, with depth writes only for covers and outward face culling");
    reset_commands();
    for(const auto& view:DiagnosticViewsFix24::Views){
        draws=loads=version=loaded_version=uploads=0;
        renderer.set_diagnostic_view(view.tag,view.sample_surface);
        const auto rotated_model=mat4_mul(mat4_translate(0,0,55),
            mat4_mul(mat4_mul(mat4_rotate_x_deg(view.pitch_deg),mat4_rotate_y_deg(view.yaw_deg)),mat4_scale(0.72f)));
        const auto rotated_mvp=mat4_mul(mat4_mul(mat4_perspective(29,1280.0f/720,1,2000),
            mat4_look_at(0,2,425,0,0,0,0,1,0)),rotated_model);
        for(int r=0;r<4;++r) for(int c=0;c<4;++c){
            expected_mvp[r*4+c]=rotated_mvp.m[c*4+r];expected_model[r*4+c]=rotated_model.m[c*4+r];
        }
        for(auto& packet:plan.packets){ packet.model=rotated_model;packet.mvp=rotated_mvp; }
        assert(renderer.draw_frame_ps3(plan,mesh));
        assert(draws==6 && loads==6);
    }
    assert(flip_calls==3 && renderer.current_buffer_==1);
    puts("PASS: three actual draw bodies with different matrices and diagnostic faces, one texture, cumulative command cursor and alternating buffers");
    renderer.last_plan_=plan;
    std::fill(pixels.begin(),pixels.end(),0xff11814e);
    assert(renderer.front_green_samples(mesh)==6);
    std::fill(pixels.begin(),pixels.end(),0xff2558a0);
    assert(renderer.front_green_samples(mesh)==0);
    renderer.set_front_face_clockwise(true);expected_clockwise=true;
    reset_commands();assert(renderer.draw_frame_ps3(plan,mesh));
    renderer.set_front_face_clockwise(false);expected_clockwise=false;
    puts("PASS: initial FRONT pixel verification accepts green, rejects blue and supports either cull convention");
    renderer.hud_texture_.uploaded=true;renderer.hud_texture_.width=1024;renderer.hud_texture_.height=192;
    renderer.hud_mesh_.vertices=renderer.hud_mesh_.indices=pixels.data();renderer.hud_mesh_.index_count=12;
    reset_commands();assert(renderer.draw_frame_ps3(plan,mesh));
    assert(draws==7 && loads==7 && renderer.last_stats_.hud_draw_calls==1 && flip_calls==1);
    // The next frame must restore the case depth/cull states after HUD disabled them.
    reset_commands();assert(renderer.draw_frame_ps3(plan,mesh));assert(draws==7);
    puts("PASS: actual HUD body draws both screen panels once, after the six case draws and before flip; identity matrices, FP reload, alpha blend and disabled depth/cull checked");
    renderer.hud_mesh_.indices=nullptr;reset_commands();
    assert(!renderer.draw_frame_ps3(plan,mesh) && draws==6 && flip_calls==0);
    renderer.hud_mesh_.indices=pixels.data();renderer.hud_texture_.uploaded=false;
    texture_mode=2;renderer.covers_[0].texture.full_cover=false;reset_commands();
    assert(renderer.draw_frame_ps3(plan,mesh) && draws==6 && renderer.last_stats_.textured_draw_calls==1);
    puts("PASS: front-only artwork uses the full image on front, without repeating it on spine/back; missing HUD geometry stops before flip");
    texture_mode=1;renderer.covers_[0].texture.full_cover=true;
    const auto single_plan=plan;
    const auto second_model=mat4_mul(mat4_translate(185,0,-70),mat4_mul(mat4_rotate_x_deg(-3),mat4_mul(mat4_rotate_y_deg(-36),mat4_scale(0.80f))));
    const auto second_mvp=mat4_mul(mat4_mul(mat4_perspective(29,1280.0f/720,1,2000),mat4_look_at(0,2,425,0,0,0,0,1,0)),second_model);
    for(int r=0;r<4;++r) for(int c=0;c<4;++c){neighbor_model[r*4+c]=second_model.m[c*4+r];neighbor_mvp[r*4+c]=second_mvp.m[c*4+r];}
    for(auto packet:single_plan.packets){packet.game_index=1;packet.model=second_model;packet.mvp=second_mvp;packet.alpha=0.76f;plan.packets.push_back(packet);}
    renderer.covers_[1]=record;renderer.covers_[1].texture.gpu_offset=20;
    case_draws=12;texture_mode=3;renderer.hud_texture_.uploaded=true;
    reset_commands();assert(renderer.draw_frame_ps3(plan,mesh));
    assert(draws==13 && loads==13 && flip_calls==1 && renderer.last_stats_.textured_draw_calls==6 && renderer.last_stats_.hud_draw_calls==1);
    texture_mode=4;renderer.covers_[1].texture.full_cover=false;
    reset_commands();assert(renderer.draw_frame_ps3(plan,mesh));assert(draws==13 && renderer.last_stats_.textured_draw_calls==4);
    puts("PASS: actual native body draws two cases with independent matrices and texture offsets, handles a front-only neighbor, and submits HUD once after twelve case draws");
    renderer.hud_texture_.uploaded=false;plan=single_plan;case_draws=6;texture_mode=1;
    renderer.current_buffer_=0;
    reset_commands();ctx.end=commands+100;
    assert(!renderer.draw_frame_ps3(plan,mesh) && draws==0);
    reset_commands();plan.packets[0].surface=V14Surface::CoverFront;
    assert(!renderer.draw_frame_ps3(plan,mesh) && draws==0);
    plan.packets[0].surface=V14Surface::ShellFront;
    reset_commands();flip_rc=-7;
    assert(!renderer.draw_frame_ps3(plan,mesh) && renderer.current_buffer_==0);
    reset_commands();wait_ok=false;
    assert(!renderer.draw_frame_ps3(plan,mesh) && renderer.current_buffer_==0);
    puts("PASS: command-room, packet-order, flip error and polling failure stop the actual draw body");
    reset_commands();padding_on_clear=2800;
    assert(!renderer.draw_frame_ps3(plan,mesh) && flip_calls==0);
    assert(renderer.last_error_=="FIX28 frame exceeds its bounded command budget");
    puts("PASS: over-budget frame stops before flip/flush");
    puts("SIMULATION ONLY: not a GPU/PS3 test; stubs do not model FIFO word counts, shader caches or texture sampling");
}
'''
defines = ''.join(f'constexpr u32 {n}={0 if n=="GCM_FALSE" else 1 if n=="GCM_TRUE" else i+2};\n'
                  for i,n in enumerate(constants))
print('SOURCE_SHA256: '+hashlib.sha256((root/'src/rsx_renderer_v10.cpp').read_bytes()).hexdigest(),flush=True)
with tempfile.TemporaryDirectory(prefix='fix28-renderer-') as directory:
    folder=Path(directory)
    variants = {
        'corrected': (draw,hud),
        'old-matrix-upload': (draw.replace('mvp_rows.data()','packet.mvp.m.data()').replace('model_rows.data()','packet.model.m.data()'),hud),
        'missing-culling': (draw.replace('rsxSetCullFaceEnable(ctx,GCM_TRUE);','rsxSetCullFaceEnable(ctx,GCM_FALSE);'),hud),
        'missing-fp-reload': (draw.replace('rsxLoadFragmentProgramLocation(ctx,fp,fragment_offset_,GCM_LOCATION_RSX);',''),hud),
        'hud-missing-fp-reload': (draw,hud.replace('rsxLoadFragmentProgramLocation(ctx,fp,fragment_offset_,GCM_LOCATION_RSX);','')),
        'wrong-neighbor-texture': (draw.replace('texture_for_game(packet.game_index)','texture_for_game(0)'),hud),
        'hud-depth-write': (draw,hud.replace('rsxSetDepthWriteEnable(ctx,GCM_FALSE);','rsxSetDepthWriteEnable(ctx,GCM_TRUE);')),
    }
    for name,(body,hud_body) in variants.items():
        cpp=folder/(name+'.cpp');binary=folder/name
        cpp.write_text(prefix.replace('struct gcmContextData {',defines+'struct gcmContextData {')+helpers+'\n'+calibrate+'\n'+hud_body+'\n'+body+'\n'+suffix)
        subprocess.run(['g++','-std=c++17','-O2','-Wall','-Wextra','-I'+str(root/'include'),
                        str(cpp),str(root/'src/jfx_case_fix29.cpp'),str(root/'src/math3d.cpp'),str(root/'src/v14_case_mesh.cpp'),str(root/'src/case_render_fix25.cpp'),str(root/'src/full_cover_case_fix26.cpp'),'-o',str(binary)],
                       check=True,timeout=30,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
        result=subprocess.run([str(binary)],capture_output=True,text=True,timeout=15)
        if name=='corrected':
            print(result.stdout,end='',flush=True)
            if result.returncode: print(result.stderr)
            assert result.returncode==0
        else:
            assert result.returncode!=0, f'{name} regression was not detected'
            print(f'PASS: regression variant {name} rejected',flush=True)
