#include "rsx_renderer_v10.h"
#ifdef PS3_GAME_ORBIT_FIX36
#include "runtime_diag.h"
#include <algorithm>
#include <cstring>
#ifdef __PSL1GHT__
#include <rsx/rsx.h>
#endif
bool RsxRendererV10::prepare_animated_background(){
    if(animated_base_texture_.uploaded && wave_texture_.uploaded)return true;
    if(!stage1_->prepare_overlay(OrbitBackgroundFix36::gradient(),animated_base_texture_) || !stage1_->prepare_overlay(OrbitBackgroundFix36::ribbon_texture(),wave_texture_)){last_error_=stage1_->last_error();return false;}
    wave_cpu_mesh_=OrbitBackgroundFix36::build(wave_phase_);
#ifdef __PSL1GHT__
    const auto vb=wave_cpu_mesh_.vertices.size()*sizeof(V14Vertex),ib=wave_cpu_mesh_.indices.size()*sizeof(std::uint16_t);
    wave_mesh_.vertices=rsxMemalign(128,u32(vb));wave_mesh_.indices=rsxMemalign(128,u32(ib));
    if(!wave_mesh_.vertices || !wave_mesh_.indices || rsxAddressToOffset(wave_mesh_.vertices,&wave_mesh_.vertex_offset)!=0 || rsxAddressToOffset(wave_mesh_.indices,&wave_mesh_.index_offset)!=0){last_error_="Wave allocation/mapping failed";return false;}
    wave_mesh_.vertex_count=u32(wave_cpu_mesh_.vertices.size());wave_mesh_.index_count=u32(wave_cpu_mesh_.indices.size());
    std::memcpy(wave_mesh_.vertices,wave_cpu_mesh_.vertices.data(),vb);std::memcpy(wave_mesh_.indices,wave_cpu_mesh_.indices.data(),ib);__asm__ volatile("sync" ::: "memory");
#endif
    RuntimeDiag::log("BACKGROUND 1.3.3: 3 ribbons, %u vertices; frozen shaders; no frame bitmap decode",unsigned(wave_cpu_mesh_.vertices.size()));return true;
}
bool RsxRendererV10::update_orbit_background(bool animated,float dt){
    animated_background_=animated;if(!ready_ || !animated_base_texture_.uploaded || !wave_texture_.uploaded)return false;
    if(!animated)return true;
    wave_phase_+=std::clamp(dt,0.f,.05f);if(wave_phase_>=314.159265f)wave_phase_-=314.159265f;
    OrbitBackgroundFix36::update(wave_cpu_mesh_,wave_phase_);
#ifdef __PSL1GHT__
    if(!wave_mesh_.vertices)return false;
    // Called only between acknowledged frames, before begin_frame().
    std::memcpy(wave_mesh_.vertices,wave_cpu_mesh_.vertices.data(),wave_cpu_mesh_.vertices.size()*sizeof(V14Vertex));__asm__ volatile("sync" ::: "memory");
#endif
    return true;
}
#ifdef __PSL1GHT__
bool RsxRendererV10::draw_orbit_waves_ps3(){
    if(!animated_background_)return true;
    auto* ctx=static_cast<gcmContextData*>(stage1_->native_context());
    const auto current=reinterpret_cast<std::uintptr_t>(static_cast<void*>(ctx->current)),end=reinterpret_cast<std::uintptr_t>(static_cast<void*>(ctx->end));
    if(current>end || end-current<8192u || !wave_mesh_.vertices || !wave_mesh_.indices){last_error_="Wave command room unavailable";return false;}
    auto* fp=static_cast<const rsxFragmentProgram*>(fragment_program_);rsxSetBlendEnable(ctx,GCM_TRUE);setup_texture_ps3(wave_texture_);
    const u8 stride=sizeof(V14Vertex);
    rsxBindVertexArrayAttrib(ctx,attr_position_,0,wave_mesh_.vertex_offset+offsetof(V14Vertex,x),stride,3,GCM_VERTEX_DATA_TYPE_F32,GCM_LOCATION_RSX);
    rsxBindVertexArrayAttrib(ctx,attr_normal_,0,wave_mesh_.vertex_offset+offsetof(V14Vertex,nx),stride,3,GCM_VERTEX_DATA_TYPE_F32,GCM_LOCATION_RSX);
    rsxBindVertexArrayAttrib(ctx,attr_uv_,0,wave_mesh_.vertex_offset+offsetof(V14Vertex,u),stride,2,GCM_VERTEX_DATA_TYPE_F32,GCM_LOCATION_RSX);
    for(int layer=2;layer>=0;--layer){
        const float alpha[4]={layer==0?1.f:layer==1?.47f:.22f,0,0,0};
        rsxSetFragmentProgramParameter(ctx,fp,static_cast<const rsxProgramConst*>(fp_alpha_),alpha,fragment_offset_,GCM_LOCATION_RSX);rsxLoadFragmentProgramLocation(ctx,fp,fragment_offset_,GCM_LOCATION_RSX);
        rsxDrawIndexArray(ctx,GCM_TYPE_TRIANGLES,wave_mesh_.index_offset+u32(layer)*OrbitBackgroundFix36::IndicesPerRibbon*sizeof(std::uint16_t),OrbitBackgroundFix36::IndicesPerRibbon,GCM_INDEX_TYPE_16B,GCM_LOCATION_RSX);++last_stats_.wave_draw_calls;
    }
    rsxSetBlendEnable(ctx,GCM_FALSE);return true;
}
#endif
#endif
