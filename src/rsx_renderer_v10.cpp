#include "rsx_renderer_v10.h"
#include "performance_fix35.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <unordered_set>
#include "runtime_diag.h"
#include "rsx_present_fix20.h"
#include "case_render_fix25.h"
#include "full_cover_case_fix26.h"
#include "library_pair_fix28.h"
#ifdef PS3_GAME_ORBIT_FIX30
#include "orbit_ui_fix30.h"
#include "project_identity.h"
#endif
#include "jfx_case_fix29.h"
#ifdef PS3_GAME_ORBIT_FIX32
#include "case_animation_fix32.h"
#endif
#ifdef PS3_GAME_ORBIT_FIX37
#include "case_model_fix37.h"
#endif

#ifdef __PSL1GHT__
#include <cstddef>
#include <ppu-types.h>
#include <rsx/rsx.h>
#include "video_compat_fix20.h"
#include "ps3_lifecycle.h"
#include <lv2/systime.h>
#include <sysutil/sysutil.h>
#include <unistd.h>

extern "C" {
extern const u8 v14_case_vpo[];
extern const u32 v14_case_vpo_size;
extern const u8 v14_case_fpo[];
extern const u32 v14_case_fpo_size;
}
#endif

V10FramePlan build_v10_frame_plan(const std::vector<CasePose>& poses,
                                   const V14CaseMesh& mesh,
                                   int sw,int sh) {
    V10FramePlan out; out.screen_width=sw; out.screen_height=sh;
    const float aspect=sh>0 ? float(sw)/float(sh) : 16.0f/9.0f;
    const Mat4 proj=mat4_perspective(29.0f,aspect,1.0f,2000.0f);
    const Mat4 view=mat4_look_at(0.0f,2.0f,425.0f, 0.0f,0.0f,0.0f, 0.0f,1.0f,0.0f);
    const Mat4 vp=mat4_mul(proj,view);

    for(const auto& pose:poses){
        Mat4 model=mat4_identity();
        model=mat4_mul(model,mat4_translate(pose.x,pose.y,pose.z));
        model=mat4_mul(model,mat4_rotate_x_deg(pose.pitch_deg));
        model=mat4_mul(model,mat4_rotate_y_deg(pose.yaw_deg));
        model=mat4_mul(model,mat4_scale(pose.scale));
#ifndef PS3_GAME_ORBIT_FIX32
        const Mat4 mvp=mat4_mul(vp,model);
#endif
        for(std::size_t part_i=0;part_i<mesh.parts.size();++part_i){
            const auto& part=mesh.parts[part_i];
#ifdef PS3_GAME_ORBIT_FIX32
            const bool opened=pose.selected && pose.inspection_phase>.20f;
            if((part.joint==V14MeshPart::Joint::Closed)==opened) continue;
#ifdef PS3_GAME_ORBIT_FIX37
            const auto joint=opened ? CaseModelFix37::joint_transform(part.joint,pose.inspection_phase) : mat4_identity();
#else
            const auto joint=opened ? CaseAnimationFix32::joint_transform(part.joint,pose.inspection_phase) : mat4_identity();
#endif
            const auto part_model=mat4_mul(model,joint);
#endif
            V10DrawPacket p{};
            p.game_index=pose.game_index;
            p.relative_slot=pose.relative_slot;
            p.surface=part.surface;
#ifdef PS3_GAME_ORBIT_FIX32
            p.model=part_model;p.mvp=mat4_mul(vp,part_model);
#else
            p.model=model;p.mvp=mvp;
#endif
            p.alpha=pose.alpha;
            p.visibility=pose.visibility;
            p.mesh_textured=part.textured;
            p.selected=pose.selected;
            p.mesh_part=part_i;
            if(mesh.jfx && (part.material==V14MeshPart::Material::ClearPlastic
#ifdef PS3_GAME_ORBIT_FIX32
                || part.material==V14MeshPart::Material::DiscGlass
#endif
            )){
                p.plastic_pass=1;out.packets.push_back(p);p.plastic_pass=2;
            }
            out.packets.push_back(p);
        }
    }
    return out;
}

#ifdef PS3_GAME_ORBIT_FIX37
bool RsxRendererV10::sync_case_animation(V14CaseMesh& mesh,float phase){
    if(!std::isfinite(phase) || phase<0 || phase>1)return false;
    if(std::fabs(phase-geometry_phase_)<.00001f)return true;
    CaseModelFix37::deform_spine(mesh,phase);
#ifdef __PSL1GHT__
    if(gpu_mesh_.size()!=mesh.parts.size())return false;
    for(std::size_t i=0;i<mesh.parts.size();++i){
        const auto& part=mesh.parts[i];if(part.flex_rest.empty())continue;
        if(!gpu_mesh_[i].vertices || gpu_mesh_[i].vertex_count!=part.vertices.size())return false;
        std::memcpy(gpu_mesh_[i].vertices,part.vertices.data(),part.vertices.size()*sizeof(V14Vertex));
    }
    __asm__ volatile("sync" ::: "memory");
#endif
    geometry_phase_=phase;return true;
}
#endif

std::size_t native_packet_part(const V10DrawPacket& packet,std::size_t i,const V14CaseMesh& mesh){
    return packet.mesh_part==static_cast<std::size_t>(-1) ? i%mesh.parts.size() : packet.mesh_part;
}

V10UvTransform compute_native_uv_transform(const V14CaseMesh& mesh,V14Surface surface,bool full){
    if(!mesh.jfx) return compute_v10_uv_transform(surface,full);
    V10UvTransform t;
    // The original OBJ already has a readable back. Do not apply the legacy
    // back reflection a second time. Native V was converted to top-origin.
    if(!full && surface==V14Surface::CoverFront){
        t.scale_u=1/(1-JfxCaseFix29::FrontBegin);
        t.bias_u=-JfxCaseFix29::FrontBegin*t.scale_u;
    }
    return t;
}

std::vector<std::size_t> native_submission_order(const V10FramePlan& plan,const V14CaseMesh& mesh){
    if(!mesh.jfx) return CaseRenderFix25::submission_order(plan,mesh);
    std::vector<std::size_t> order;
    for(std::size_t i=0;i<plan.packets.size();++i) order.push_back(i);
    std::stable_sort(order.begin(),order.end(),[&](std::size_t a,std::size_t b){
        const auto& pa=plan.packets[a];const auto& pb=plan.packets[b];
#ifdef PS3_GAME_ORBIT_FIX31
        const bool fade_a=pa.visibility<.99999f,fade_b=pb.visibility<.99999f;
        const bool transparent_a=pa.plastic_pass!=0 || fade_a;
        const bool transparent_b=pb.plastic_pass!=0 || fade_b;
        if(transparent_a!=transparent_b) return !transparent_a;
        if(!transparent_a) return false;
        const float da=pa.mvp.m[14]/pa.mvp.m[15],db=pb.mvp.m[14]/pb.mvp.m[15];
        if(std::fabs(da-db)>.000001f) return da>db;
        return pa.plastic_pass<pb.plastic_pass;
#else
        const bool ta=pa.plastic_pass!=0,tb=pb.plastic_pass!=0;
        if(ta!=tb) return !ta; // Paper AND the solid 3D logo first.
        if(!ta) return false;
        if(pa.game_index==pb.game_index) return pa.plastic_pass<pb.plastic_pass;
        const auto depth=[](const V10DrawPacket& p){return p.mvp.m[14]/p.mvp.m[15];};
        return depth(pa)>depth(pb);
#endif
    });
    return order;
}


V10UvTransform compute_v10_uv_transform(V14Surface surface,bool full_cover_texture){
    V10UvTransform t{};
    if(full_cover_texture && surface==V14Surface::CoverBack){
        // The frozen back plane has increasing U along model +X. Viewed from
        // behind, +X runs screen-right to screen-left. Reflect only this atlas
        // strip at upload time so the back label reads normally after yaw=180.
        t.scale_u=-1.0f;
        t.bias_u=FullCoverLayout::Back.u0+FullCoverLayout::Back.u1;
        return t;
    }
    if(full_cover_texture || surface!=V14Surface::CoverFront) return t;
    const float u0=FullCoverLayout::Front.u0;
    const float u1=FullCoverLayout::Front.u1;
    const float span=u1-u0;
    if(span>0.00001f){
        t.scale_u=1.0f/span;
        t.bias_u=-u0/span;
    }
    return t;
}

V10SubmissionStats summarize_v10_submission(const V10FramePlan& plan,
                                             const V14CaseMesh& mesh) {
    V10SubmissionStats s{};
    s.draw_calls=plan.packets.size();
    for(const auto& p:plan.packets){
        if(p.mesh_textured) ++s.textured_draw_calls;
        else ++s.shell_draw_calls;
    }
    std::unordered_set<int> cases;
    for(const auto& p:plan.packets) cases.insert(p.game_index);
    s.visible_cases=cases.size();
    for(const auto& p:mesh.parts){
        s.uploaded_geometry_bytes += p.vertices.size()*sizeof(V14Vertex);
        s.uploaded_geometry_bytes += p.indices.size()*sizeof(std::uint16_t);
    }
    return s;
}

bool RsxRendererV10::init(RsxStage1& stage1,const V14CaseMesh& mesh,bool present_boot_visuals){
    shutdown();
    last_error_.clear();
    RuntimeDiag::log("RENDER 00: init begin; present=%s", present_boot_visuals ? "YES" : "NO");
    if(!stage1.initialized()){
        last_error_="RSX Stage1 must be initialized first";
        RuntimeDiag::log("RENDER 01: stage1 unavailable");
        return false;
    }
    stage1_=&stage1;
#ifndef __PSL1GHT__
    (void)mesh;
    (void)present_boot_visuals;
#endif
#ifdef __PSL1GHT__
    RuntimeDiag::log("RENDER 10: display init begin");
    if(!init_display_ps3()) {
        RuntimeDiag::log("RENDER 19: display init FAILED - %s", last_error_.c_str());
        return false;
    }
    RuntimeDiag::log("RENDER 19: display init OK");
    boot_visual_stage_=BootVisualStage::DisplayReady;
    if(present_boot_visuals){
        RuntimeDiag::log("RENDER 20: first boot visual begin");
        if(!present_boot_visual_ps3(boot_visual_stage_)){
            last_error_="DisplayReady visual presentation failed";
            RuntimeDiag::log("RENDER 20: first boot visual FAILED");
            return false;
        }
        RuntimeDiag::log("RENDER 20: first boot visual OK");
    } else {
        RuntimeDiag::log("RENDER 20: boot visual deliberately SKIPPED");
    }

    RuntimeDiag::log("RENDER 30: shader init begin");
    if(!init_shaders_ps3()) {
        boot_visual_stage_=BootVisualStage::FatalShader;
        RuntimeDiag::log("RENDER 31: shader init FAILED - %s", last_error_.c_str());
        if(present_boot_visuals) present_boot_visual_ps3(boot_visual_stage_);
        return false;
    }
    RuntimeDiag::log("RENDER 31: shader init OK");
    boot_visual_stage_=BootVisualStage::ShaderReady;
    if(present_boot_visuals) present_boot_visual_ps3(boot_visual_stage_);

    RuntimeDiag::log("RENDER 40: geometry upload begin");
    if(!upload_geometry_ps3(mesh)) {
        boot_visual_stage_=BootVisualStage::FatalGeometry;
        RuntimeDiag::log("RENDER 41: geometry upload FAILED - %s", last_error_.c_str());
        if(present_boot_visuals) present_boot_visual_ps3(boot_visual_stage_);
        return false;
    }
    RuntimeDiag::log("RENDER 41: geometry upload OK; parts=%u", (unsigned)gpu_mesh_.size());
    boot_visual_stage_=BootVisualStage::GeometryReady;
    if(present_boot_visuals) present_boot_visual_ps3(boot_visual_stage_);
#else
    (void)mesh;
#endif
#ifdef PS3_GAME_ORBIT_FIX32
    if(!prepare_disc_textures()) return false;
#endif
    ready_=true;
    boot_visual_stage_=BootVisualStage::Ready;
#ifdef __PSL1GHT__
    if(present_boot_visuals) present_boot_visual_ps3(boot_visual_stage_);
    else RuntimeDiag::log("RENDER 50: Ready; all flip/draw commands SKIPPED");
#endif
    RuntimeDiag::log("RENDER 59: init complete");
    return true;
}

void RsxRendererV10::release_cover_record(GpuCoverRecord& rec){
#ifdef PS3_GAME_ORBIT_FIX32
    const auto bytes=gpu_texture_storage_bytes(rec.texture);
    gpu_cache_bytes_-=std::min(gpu_cache_bytes_,bytes);
#endif
    if(stage1_ && rec.texture.gpu_ptr) stage1_->release_cover(rec.texture);
    rec={};
}

void RsxRendererV10::clear_cover_textures(){
    for(auto& kv:covers_) release_cover_record(kv.second);
    covers_.clear();
#ifdef PS3_GAME_ORBIT_FIX32
    failed_gpu_covers_.clear();gpu_cache_bytes_=0;gpu_cache_clock_=0;
#ifdef PS3_GAME_ORBIT_FIX34
    clear_inspection_cache(inside_cache_);clear_inspection_cache(disc_cache_);
#elif defined(PS3_GAME_ORBIT_FIX33)
    release_inspection_art(inside_art_);release_inspection_art(disc_art_);
#endif
#endif
}
void RsxRendererV10::shutdown(){
    clear_cover_textures();
    if(stage1_) stage1_->release_cover(hud_texture_);
    if(stage1_) stage1_->release_cover(background_texture_);
#ifdef PS3_GAME_ORBIT_FIX36
    if(stage1_){stage1_->release_cover(animated_base_texture_);stage1_->release_cover(wave_texture_);}
    wave_cpu_mesh_={};wave_phase_=6;
#endif
    hud_lines_={};
#ifdef PS3_GAME_ORBIT_FIX35
    hud_cache_.clear();hud_menu_={};
#endif
#ifdef PS3_GAME_ORBIT_FIX32
    if(stage1_){stage1_->release_cover(disc_label_texture_);stage1_->release_cover(disc_back_texture_);}
    hud_rows_.clear();hud_active_row_=-1;
#endif
#ifdef __PSL1GHT__
    if(hud_mesh_.vertices) rsxFree(hud_mesh_.vertices);
    if(hud_mesh_.indices) rsxFree(hud_mesh_.indices);
    hud_mesh_={};
    if(background_mesh_.vertices) rsxFree(background_mesh_.vertices);
    if(background_mesh_.indices) rsxFree(background_mesh_.indices);
    background_mesh_={};
#ifdef PS3_GAME_ORBIT_FIX36
    if(wave_mesh_.vertices)rsxFree(wave_mesh_.vertices);
    if(wave_mesh_.indices)rsxFree(wave_mesh_.indices);
    wave_mesh_={};
#endif
    release_geometry_ps3();
    if(fragment_ucode_) rsxFree(fragment_ucode_);
    fragment_ucode_=nullptr;
    for(void*& p:color_buffer_){ if(p) rsxFree(p); p=nullptr; }
    if(depth_buffer_) rsxFree(depth_buffer_);
    depth_buffer_=nullptr;
#endif
    ready_=false;
    boot_visual_stage_=BootVisualStage::None;
    stage1_=nullptr;
    last_plan_={};
    last_stats_={};
}

const GpuTextureStage1* RsxRendererV10::texture_for_game(int game_index) const{
    auto it=covers_.find(game_index);
    return it==covers_.end()?nullptr:&it->second.texture;
}

bool RsxRendererV10::sync_visible_covers(const CoverflowState& state,CoverCache& cache,int radius){
    last_error_.clear();
    if(!ready_ || !stage1_){ last_error_="Renderer not initialized"; return false; }

#ifdef PS3_SP_LOADER_FIX28
    const auto poses=LibraryPairFix28::poses(state,radius);
#else
    const auto poses=build_coverflow_render_plan(state,radius);
#endif
    std::unordered_set<int> wanted;
#ifdef PS3_GAME_ORBIT_FIX32
    for(const auto& pose:poses) wanted.insert(pose.game_index);
#ifdef PS3_GAME_ORBIT_FIX33
    trim_cover_cache(wanted);
#endif
    for(const auto& pose:poses) if(pose.game_index>=0 && pose.game_index<int(state.games.size()))
        load_gpu_cover(pose.game_index,state.games[std::size_t(pose.game_index)],cache,wanted);
    return true;
#else
    for(const auto& pose:poses){
        const int gi=pose.game_index;
        if(gi<0 || gi>=(int)state.games.size()) continue;
        wanted.insert(gi);
        const GameEntry& g=state.games[(std::size_t)gi];
        auto it=covers_.find(gi);
        if(g.cover_path.empty()){
            if(it!=covers_.end()){
                release_cover_record(it->second);
                covers_.erase(it);
            }
            continue;
        }
        if(it!=covers_.end() && it->second.path==g.cover_path && it->second.kind==g.cover_kind &&
           it->second.orientation==g.cover_orientation && it->second.texture.uploaded) continue;

        const CoverImage* image=cache.get_or_load(g);
        if(!image || !image->valid()){
            if(it!=covers_.end()){release_cover_record(it->second);covers_.erase(it);}
            continue;
        }

        GpuCoverRecord fresh{};
        fresh.path=g.cover_path;
        fresh.kind=g.cover_kind;fresh.orientation=g.cover_orientation;
        if(!stage1_->prepare_cover(*image,fresh.texture,g.cover_orientation)){
            RuntimeDiag::log("COVER FALLBACK: game=%d path=%s error=%s",gi,g.cover_path.c_str(),stage1_->last_error().c_str());
            if(it!=covers_.end()){release_cover_record(it->second);covers_.erase(it);}
            continue;
        }
        if(it!=covers_.end()){
            release_cover_record(it->second);
            it->second=std::move(fresh);
        }else{
            covers_.emplace(gi,std::move(fresh));
        }
    }

    for(auto it=covers_.begin();it!=covers_.end();){
        if(wanted.find(it->first)==wanted.end()){
            release_cover_record(it->second);
            it=covers_.erase(it);
        }else ++it;
    }
    return true;
#endif
}

void RsxRendererV10::show_runtime_failure(){
    boot_visual_stage_=BootVisualStage::FatalRuntime;
#ifdef __PSL1GHT__
    present_boot_visual_ps3(boot_visual_stage_);
#endif
}

bool RsxRendererV10::render(const CoverflowState& state,const V14CaseMesh& mesh,int radius,float selected_scale){
    last_error_.clear();
    if(!ready_){ last_error_="Renderer not initialized"; return false; }
#ifdef PS3_SP_LOADER_FIX28
    auto poses=LibraryPairFix28::poses(state,radius);
#else
    auto poses=build_coverflow_render_plan(state,radius);
#endif
#ifdef PS3_GAME_ORBIT_FIX31
    (void)selected_scale; // Animated poses already include the current zoom.
#else
    for(auto& pose:poses) if(pose.selected) pose.scale=selected_scale;
#endif
#ifdef __PSL1GHT__
    last_plan_=build_v10_frame_plan(poses,mesh,width_,height_);
#else
    last_plan_=build_v10_frame_plan(poses,mesh,1280,720);
#endif
    last_stats_=summarize_v10_submission(last_plan_,mesh);
    last_stats_.hud_draw_calls=hud_texture_.uploaded ? 1 : 0;
    last_stats_.background_draw_calls=background_texture_.uploaded ? 1 : 0;
#ifdef PS3_GAME_ORBIT_FIX36
    last_stats_.wave_draw_calls=animated_background_ && wave_texture_.uploaded ? OrbitBackgroundFix36::Ribbons : 0;
#endif
#ifdef __PSL1GHT__
    return draw_frame_ps3(last_plan_,mesh);
#else
    return true;
#endif
}

bool RsxRendererV10::set_library_hud(const LibraryHudLinesFix28& lines
#ifdef PS3_GAME_ORBIT_FIX32
    ,const CoverflowState* state
#endif
){
    if(!ready_ || !stage1_){last_error_="Renderer not ready for HUD";return false;}
#ifdef PS3_GAME_ORBIT_FIX32
    std::vector<std::string> rows;int active=-1;
    if(state && state->layout==OrbitLayout::List && !state->inspection_target && state->inspection_phase==0) {
        const int n=int(state->visible.size());const int begin=std::max(0,std::min(state->selected-4,n-9));
        for(int vi=begin;vi<std::min(n,begin+9);++vi) {
            const auto& g=state->games[std::size_t(state->visible[std::size_t(vi)])];
            rows.push_back(std::to_string(vi+1)+"   "+g.title+(g.favorite ? "  *" : ""));
        }
        if(n) active=state->selected-begin;
    }
#ifdef PS3_GAME_ORBIT_FIX35
    const auto menu=state ? state->menu : GameMenuStateFix35{};
    if(hud_texture_.uploaded && hud_lines_==lines && hud_rows_==rows && hud_active_row_==active && hud_menu_==menu)return true;
    OrbitPerformanceFix35::Scope timer(OrbitPerformanceFix35::Kind::Interface);
#else
    if(hud_texture_.uploaded && hud_lines_==lines && hud_rows_==rows && hud_active_row_==active) return true;
#endif
#else
    if(hud_texture_.uploaded && hud_lines_==lines) return true;
#endif
#ifdef __PSL1GHT__
    if(!hud_mesh_.vertices){
        const auto vertices=library_hud_vertices_fix28();
        hud_mesh_.vertices=rsxMemalign(128,sizeof(vertices));
        hud_mesh_.indices=rsxMemalign(128,sizeof(LibraryHudIndicesFix28));
        if(!hud_mesh_.vertices || !hud_mesh_.indices ||
           rsxAddressToOffset(hud_mesh_.vertices,&hud_mesh_.vertex_offset)!=0 ||
           rsxAddressToOffset(hud_mesh_.indices,&hud_mesh_.index_offset)!=0){
            if(hud_mesh_.vertices) rsxFree(hud_mesh_.vertices);
            if(hud_mesh_.indices) rsxFree(hud_mesh_.indices);
            hud_mesh_={};last_error_="HUD geometry allocation/mapping failed";return false;
        }
        std::memcpy(hud_mesh_.vertices,vertices.data(),sizeof(vertices));
        std::memcpy(hud_mesh_.indices,LibraryHudIndicesFix28.data(),sizeof(LibraryHudIndicesFix28));
        hud_mesh_.vertex_count=vertices.size();hud_mesh_.index_count=LibraryHudIndicesFix28.size();
        __asm__ volatile("sync" ::: "memory");
    }
#endif
#ifdef PS3_GAME_ORBIT_FIX32
#ifdef PS3_GAME_ORBIT_FIX35
    const auto regions=hud_cache_.update(lines,rows,active,menu);
    if(!stage1_->update_overlay_regions(hud_cache_.image(),hud_texture_,regions)) {
#else
    if(!stage1_->update_overlay(OrbitUiFix30::hud(lines,rows,active),hud_texture_)) {
#endif
        last_error_=stage1_->last_error();return false;
    }
    hud_lines_=lines;hud_rows_=std::move(rows);hud_active_row_=active;
#ifdef PS3_GAME_ORBIT_FIX35
    hud_menu_=menu;
#endif
#else
    GpuTextureStage1 fresh;
    if(!stage1_->prepare_overlay(rasterize_library_hud_fix28(lines),fresh)){
        last_error_=stage1_->last_error();return false;
    }
    stage1_->release_cover(hud_texture_);hud_texture_=fresh;hud_lines_=lines;
#endif
    return true;
}

bool RsxRendererV10::prepare_orbit_background(){
#ifdef PS3_GAME_ORBIT_FIX30
    if(!ready_ || !stage1_){last_error_="Renderer not ready for Orbit background";return false;}
#ifdef __PSL1GHT__
    if(!background_mesh_.vertices){
        const auto vertices=OrbitUiFix30::background_vertices();
        background_mesh_.vertices=rsxMemalign(128,sizeof(vertices));
        background_mesh_.indices=rsxMemalign(128,sizeof(OrbitUiFix30::BackgroundIndices));
        if(!background_mesh_.vertices || !background_mesh_.indices ||
           rsxAddressToOffset(background_mesh_.vertices,&background_mesh_.vertex_offset)!=0 ||
           rsxAddressToOffset(background_mesh_.indices,&background_mesh_.index_offset)!=0){
            if(background_mesh_.vertices) rsxFree(background_mesh_.vertices);
            if(background_mesh_.indices) rsxFree(background_mesh_.indices);
            background_mesh_={};last_error_="Orbit background geometry allocation/mapping failed";return false;
        }
        std::memcpy(background_mesh_.vertices,vertices.data(),sizeof(vertices));
        std::memcpy(background_mesh_.indices,OrbitUiFix30::BackgroundIndices.data(),sizeof(OrbitUiFix30::BackgroundIndices));
        background_mesh_.vertex_count=vertices.size();background_mesh_.index_count=OrbitUiFix30::BackgroundIndices.size();
        __asm__ volatile("sync" ::: "memory");
    }
#endif
    GpuTextureStage1 fresh;
    if(!stage1_->prepare_overlay(OrbitUiFix30::background(),fresh)){last_error_=stage1_->last_error();return false;}
    stage1_->release_cover(background_texture_);background_texture_=fresh;
    RuntimeDiag::log("ORBIT UI: background=%dx%d font=Noto Sans Light controls=compact help=SELECT",fresh.width,fresh.height);
#ifdef PS3_GAME_ORBIT_FIX36
    if(!prepare_animated_background())return false;
#endif
#endif
    return true;
}

bool RsxRendererV10::run_first_present_probe(){
#ifndef __PSL1GHT__
    RuntimeDiag::log("PROBE 90: host present probe skipped");
    return true;
#else
    if(!ready_ || !stage1_){
        last_error_="Renderer not ready for present probe";
        RuntimeDiag::log("PROBE 90: renderer not ready");
        return false;
    }
    auto* ctx=static_cast<gcmContextData*>(stage1_->native_context());
    if(!ctx || !color_buffer_[current_buffer_] || !depth_buffer_){
        last_error_="Present probe buffers/context unavailable";
        RuntimeDiag::log("PROBE 90: context/buffers unavailable");
        return false;
    }

    gcmSurface sf{};
    sf.colorFormat=GCM_SURFACE_X8R8G8B8;
    sf.colorTarget=GCM_SURFACE_TARGET_0;
    sf.colorLocation[0]=GCM_LOCATION_RSX;
    sf.colorOffset[0]=color_offset_[current_buffer_];
    sf.colorPitch[0]=color_pitch_;
    sf.colorLocation[1]=sf.colorLocation[2]=sf.colorLocation[3]=GCM_LOCATION_RSX;
    sf.colorOffset[1]=sf.colorOffset[2]=sf.colorOffset[3]=0;
    sf.colorPitch[1]=sf.colorPitch[2]=sf.colorPitch[3]=64;
    sf.depthFormat=GCM_SURFACE_ZETA_Z24S8;
    sf.depthLocation=GCM_LOCATION_RSX;
    sf.depthOffset=depth_offset_;
    sf.depthPitch=depth_pitch_;
    sf.type=GCM_SURFACE_TYPE_LINEAR;
    sf.antiAlias=GCM_SURFACE_CENTER_1;
    sf.width=width_; sf.height=height_; sf.x=0; sf.y=0;

    RuntimeDiag::log("PROBE 90: begin; buffer=%u format=X8R8G8B8 %ux%u",
                     (unsigned)current_buffer_, (unsigned)width_, (unsigned)height_);
    RuntimeDiag::log("PROBE 91: rsxSetSurface begin");
    rsxSetSurface(ctx,&sf);
    RuntimeDiag::log("PROBE 91: rsxSetSurface returned");

    RuntimeDiag::log("PROBE 92: clear state begin");
    rsxSetClearColor(ctx,0x304860ff);
    rsxSetClearDepthStencil(ctx,0xffffff00);
    RuntimeDiag::log("PROBE 92: clear state returned");

    RuntimeDiag::log("PROBE 93: color clear begin");
    rsxClearSurface(ctx,GCM_CLEAR_R|GCM_CLEAR_G|GCM_CLEAR_B|GCM_CLEAR_A);
    RuntimeDiag::log("PROBE 93: color clear returned");

    RuntimeDiag::log("PROBE 94: gcmSetFlip begin; buffer=%u", (unsigned)current_buffer_);
    gcmResetFlipStatus();
    gcmSetFlip(ctx,current_buffer_);
    RuntimeDiag::log("PROBE 94: gcmSetFlip returned");

    RuntimeDiag::log("PROBE 95: rsxFlushBuffer begin");
    rsxFlushBuffer(ctx);
    RuntimeDiag::log("PROBE 95: rsxFlushBuffer returned");

    // Deliberately omit gcmSetWaitFlip in this probe. We only need to prove that
    // the clear + flip command stream completes on real hardware. CPU polling is
    // bounded so this diagnostic cannot spin forever waiting for flip status.
    RuntimeDiag::log("PROBE 96: flip status polling begin; initial=%u", (unsigned)gcmGetFlipStatus());
    unsigned polls=0;
    while(gcmGetFlipStatus()!=0 && polls<10000u){
        usleep(200);
        ++polls;
    }
    const u32 status=gcmGetFlipStatus();
    RuntimeDiag::log("PROBE 96: flip polling end; status=%u polls=%u", (unsigned)status, polls);
    if(status!=0){
        last_error_="Present probe flip did not complete within timeout";
        RuntimeDiag::log("PROBE 97: TIMEOUT - no gcmSetWaitFlip was issued");
        return false;
    }
    gcmResetFlipStatus();
    current_buffer_^=1;
    RuntimeDiag::log("PROBE 99: PRESENT SUCCESS; next_buffer=%u", (unsigned)current_buffer_);
    return true;
#endif
}

bool RsxRendererV10::run_depth_clear_probe(){
#ifndef __PSL1GHT__
    RuntimeDiag::log("DEPTH 100: host depth-clear probe skipped");
    return true;
#else
    if(!ready_ || !stage1_){
        last_error_="Renderer not ready for depth-clear probe";
        RuntimeDiag::log("DEPTH 100: renderer not ready");
        return false;
    }
    auto* ctx=static_cast<gcmContextData*>(stage1_->native_context());
    if(!ctx || !color_buffer_[current_buffer_] || !depth_buffer_){
        last_error_="Depth-clear probe buffers/context unavailable";
        RuntimeDiag::log("DEPTH 100: context/buffers unavailable");
        return false;
    }

    gcmSurface sf{};
    sf.colorFormat=GCM_SURFACE_X8R8G8B8;
    sf.colorTarget=GCM_SURFACE_TARGET_0;
    sf.colorLocation[0]=GCM_LOCATION_RSX;
    sf.colorOffset[0]=color_offset_[current_buffer_];
    sf.colorPitch[0]=color_pitch_;
    sf.colorLocation[1]=sf.colorLocation[2]=sf.colorLocation[3]=GCM_LOCATION_RSX;
    sf.colorOffset[1]=sf.colorOffset[2]=sf.colorOffset[3]=0;
    sf.colorPitch[1]=sf.colorPitch[2]=sf.colorPitch[3]=64;
    sf.depthFormat=GCM_SURFACE_ZETA_Z24S8;
    sf.depthLocation=GCM_LOCATION_RSX;
    sf.depthOffset=depth_offset_;
    sf.depthPitch=depth_pitch_;
    sf.type=GCM_SURFACE_TYPE_LINEAR;
    sf.antiAlias=GCM_SURFACE_CENTER_1;
    sf.width=width_; sf.height=height_; sf.x=0; sf.y=0;

    RuntimeDiag::log("DEPTH 100: begin; buffer=%u format=X8R8G8B8 depth=Z24S8 %ux%u",
                     (unsigned)current_buffer_, (unsigned)width_, (unsigned)height_);
    RuntimeDiag::log("DEPTH 101: rsxSetSurface begin");
    rsxSetSurface(ctx,&sf);
    RuntimeDiag::log("DEPTH 101: rsxSetSurface returned");

    RuntimeDiag::log("DEPTH 102: clear state begin");
    rsxSetClearColor(ctx,0x385038ff);
    rsxSetClearDepthStencil(ctx,0xffffff00);
    RuntimeDiag::log("DEPTH 102: clear state returned");

    RuntimeDiag::log("DEPTH 103: color+Z clear begin");
    rsxClearSurface(ctx,GCM_CLEAR_R|GCM_CLEAR_G|GCM_CLEAR_B|GCM_CLEAR_A|GCM_CLEAR_Z);
    RuntimeDiag::log("DEPTH 103: color+Z clear returned");

    RuntimeDiag::log("DEPTH 104: gcmSetFlip begin; buffer=%u", (unsigned)current_buffer_);
    gcmResetFlipStatus();
    gcmSetFlip(ctx,current_buffer_);
    RuntimeDiag::log("DEPTH 104: gcmSetFlip returned");

    RuntimeDiag::log("DEPTH 105: rsxFlushBuffer begin");
    rsxFlushBuffer(ctx);
    RuntimeDiag::log("DEPTH 105: rsxFlushBuffer returned");

    RuntimeDiag::log("DEPTH 106: flip status polling begin; initial=%u", (unsigned)gcmGetFlipStatus());
    unsigned polls=0;
    while(gcmGetFlipStatus()!=0 && polls<10000u){ usleep(200); ++polls; }
    const u32 status=gcmGetFlipStatus();
    RuntimeDiag::log("DEPTH 106: flip polling end; status=%u polls=%u", (unsigned)status, polls);
    if(status!=0){
        last_error_="Depth-clear probe flip did not complete within timeout";
        RuntimeDiag::log("DEPTH 107: TIMEOUT - gcmSetWaitFlip was NOT issued");
        return false;
    }
    gcmResetFlipStatus();
    current_buffer_^=1;
    RuntimeDiag::log("DEPTH 109: SUCCESS; color+Z clear and flip completed; next_buffer=%u", (unsigned)current_buffer_);
    return true;
#endif
}

bool RsxRendererV10::run_boot_visual_polling_probe(){
#ifndef __PSL1GHT__
    RuntimeDiag::log("WAITFIX 110: host boot-visual polling probe skipped");
    return true;
#else
    if(!ready_ || !stage1_){
        last_error_="Renderer not ready for boot-visual polling probe";
        RuntimeDiag::log("WAITFIX 110: renderer not ready");
        return false;
    }
    RuntimeDiag::log("WAITFIX 110: begin; invoking corrected boot visual path");
    const bool ok=present_boot_visual_ps3(BootVisualStage::Ready);
    RuntimeDiag::log("WAITFIX 119: boot visual polling probe=%s", ok ? "OK" : "FAILED");
    return ok;
#endif
}

bool RsxRendererV10::run_single_shell_draw_probe(){
#ifndef __PSL1GHT__
    RuntimeDiag::log("DRAW 120: host single-shell draw probe skipped");
    return true;
#else
    if(!ready_ || !stage1_){
        last_error_="Renderer not ready for single-shell draw probe";
        RuntimeDiag::log("DRAW 120: renderer not ready");
        return false;
    }
    auto* ctx=static_cast<gcmContextData*>(stage1_->native_context());
    if(!ctx || gpu_mesh_.empty() || !color_buffer_[current_buffer_] || !depth_buffer_){
        last_error_="Single-shell draw probe context/buffers/geometry unavailable";
        RuntimeDiag::log("DRAW 120: context/buffers/geometry unavailable");
        return false;
    }
    const GpuMeshPart& gp=gpu_mesh_.front(); // ShellFront: approved V14 geometry, untextured.
    if(!gp.vertices || !gp.indices || gp.index_count==0){
        last_error_="Single-shell draw probe mesh part is invalid";
        RuntimeDiag::log("DRAW 120: first mesh part invalid");
        return false;
    }

    const auto current = reinterpret_cast<std::uintptr_t>(static_cast<void*>(ctx->current));
    const auto begin = reinterpret_cast<std::uintptr_t>(static_cast<void*>(ctx->begin));
    const auto end = reinterpret_cast<std::uintptr_t>(static_cast<void*>(ctx->end));
    if(current < begin || current > end || end - current < 16384u){
        last_error_="Single-shell draw command buffer has insufficient room";
        return false;
    }
    gcmResetFlipStatus();
    RuntimeDiag::log("DRAW 120: begin; FIX20 after magenta warmup; part=0 vertices=%u indices=%u buffer=%u",
                     (unsigned)gp.vertex_count,(unsigned)gp.index_count,(unsigned)current_buffer_);

    gcmSurface sf{};
    sf.colorFormat=GCM_SURFACE_X8R8G8B8;
    sf.colorTarget=GCM_SURFACE_TARGET_0;
    sf.colorLocation[0]=GCM_LOCATION_RSX;
    sf.colorOffset[0]=color_offset_[current_buffer_];
    sf.colorPitch[0]=color_pitch_;
    sf.colorLocation[1]=sf.colorLocation[2]=sf.colorLocation[3]=GCM_LOCATION_RSX;
    sf.colorOffset[1]=sf.colorOffset[2]=sf.colorOffset[3]=0;
    sf.colorPitch[1]=sf.colorPitch[2]=sf.colorPitch[3]=64;
    sf.depthFormat=GCM_SURFACE_ZETA_Z24S8;
    sf.depthLocation=GCM_LOCATION_RSX;
    sf.depthOffset=depth_offset_;
    sf.depthPitch=depth_pitch_;
    sf.type=GCM_SURFACE_TYPE_LINEAR;
    sf.antiAlias=GCM_SURFACE_CENTER_1;
    sf.width=width_; sf.height=height_; sf.x=0; sf.y=0;

    RuntimeDiag::log("DRAW 121: rsxSetSurface begin");
    rsxSetSurface(ctx,&sf);
    RuntimeDiag::log("DRAW 121: rsxSetSurface returned");

    const f32 viewport_scale[4]={width_*0.5f,height_*-0.5f,0.5f,0.0f};
    const f32 viewport_offset[4]={width_*0.5f,height_*0.5f,0.5f,0.0f};
    RuntimeDiag::log("DRAW 122: fixed render state begin");
    rsxSetViewport(ctx,0,0,(u16)width_,(u16)height_,0.0f,1.0f,viewport_scale,viewport_offset);
    rsxSetScissor(ctx,0,0,(u16)width_,(u16)height_);
    rsxSetColorMask(ctx,GCM_COLOR_MASK_R|GCM_COLOR_MASK_G|GCM_COLOR_MASK_B|GCM_COLOR_MASK_A);
    rsxSetDepthTestEnable(ctx,GCM_TRUE);
    rsxSetDepthFunc(ctx,GCM_LEQUAL);
    rsxSetDepthWriteEnable(ctx,GCM_TRUE);
    rsxSetCullFaceEnable(ctx,GCM_FALSE);
    rsxSetShadeModel(ctx,GCM_SHADE_MODEL_SMOOTH);
    rsxSetBlendEnable(ctx,GCM_FALSE);
    RuntimeDiag::log("DRAW 122: fixed render state returned");

    rsxSetClearColor(ctx,0xff0040ff); // FIX13: deliberately bright background for human-visible hardware probe.
    rsxSetClearDepthStencil(ctx,0xffffff00);
    RuntimeDiag::log("DRAW 123: bright color+Z clear begin; clear=0xff0040ff");
    rsxClearSurface(ctx,GCM_CLEAR_R|GCM_CLEAR_G|GCM_CLEAR_B|GCM_CLEAR_A|GCM_CLEAR_Z);
    RuntimeDiag::log("DRAW 123: color+Z clear returned");

    auto* vp=(const rsxVertexProgram*)vertex_program_;
    auto* fp=(const rsxFragmentProgram*)fragment_program_;
    RuntimeDiag::log("DRAW 124: load shaders begin");
    rsxLoadVertexProgram(ctx,vp,vertex_ucode_);
    RuntimeDiag::log("DRAW 124: vertex load returned; fragment load follows constants");

    const float aspect=height_>0 ? float(width_)/float(height_) : 16.0f/9.0f;
    const Mat4 proj=mat4_perspective(29.0f,aspect,1.0f,2000.0f);
    const Mat4 view=mat4_look_at(0.0f,2.0f,425.0f, 0.0f,0.0f,0.0f, 0.0f,1.0f,0.0f);
    const Mat4 model=mat4_identity();
    const Mat4 mvp=mat4_mul(mat4_mul(proj,view),model);
    const float uv_transform[4]={1.0f,1.0f,0.0f,0.0f};
    const float shell_color[4]={1.0f,1.0f,0.0f,1.0f}; // FIX13: bright yellow shell for visibility.
    const float use_texture[4]={0.0f,0.0f,0.0f,0.0f};
    const float alpha[4]={1.0f,0.0f,0.0f,0.0f};

    RuntimeDiag::log("DRAW 125: shader parameters begin");
    const auto mvp_rows=mat4_shader_rows(mvp);
    const auto model_rows=mat4_shader_rows(model);
    rsxSetVertexProgramParameter(ctx,vp,(const rsxProgramConst*)vp_mvp_,mvp_rows.data());
    rsxSetVertexProgramParameter(ctx,vp,(const rsxProgramConst*)vp_model_,model_rows.data());
    rsxSetVertexProgramParameter(ctx,vp,(const rsxProgramConst*)vp_uv_transform_,uv_transform);
    rsxSetFragmentProgramParameter(ctx,fp,(const rsxProgramConst*)fp_base_color_,shell_color,fragment_offset_,GCM_LOCATION_RSX);
    rsxSetFragmentProgramParameter(ctx,fp,(const rsxProgramConst*)fp_use_texture_,use_texture,fragment_offset_,GCM_LOCATION_RSX);
    rsxSetFragmentProgramParameter(ctx,fp,(const rsxProgramConst*)fp_alpha_,alpha,fragment_offset_,GCM_LOCATION_RSX);
    // InlineTransfer updates embedded constants; activate the program afterwards.
    rsxLoadFragmentProgramLocation(ctx,fp,fragment_offset_,GCM_LOCATION_RSX);
    rsxTextureControl(ctx,0,GCM_FALSE,0,0,GCM_TEXTURE_MAX_ANISO_1);
    RuntimeDiag::log("DRAW 125: shader parameters returned");

    const u8 stride=(u8)sizeof(V14Vertex);
    RuntimeDiag::log("DRAW 126: bind vertex arrays begin; stride=%u pos=%d norm=%d uv=%d",
                     (unsigned)stride,attr_position_,attr_normal_,attr_uv_);
    rsxBindVertexArrayAttrib(ctx,(u8)attr_position_,0,gp.vertex_offset+(u32)offsetof(V14Vertex,x),stride,3,GCM_VERTEX_DATA_TYPE_F32,GCM_LOCATION_RSX);
    rsxBindVertexArrayAttrib(ctx,(u8)attr_normal_,0,gp.vertex_offset+(u32)offsetof(V14Vertex,nx),stride,3,GCM_VERTEX_DATA_TYPE_F32,GCM_LOCATION_RSX);
    rsxBindVertexArrayAttrib(ctx,(u8)attr_uv_,0,gp.vertex_offset+(u32)offsetof(V14Vertex,u),stride,2,GCM_VERTEX_DATA_TYPE_F32,GCM_LOCATION_RSX);
    RuntimeDiag::log("DRAW 126: bind vertex arrays returned");

    RuntimeDiag::log("DRAW 127: rsxDrawIndexArray begin");
    rsxDrawIndexArray(ctx,GCM_TYPE_TRIANGLES,gp.index_offset,gp.index_count,GCM_INDEX_TYPE_16B,GCM_LOCATION_RSX);
    RuntimeDiag::log("DRAW 127: rsxDrawIndexArray returned");

    RuntimeDiag::log("DRAW 128: gcmSetFlip begin; buffer=%u",(unsigned)current_buffer_);
    const int presented_buffer=current_buffer_;
    const s32 flip_rc=gcmSetFlip(ctx,current_buffer_);
    RuntimeDiag::log("DRAW 128: gcmSetFlip returned; rc=%d", int(flip_rc));
    if(flip_rc!=0){ last_error_="Single-shell gcmSetFlip failed"; return false; }
    RuntimeDiag::log("DRAW 128: rsxFlushBuffer begin");
    rsxFlushBuffer(ctx);
    RuntimeDiag::log("DRAW 128: rsxFlushBuffer returned");
    if(!wait_for_flip_polling_ps3("DRAW 129",10000u)){
        RuntimeDiag::log("DRAW 129: flip polling failed: %s",last_error_.c_str());
        return false;
    }
    current_buffer_^=1;
    RsxPresentFix20::log_display("after_draw");
    RsxPresentFix20::log_buffers("after_draw",color_offset_,width_,height_,color_pitch_);
    const auto* pixels=static_cast<volatile u32*>(color_buffer_[presented_buffer]);
    const std::size_t pixel_count=std::size_t(color_pitch_)*height_/sizeof(u32);
    RuntimeDiag::log("DRAW 130: CPU samples; buffer=%u first=0x%08x middle=0x%08x center=0x%08x last=0x%08x",
                     unsigned(presented_buffer),pixels[0],pixels[pixel_count/2],
                     pixels[std::size_t(height_/2)*(color_pitch_/sizeof(u32))+width_/2],pixels[pixel_count-1]);
    RuntimeDiag::log("DRAW 131: draw/flip completed; TV visibility requires user report; next_buffer=%u",
                     (unsigned)current_buffer_);
    return true;
#endif
}

bool RsxRendererV10::run_cpu_framebuffer_probe(){
#ifndef __PSL1GHT__
    RuntimeDiag::log("CPUFB 140: host CPU framebuffer probe skipped");
    return true;
#else
    if(!ready_ || !stage1_){
        last_error_="Renderer not ready for CPU framebuffer probe";
        RuntimeDiag::log("CPUFB 140: renderer not ready");
        return false;
    }
    auto* ctx=static_cast<gcmContextData*>(stage1_->native_context());
    if(!ctx || !color_buffer_[0] || !color_buffer_[1]){
        last_error_="CPU framebuffer probe context/color buffer unavailable";
        RuntimeDiag::log("CPUFB 140: context/color buffer unavailable");
        return false;
    }

#ifdef PS3_GAME_ORBIT_FIX30
    const auto file=load_cover_file(ProjectIdentity::SplashPath,GameCoverKind::None);
    DecodedImageRGBA artwork;std::string artwork_error;
    const bool artwork_ok=file.valid() && file.width==1280 && file.height==720 && decode_cover_rgba(file,artwork,artwork_error);
    const auto image=OrbitUiFix30::splash(artwork_ok ? &artwork : nullptr);
    std::vector<std::uint32_t> startup(std::size_t(width_)*height_);
    for(std::size_t i=0;i<startup.size();++i){const auto* c=image.rgba.data()+i*4;startup[i]=0xff000000u|(unsigned(c[0])<<16)|(unsigned(c[1])<<8)|c[2];}
    RuntimeDiag::log("ORBIT STARTUP: artwork=%u fallback=%u dimensions=%dx%d",unsigned(artwork_ok),unsigned(!artwork_ok),image.width,image.height);
    if(!RsxPresentFix20::warm_up(ctx,color_buffer_,color_offset_,width_,height_,color_pitch_,last_error_,startup.data(),startup.size()))
#else
    if(!RsxPresentFix20::warm_up(ctx,color_buffer_,color_offset_,width_,height_,color_pitch_,last_error_))
#endif
        return false;
    current_buffer_=0; // Warmup finishes displaying buffer 1; draw into the other buffer.
    RuntimeDiag::log("CPUFB 149: FIX19 sequence completed on renderer buffers; next_buffer=%u",
                     (unsigned)current_buffer_);
    return true;
#endif
}

#ifdef __PSL1GHT__
namespace {
static std::uint32_t identity_texture_remap(){
    return
        (GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_A_SHIFT) |
        (GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_R_SHIFT) |
        (GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_G_SHIFT) |
        (GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_B_SHIFT) |
        (GCM_TEXTURE_REMAP_COLOR_A << GCM_TEXTURE_REMAP_COLOR_A_SHIFT) |
        (GCM_TEXTURE_REMAP_COLOR_R << GCM_TEXTURE_REMAP_COLOR_R_SHIFT) |
        (GCM_TEXTURE_REMAP_COLOR_G << GCM_TEXTURE_REMAP_COLOR_G_SHIFT) |
        (GCM_TEXTURE_REMAP_COLOR_B << GCM_TEXTURE_REMAP_COLOR_B_SHIFT);
}

static bool surface_uses_texture(V14Surface s,const GpuTextureStage1* tex){
    if(!tex || !tex->uploaded) return false;
    if(s==V14Surface::CoverFront) return true;
    if(!tex->full_cover) return false;
    return s==V14Surface::CoverBack || s==V14Surface::CoverSpine;
}
}

bool RsxRendererV10::init_display_ps3(){
    RuntimeDiag::log("RENDER 11: acquire GCM context");
    auto* ctx=static_cast<gcmContextData*>(stage1_->native_context());
    if(!ctx){ last_error_="Missing GCM context"; return false; }
    RuntimeDiag::log("RENDER 11: GCM context OK");
    if(!RsxPresentFix20::acknowledge(ctx,last_error_)) return false;

    // Match FIX19's 720p/XRGB/AUTO configuration and bounded readback.
    RuntimeDiag::log("RENDER 12: FIX20 check 720p availability begin");
    const s32 availability=videoGetResolutionAvailability(VIDEO_PRIMARY,VIDEO_RESOLUTION_720,VIDEO_ASPECT_AUTO,0);
    RuntimeDiag::log("RENDER 12: FIX20 720p availability=%d", (int)availability);
    if(availability!=1){ last_error_="720p video mode is not available"; return false; }

    RuntimeDiag::log("RENDER 13: videoGetResolution(720p) begin");
    videoResolution res{};
    if(videoGetResolution(VIDEO_RESOLUTION_720,&res)!=0){ last_error_="videoGetResolution(720p) failed"; return false; }
    width_=res.width; height_=res.height;
    if(width_!=1280 || height_!=720){ last_error_="Unexpected 720p resolution dimensions"; return false; }
    RuntimeDiag::log("RENDER 13: FIX20 resolution selected; id=%u %u x %u",
                     (unsigned)VIDEO_RESOLUTION_720,(unsigned)width_,(unsigned)height_);

    videoConfiguration cfg{};
    cfg.resolution=VIDEO_RESOLUTION_720;
    cfg.format=VIDEO_BUFFER_FORMAT_XRGB;
    cfg.aspect=VIDEO_ASPECT_AUTO;
    cfg.pitch=width_*4;
    RuntimeDiag::log("RENDER 14: FIX20 videoConfigure begin; aspect=AUTO pitch=%u", (unsigned)cfg.pitch);
    if(videoConfigure(VIDEO_PRIMARY,&cfg,nullptr,0)!=0){ last_error_="videoConfigure(720p) failed"; return false; }
    RuntimeDiag::log("RENDER 14: videoConfigure returned; readback begin");
    const s64 video_start=sysGetSystemTime();
    videoState state{};
    videoConfiguration actual{};
    bool configured=false;
    do {
        const s32 state_rc=videoGetState(VIDEO_PRIMARY,0,&state);
        const s32 config_rc=videoGetConfiguration(VIDEO_PRIMARY,&actual,nullptr);
        if(state_rc!=0 || config_rc!=0){
            RuntimeDiag::log("RENDER 14: readback failed; state_rc=%d config_rc=%d",int(state_rc),int(config_rc));
            last_error_="Video configuration readback failed"; return false;
        }
        // FIX19 accepted the matching mode/configuration without interpreting
        // state.state: the pinned header's state constants differ from its log.
        configured=state.displayMode.resolution==VIDEO_RESOLUTION_720 &&
                   actual.resolution==VIDEO_RESOLUTION_720 &&
                   actual.format==VIDEO_BUFFER_FORMAT_XRGB && actual.pitch==cfg.pitch;
        if(configured) break;
        sysUtilCheckCallback();
        if(Ps3Lifecycle::global_exit_requested()) { last_error_="Exit requested during video setup"; return false; }
        usleep(1000);
    } while(sysGetSystemTime()-video_start<3000000);
    RuntimeDiag::log("RENDER 14: state=%u mode=%u config=%u format=%u aspect=%u pitch=%u color_space=%u scan=%u conversion=%u refresh=0x%04x elapsed_us=%lld",
                     unsigned(state.state),unsigned(state.displayMode.resolution),unsigned(actual.resolution),
                     unsigned(actual.format),unsigned(actual.aspect),unsigned(actual.pitch),unsigned(state.colorSpace),
                     unsigned(state.displayMode.scanMode),unsigned(state.displayMode.conversion),
                     unsigned(state.displayMode.refreshRates),static_cast<long long>(sysGetSystemTime()-video_start));
    if(!configured){ last_error_="720p configuration polling reached its deadline"; return false; }
    RuntimeDiag::log("RENDER 14: gcmSetFlipMode VSYNC begin");
    gcmSetFlipMode(GCM_FLIP_VSYNC);
    RuntimeDiag::log("RENDER 14: gcmSetFlipMode returned");

    color_pitch_=width_*4;
    const std::size_t color_bytes=std::size_t(color_pitch_)*height_;
    for(int i=0;i<2;++i){
        RuntimeDiag::log("RENDER 15.%u: color rsxMemalign begin; bytes=%u", (unsigned)i, (unsigned)color_bytes);
        color_buffer_[i]=rsxMemalign(64,(u32)color_bytes);
        RuntimeDiag::log("RENDER 15.%u: color rsxMemalign returned", (unsigned)i);
        if(!color_buffer_[i]){ last_error_="color buffer allocation failed"; return false; }
        RuntimeDiag::log("RENDER 16.%u: rsxAddressToOffset begin", (unsigned)i);
        if(rsxAddressToOffset(color_buffer_[i],&color_offset_[i])!=0){
            last_error_="color buffer offset mapping failed"; return false;
        }
        RuntimeDiag::log("RENDER 16.%u: offset OK; gcmSetDisplayBuffer begin", (unsigned)i);
        const s32 rc=gcmSetDisplayBuffer(i,color_offset_[i],color_pitch_,width_,height_);
        RuntimeDiag::log("RENDER 16.%u: gcmSetDisplayBuffer returned; rc=%d offset=0x%08x", (unsigned)i,int(rc),color_offset_[i]);
        if(rc!=0){ last_error_="gcmSetDisplayBuffer failed"; return false; }
    }
    RsxPresentFix20::log_buffers("renderer_registration",color_offset_,width_,height_,color_pitch_);

    depth_pitch_=width_*4;
    const std::size_t depth_bytes=std::size_t(depth_pitch_)*height_;
    RuntimeDiag::log("RENDER 17: depth rsxMemalign begin; bytes=%u", (unsigned)depth_bytes);
    depth_buffer_=rsxMemalign(64,(u32)depth_bytes);
    RuntimeDiag::log("RENDER 17: depth rsxMemalign returned");
    if(!depth_buffer_){ last_error_="depth buffer allocation failed"; return false; }
    RuntimeDiag::log("RENDER 17: depth rsxAddressToOffset begin");
    if(rsxAddressToOffset(depth_buffer_,&depth_offset_)!=0){
        last_error_="depth buffer offset mapping failed"; return false;
    }
    RuntimeDiag::log("RENDER 17: depth offset OK");

    RuntimeDiag::log("RENDER 18: gcmResetFlipStatus begin");
    gcmResetFlipStatus();
    RuntimeDiag::log("RENDER 18: gcmResetFlipStatus returned");
    current_buffer_=0;
    return true;
}

bool RsxRendererV10::wait_for_flip_polling_ps3(const char* tag,unsigned max_polls){
    (void)max_polls;
    return RsxPresentFix20::wait_flip(tag,last_error_);
}

bool RsxRendererV10::present_boot_visual_ps3(BootVisualStage stage){
    auto* ctx=static_cast<gcmContextData*>(stage1_ ? stage1_->native_context() : nullptr);
    if(!ctx || !color_buffer_[current_buffer_] || !depth_buffer_) return false;

    RuntimeDiag::log("WAITFIX 111: boot visual surface begin; buffer=%u", (unsigned)current_buffer_);
    gcmSurface sf{};
    sf.colorFormat=GCM_SURFACE_X8R8G8B8;
    sf.colorTarget=GCM_SURFACE_TARGET_0;
    sf.colorLocation[0]=GCM_LOCATION_RSX;
    sf.colorOffset[0]=color_offset_[current_buffer_];
    sf.colorPitch[0]=color_pitch_;
    sf.colorLocation[1]=sf.colorLocation[2]=sf.colorLocation[3]=GCM_LOCATION_RSX;
    sf.colorOffset[1]=sf.colorOffset[2]=sf.colorOffset[3]=0;
    sf.colorPitch[1]=sf.colorPitch[2]=sf.colorPitch[3]=64;
    sf.depthFormat=GCM_SURFACE_ZETA_Z24S8;
    sf.depthLocation=GCM_LOCATION_RSX;
    sf.depthOffset=depth_offset_;
    sf.depthPitch=depth_pitch_;
    sf.type=GCM_SURFACE_TYPE_LINEAR;
    sf.antiAlias=GCM_SURFACE_CENTER_1;
    sf.width=width_; sf.height=height_; sf.x=0; sf.y=0;

    rsxSetSurface(ctx,&sf);
    RuntimeDiag::log("WAITFIX 112: rsxSetSurface returned");
    rsxSetClearColor(ctx,boot_visual_color(stage));
    rsxSetClearDepthStencil(ctx,0xffffff00);
    rsxClearSurface(ctx,GCM_CLEAR_R|GCM_CLEAR_G|GCM_CLEAR_B|GCM_CLEAR_A|GCM_CLEAR_Z);
    RuntimeDiag::log("WAITFIX 113: color+Z clear returned");

    gcmResetFlipStatus();
    gcmSetFlip(ctx,current_buffer_);
    RuntimeDiag::log("WAITFIX 114: gcmSetFlip returned");
    rsxFlushBuffer(ctx);
    RuntimeDiag::log("WAITFIX 115: rsxFlushBuffer returned");
    if(!wait_for_flip_polling_ps3("WAITFIX 116",10000u)){
        RuntimeDiag::log("WAITFIX 117: polling TIMEOUT");
        return false;
    }
    current_buffer_^=1;
    RuntimeDiag::log("WAITFIX 118: boot visual present OK; next_buffer=%u", (unsigned)current_buffer_);
    return true;
}

bool RsxRendererV10::init_shaders_ps3(){
    RuntimeDiag::log("RENDER 30.0: shader blobs validate begin; vpo=%u fpo=%u",
                     (unsigned)v14_case_vpo_size, (unsigned)v14_case_fpo_size);
    if(v14_case_vpo_size<sizeof(rsxVertexProgram) || v14_case_fpo_size<sizeof(rsxFragmentProgram)){
        last_error_="Embedded VPO/FPO are missing or invalid";
        return false;
    }

    RuntimeDiag::log("RENDER 30.1: shader blobs OK");
    auto* vp=reinterpret_cast<const rsxVertexProgram*>(v14_case_vpo);
    auto* fp=reinterpret_cast<const rsxFragmentProgram*>(v14_case_fpo);
    vertex_program_=vp;
    fragment_program_=fp;

    void* vp_ucode=nullptr;
    u32 vp_ucode_size=0;
    RuntimeDiag::log("RENDER 30.2: vertex ucode query begin");
    rsxVertexProgramGetUCode(vp,&vp_ucode,&vp_ucode_size);
    RuntimeDiag::log("RENDER 30.2: vertex ucode query returned; bytes=%u", (unsigned)vp_ucode_size);
    if(!vp_ucode || vp_ucode_size==0){ last_error_="Vertex shader ucode not found"; return false; }
    vertex_ucode_=vp_ucode;

    void* fp_ucode=nullptr;
    u32 fp_ucode_size=0;
    RuntimeDiag::log("RENDER 30.3: fragment ucode query begin");
    rsxFragmentProgramGetUCode(fp,&fp_ucode,&fp_ucode_size);
    RuntimeDiag::log("RENDER 30.3: fragment ucode query returned; bytes=%u", (unsigned)fp_ucode_size);
    if(!fp_ucode || fp_ucode_size==0){ last_error_="Fragment shader ucode not found"; return false; }
    RuntimeDiag::log("RENDER 30.4: fragment rsxMemalign begin");
    fragment_ucode_=rsxMemalign(64,fp_ucode_size);
    RuntimeDiag::log("RENDER 30.4: fragment rsxMemalign returned");
    if(!fragment_ucode_){ last_error_="Could not allocate fragment shader ucode"; return false; }
    std::memcpy(fragment_ucode_,fp_ucode,fp_ucode_size);
    RuntimeDiag::log("RENDER 30.5: fragment rsxAddressToOffset begin");
    if(rsxAddressToOffset(fragment_ucode_,&fragment_offset_)!=0){
        last_error_="Could not map fragment shader ucode"; return false;
    }
    RuntimeDiag::log("RENDER 30.5: fragment offset OK");

    RuntimeDiag::log("RENDER 30.6: shader constants lookup begin");
    vp_mvp_=rsxVertexProgramGetConst(vp,"modelViewProj");
    vp_model_=rsxVertexProgramGetConst(vp,"model");
    vp_uv_transform_=rsxVertexProgramGetConst(vp,"uvTransform");
    fp_base_color_=rsxFragmentProgramGetConst(fp,"baseColor");
    fp_use_texture_=rsxFragmentProgramGetConst(fp,"useTexture");
    fp_alpha_=rsxFragmentProgramGetConst(fp,"alpha");

    RuntimeDiag::log("RENDER 30.6: shader constants lookup returned");
    RuntimeDiag::log("RENDER 30.7: vertex attributes lookup begin");
    const rsxProgramAttrib* attr_position=rsxVertexProgramGetAttrib(vp,"position");
    const rsxProgramAttrib* attr_normal=rsxVertexProgramGetAttrib(vp,"normal");
    const rsxProgramAttrib* attr_uv=rsxVertexProgramGetAttrib(vp,"texcoord");

    attr_position_=attr_position ? static_cast<int>(attr_position->index) : -1;
    attr_normal_=attr_normal ? static_cast<int>(attr_normal->index) : -1;
    attr_uv_=attr_uv ? static_cast<int>(attr_uv->index) : -1;
    RuntimeDiag::log("RENDER 30.7: attributes returned; pos=%d norm=%d uv=%d",
                     attr_position_, attr_normal_, attr_uv_);

    if(!vp_mvp_ || !vp_model_ || !vp_uv_transform_ || !fp_base_color_ || !fp_use_texture_ || !fp_alpha_ ||
       attr_position_<0 || attr_normal_<0 || attr_uv_<0){
        last_error_="Shader interface mismatch (constants/attributes not found)";
        return false;
    }
    RuntimeDiag::log("RENDER 30.9: shader interface OK");
    return true;
}

bool RsxRendererV10::upload_geometry_ps3(const V14CaseMesh& mesh){
    RuntimeDiag::log("RENDER 40.0: geometry upload internal begin; parts=%u", (unsigned)mesh.parts.size());
    release_geometry_ps3();
    gpu_mesh_.reserve(mesh.parts.size());
    unsigned part_index=0;
    for(const auto& part:mesh.parts){
        RuntimeDiag::log("RENDER 40.%u: part begin; vertices=%u indices=%u",
                         part_index+1u, (unsigned)part.vertices.size(), (unsigned)part.indices.size());
        GpuMeshPart gp{};
        gp.surface=part.surface;
        gp.textured=part.textured;
        gp.vertex_count=(std::uint32_t)part.vertices.size();
        gp.index_count=(std::uint32_t)part.indices.size();
        const std::size_t vb=part.vertices.size()*sizeof(V14Vertex);
        const std::size_t ib=part.indices.size()*sizeof(std::uint16_t);
        gp.vertices=rsxMemalign(128,(u32)vb);
        gp.indices=rsxMemalign(128,(u32)ib);
        RuntimeDiag::log("RENDER 40.%u: allocations returned", part_index+1u);
        if(!gp.vertices || !gp.indices){ last_error_="V14 geometry allocation failed"; release_geometry_ps3(); return false; }
        std::memcpy(gp.vertices,part.vertices.data(),vb);
        const auto indices=CaseRenderFix25::outward_indices(part);
        std::memcpy(gp.indices,indices.data(),ib);
        unsigned repaired=0;
        for(std::size_t i=0;i<indices.size();i+=3) if(indices[i+1]!=part.indices[i+1]) ++repaired;
        RuntimeDiag::log("GEOMETRY FIX25: surface=%u triangles=%u repaired=%u",unsigned(part.surface),unsigned(indices.size()/3),repaired);
        if(rsxAddressToOffset(gp.vertices,&gp.vertex_offset)!=0 || rsxAddressToOffset(gp.indices,&gp.index_offset)!=0){
            last_error_="V14 geometry offset mapping failed"; release_geometry_ps3(); return false;
        }
        gpu_mesh_.push_back(gp);
        RuntimeDiag::log("RENDER 40.%u: offsets OK", part_index+1u);
        ++part_index;
    }
    __asm__ volatile("sync" ::: "memory");
    RuntimeDiag::log("RENDER 40.9: geometry upload internal complete; parts=%u", (unsigned)gpu_mesh_.size());
    return gpu_mesh_.size()==mesh.parts.size();
}

void RsxRendererV10::release_geometry_ps3(){
    for(auto& p:gpu_mesh_){
        if(p.vertices) rsxFree(p.vertices);
        if(p.indices) rsxFree(p.indices);
    }
    gpu_mesh_.clear();
}

void RsxRendererV10::setup_texture_ps3(const GpuTextureStage1& tex){
    auto* ctx=static_cast<gcmContextData*>(stage1_->native_context());
    gcmTexture t{};
    t.format=GCM_TEXTURE_FORMAT_A8R8G8B8 | GCM_TEXTURE_FORMAT_LIN | GCM_TEXTURE_FORMAT_NRM;
    t.mipmap=1; // One base level; min/max LOD remain zero and no mip chain is sampled.
    t.dimension=GCM_TEXTURE_DIMS_2D;
    t.cubemap=GCM_FALSE;
    t.remap=identity_texture_remap();
    t.width=(u16)tex.width;
    t.height=(u16)tex.height;
    t.depth=1;
    t.location=GCM_LOCATION_RSX;
    t.pitch=(u32)tex.pitch;
    t.offset=tex.gpu_offset;
    rsxInvalidateTextureCache(ctx,GCM_INVALIDATE_TEXTURE);
    rsxLoadTexture(ctx,0,&t);
    rsxTextureControl(ctx,0,GCM_TRUE,0,0,GCM_TEXTURE_MAX_ANISO_1);
    rsxTextureFilter(ctx,0,0,GCM_TEXTURE_LINEAR,GCM_TEXTURE_LINEAR,GCM_TEXTURE_CONVOLUTION_QUINCUNX);
    rsxTextureWrapMode(ctx,0,GCM_TEXTURE_CLAMP_TO_EDGE,GCM_TEXTURE_CLAMP_TO_EDGE,GCM_TEXTURE_CLAMP_TO_EDGE,
                       GCM_TEXTURE_UNSIGNED_REMAP_NORMAL,GCM_TEXTURE_ZFUNC_NEVER,0);
}

unsigned RsxRendererV10::front_green_samples(const V14CaseMesh& mesh) const {
#ifdef PS3_GAME_ORBIT_FIX30
    const int sample_buffer=diagnostic_view_label_=="CULL_FRONT" ? current_buffer_ : current_buffer_^1;
#else
    const int sample_buffer=current_buffer_^1;
#endif
    if(last_plan_.packets.size()!=6 || !color_buffer_[sample_buffer]) return 0;
    const std::size_t part_i=mesh.jfx ? 1 : 3,packet_i=mesh.jfx ? 2 : 3;
    if(part_i>=mesh.parts.size()) return 0;
    const auto& part=mesh.parts[part_i];
    if(part.surface!=V14Surface::CoverFront || part.vertices.size()<4) return 0;
    const auto& m=last_plan_.packets[packet_i].mvp.m;
    const auto* pixels=static_cast<volatile u32*>(color_buffer_[sample_buffer]);
    unsigned green=0;
    for(int row:{0,2}) for(int col=0;col<3;++col){
        const float u=0.25f+0.25f*col,v=0.25f+0.25f*row;
        std::array<float,3> point;
        if(mesh.jfx){if(!JfxCaseFix29::surface_point(part,u,v,point)) continue;}
        else point=full_cover_surface_point_fix26(part,u,v);
        const float x=point[0],y=point[1],z=point[2];
        const float cx=m[0]*x+m[4]*y+m[8]*z+m[12];
        const float cy=m[1]*x+m[5]*y+m[9]*z+m[13];
        const float cw=m[3]*x+m[7]*y+m[11]*z+m[15];
        if(cw<=0) continue;
        const int px=static_cast<int>((cx/cw+1)*width_*0.5f);
        const int py=static_cast<int>((1-cy/cw)*height_*0.5f);
        if(px<0 || py<0 || px>=width_ || py>=height_) continue;
        const u32 color=pixels[std::size_t(py)*(color_pitch_/sizeof(u32))+px];
        const unsigned r=(color>>16)&255,g=(color>>8)&255,bv=color&255;
        if(g>80 && g>2*r && 5*g>6*bv) ++green;
        RuntimeDiag::log("CULL PIXEL: clockwise=%u xy=%d,%d argb=0x%08x green=%u",
                         unsigned(front_face_clockwise_),px,py,color,unsigned(g>80 && g>2*r && 5*g>6*bv));
    }
    return green;
}

#ifdef PS3_GAME_ORBIT_FIX30
bool RsxRendererV10::draw_orbit_background_ps3(){
    auto* ctx=static_cast<gcmContextData*>(stage1_->native_context());
    if(!background_texture_.uploaded) return true;
    const auto current=reinterpret_cast<std::uintptr_t>(static_cast<void*>(ctx->current));
    const auto end=reinterpret_cast<std::uintptr_t>(static_cast<void*>(ctx->end));
    if(current>end || end-current<2048u || !background_mesh_.vertices || !background_mesh_.indices){
        last_error_="Orbit background command room or geometry unavailable";return false;
    }
    auto* vp=static_cast<const rsxVertexProgram*>(vertex_program_);
    auto* fp=static_cast<const rsxFragmentProgram*>(fragment_program_);
    const auto identity=mat4_shader_rows(mat4_identity());
    const float uv[4]={1,1,0,0},color[4]={1,1,1,1},use[4]={1,0,0,0},alpha[4]={1,0,0,0};
    rsxSetDepthTestEnable(ctx,GCM_FALSE);rsxSetDepthWriteEnable(ctx,GCM_FALSE);
    rsxSetCullFaceEnable(ctx,GCM_FALSE);rsxSetBlendEnable(ctx,GCM_FALSE);
    rsxSetVertexProgramParameter(ctx,vp,static_cast<const rsxProgramConst*>(vp_mvp_),identity.data());
    rsxSetVertexProgramParameter(ctx,vp,static_cast<const rsxProgramConst*>(vp_model_),identity.data());
    rsxSetVertexProgramParameter(ctx,vp,static_cast<const rsxProgramConst*>(vp_uv_transform_),uv);
    rsxSetFragmentProgramParameter(ctx,fp,static_cast<const rsxProgramConst*>(fp_base_color_),color,fragment_offset_,GCM_LOCATION_RSX);
    rsxSetFragmentProgramParameter(ctx,fp,static_cast<const rsxProgramConst*>(fp_use_texture_),use,fragment_offset_,GCM_LOCATION_RSX);
    rsxSetFragmentProgramParameter(ctx,fp,static_cast<const rsxProgramConst*>(fp_alpha_),alpha,fragment_offset_,GCM_LOCATION_RSX);
    rsxLoadFragmentProgramLocation(ctx,fp,fragment_offset_,GCM_LOCATION_RSX);
#ifdef PS3_GAME_ORBIT_FIX36
    setup_texture_ps3(animated_background_ ? animated_base_texture_ : background_texture_);
#else
    setup_texture_ps3(background_texture_);
#endif
    const u8 stride=sizeof(V14Vertex);
    rsxBindVertexArrayAttrib(ctx,attr_position_,0,background_mesh_.vertex_offset+offsetof(V14Vertex,x),stride,3,GCM_VERTEX_DATA_TYPE_F32,GCM_LOCATION_RSX);
    rsxBindVertexArrayAttrib(ctx,attr_normal_,0,background_mesh_.vertex_offset+offsetof(V14Vertex,nx),stride,3,GCM_VERTEX_DATA_TYPE_F32,GCM_LOCATION_RSX);
    rsxBindVertexArrayAttrib(ctx,attr_uv_,0,background_mesh_.vertex_offset+offsetof(V14Vertex,u),stride,2,GCM_VERTEX_DATA_TYPE_F32,GCM_LOCATION_RSX);
    rsxDrawIndexArray(ctx,GCM_TYPE_TRIANGLES,background_mesh_.index_offset,background_mesh_.index_count,GCM_INDEX_TYPE_16B,GCM_LOCATION_RSX);
#ifdef PS3_GAME_ORBIT_FIX36
    if(!draw_orbit_waves_ps3())return false;
#endif
    return true;
}
#endif

bool RsxRendererV10::draw_library_hud_ps3(){
    auto* ctx=static_cast<gcmContextData*>(stage1_->native_context());
    if(!hud_texture_.uploaded) return true;
    const auto current=reinterpret_cast<std::uintptr_t>(static_cast<void*>(ctx->current));
    const auto end=reinterpret_cast<std::uintptr_t>(static_cast<void*>(ctx->end));
    if(current>end || end-current<2048u || !hud_mesh_.vertices || !hud_mesh_.indices){
        last_error_="HUD command room or geometry unavailable";return false;
    }
    auto* vp=static_cast<const rsxVertexProgram*>(vertex_program_);
    auto* fp=static_cast<const rsxFragmentProgram*>(fragment_program_);
    const auto identity=mat4_shader_rows(mat4_identity());
    const float uv[4]={1,1,0,0},color[4]={1,1,1,1},use[4]={1,0,0,0},alpha[4]={1,0,0,0};
    rsxSetDepthTestEnable(ctx,GCM_FALSE);rsxSetDepthWriteEnable(ctx,GCM_FALSE);
    rsxSetCullFaceEnable(ctx,GCM_FALSE);rsxSetBlendEnable(ctx,GCM_TRUE);
    rsxSetVertexProgramParameter(ctx,vp,static_cast<const rsxProgramConst*>(vp_mvp_),identity.data());
    rsxSetVertexProgramParameter(ctx,vp,static_cast<const rsxProgramConst*>(vp_model_),identity.data());
    rsxSetVertexProgramParameter(ctx,vp,static_cast<const rsxProgramConst*>(vp_uv_transform_),uv);
    rsxSetFragmentProgramParameter(ctx,fp,static_cast<const rsxProgramConst*>(fp_base_color_),color,fragment_offset_,GCM_LOCATION_RSX);
    rsxSetFragmentProgramParameter(ctx,fp,static_cast<const rsxProgramConst*>(fp_use_texture_),use,fragment_offset_,GCM_LOCATION_RSX);
    rsxSetFragmentProgramParameter(ctx,fp,static_cast<const rsxProgramConst*>(fp_alpha_),alpha,fragment_offset_,GCM_LOCATION_RSX);
    rsxLoadFragmentProgramLocation(ctx,fp,fragment_offset_,GCM_LOCATION_RSX);
    setup_texture_ps3(hud_texture_);
    const u8 stride=sizeof(V14Vertex);
    rsxBindVertexArrayAttrib(ctx,attr_position_,0,hud_mesh_.vertex_offset+offsetof(V14Vertex,x),stride,3,GCM_VERTEX_DATA_TYPE_F32,GCM_LOCATION_RSX);
    rsxBindVertexArrayAttrib(ctx,attr_normal_,0,hud_mesh_.vertex_offset+offsetof(V14Vertex,nx),stride,3,GCM_VERTEX_DATA_TYPE_F32,GCM_LOCATION_RSX);
    rsxBindVertexArrayAttrib(ctx,attr_uv_,0,hud_mesh_.vertex_offset+offsetof(V14Vertex,u),stride,2,GCM_VERTEX_DATA_TYPE_F32,GCM_LOCATION_RSX);
    rsxDrawIndexArray(ctx,GCM_TYPE_TRIANGLES,hud_mesh_.index_offset,hud_mesh_.index_count,GCM_INDEX_TYPE_16B,GCM_LOCATION_RSX);
    return true;
}

bool RsxRendererV10::draw_frame_ps3(const V10FramePlan& plan,const V14CaseMesh& mesh){
    auto* ctx=static_cast<gcmContextData*>(stage1_->native_context());
    if(!ctx){ last_error_="Missing GCM context"; return false; }
    if(gpu_mesh_.size()!=mesh.parts.size()){ last_error_="GPU V14 mesh is not uploaded"; return false; }
#ifdef PS3_SP_LOADER_FIX28
#ifdef PS3_GAME_ORBIT_FIX32
    if(!validate_native_plan_fix32(plan,mesh) ||
#else
    if(plan.packets.empty() || plan.packets.size()%6 || plan.packets.size()>LibraryPairFix28::MaxCases*6 ||
       gpu_mesh_.size()!=(mesh.jfx ? 5u : 6u) ||
#endif
       !color_buffer_[current_buffer_] || !depth_buffer_){
        last_error_="Library requires complete case ranges within its limit and valid buffers"; return false;
    }
    for(std::size_t i=0;i<plan.packets.size();++i){
        const auto part_i=native_packet_part(plan.packets[i],i,mesh);
        if(part_i>=gpu_mesh_.size()) {last_error_="Native mesh range out of bounds";return false;}
#ifndef PS3_GAME_ORBIT_FIX32
        if(mesh.jfx){
            const auto slot=i%6;
            if(part_i!=(slot<2 ? 0 : slot-1) || plan.packets[i].plastic_pass!=(slot<2 ? slot+1 : 0)){
                last_error_="JFX requires the original five ranges and both plastic passes";return false;
            }
        }
#endif
        const auto& gpu=gpu_mesh_[part_i];
        if(plan.packets[i].surface!=gpu.surface || !gpu.vertices ||
           !gpu.indices || !gpu.index_count){
            last_error_="FIX28 draw packet does not match the frozen GPU mesh"; return false;
        }
    }
#ifdef PS3_GAME_ORBIT_FIX32
    const bool verbose=diagnostic_frame_number_<2;
#else
    const bool verbose=diagnostic_frame_number_<3 || diagnostic_frame_number_%60==0;
#endif
    const auto command_start=reinterpret_cast<std::uintptr_t>(static_cast<void*>(ctx->current));
    const auto command_begin=reinterpret_cast<std::uintptr_t>(static_cast<void*>(ctx->begin));
    const auto command_end=reinterpret_cast<std::uintptr_t>(static_cast<void*>(ctx->end));
    if(command_start<command_begin || command_start>command_end ||
       command_end-command_start<
#ifdef PS3_GAME_ORBIT_FIX31
           OrbitFlowFix31::InitialFrameGuardBytes
#else
           CaseRenderFix25::InitialFrameGuardBytes
#endif
       ){
        last_error_="FIX28 frame has insufficient command space"; return false;
    }
    const char* tag=diagnostic_view_label_.empty()
        ? (covers_.empty() ? "GRAY_CASE" : "TEXTURED_CASE")
        : diagnostic_view_label_.c_str();
    if(verbose) RuntimeDiag::log("FRAME %s 00: begin; packets=%u buffer=%u current=%p end=%p remaining=%u",
                     tag,unsigned(plan.packets.size()),unsigned(current_buffer_),
                     static_cast<void*>(ctx->current),static_cast<void*>(ctx->end),unsigned(command_end-command_start));
#endif
    gcmResetFlipStatus();

    gcmSurface sf{};
    sf.colorFormat=GCM_SURFACE_X8R8G8B8;
    sf.colorTarget=GCM_SURFACE_TARGET_0;
    sf.colorLocation[0]=GCM_LOCATION_RSX;
    sf.colorOffset[0]=color_offset_[current_buffer_];
    sf.colorPitch[0]=color_pitch_;
    sf.colorLocation[1]=sf.colorLocation[2]=sf.colorLocation[3]=GCM_LOCATION_RSX;
    sf.colorOffset[1]=sf.colorOffset[2]=sf.colorOffset[3]=0;
    sf.colorPitch[1]=sf.colorPitch[2]=sf.colorPitch[3]=64;
    sf.depthFormat=GCM_SURFACE_ZETA_Z24S8;
    sf.depthLocation=GCM_LOCATION_RSX;
    sf.depthOffset=depth_offset_;
    sf.depthPitch=depth_pitch_;
    sf.type=GCM_SURFACE_TYPE_LINEAR;
    sf.antiAlias=GCM_SURFACE_CENTER_1;
    sf.width=width_; sf.height=height_; sf.x=0; sf.y=0;

    rsxSetSurface(ctx,&sf);
#ifdef PS3_SP_LOADER_FIX28
    if(verbose) RuntimeDiag::log("FRAME %s 01: surface returned",tag);
#endif
    const f32 viewport_scale[4]={width_*0.5f,height_*-0.5f,0.5f,0.0f};
    const f32 viewport_offset[4]={width_*0.5f,height_*0.5f,0.5f,0.0f};
    rsxSetViewport(ctx,0,0,(u16)width_,(u16)height_,0.0f,1.0f,viewport_scale,viewport_offset);
    rsxSetScissor(ctx,0,0,(u16)width_,(u16)height_);
    rsxSetColorMask(ctx,GCM_COLOR_MASK_R|GCM_COLOR_MASK_G|GCM_COLOR_MASK_B|GCM_COLOR_MASK_A);
    rsxSetDepthTestEnable(ctx,GCM_TRUE);
    rsxSetDepthFunc(ctx,GCM_LEQUAL);
    rsxSetDepthWriteEnable(ctx,GCM_TRUE);
    rsxSetFrontFace(ctx,front_face_clockwise_ ? GCM_FRONTFACE_CW : GCM_FRONTFACE_CCW);
    rsxSetCullFace(ctx,GCM_CULL_BACK);
    rsxSetCullFaceEnable(ctx,GCM_TRUE);
    rsxSetShadeModel(ctx,GCM_SHADE_MODEL_SMOOTH);
    rsxSetBlendEnable(ctx,GCM_TRUE);
    rsxSetBlendFunc(ctx,GCM_SRC_ALPHA,GCM_ONE_MINUS_SRC_ALPHA,GCM_SRC_ALPHA,GCM_ONE_MINUS_SRC_ALPHA);
    rsxSetBlendEquation(ctx,GCM_FUNC_ADD,GCM_FUNC_ADD);

#ifdef PS3_SP_LOADER_FIX28
    rsxSetClearColor(ctx,mesh.jfx ? JfxCaseFix29::Background : CaseRenderFix25::Background);
#else
    rsxSetClearColor(ctx,0x101418ff);
#endif
    rsxSetClearDepthStencil(ctx,0xffffff00);
    rsxClearSurface(ctx,GCM_CLEAR_R|GCM_CLEAR_G|GCM_CLEAR_B|GCM_CLEAR_A|GCM_CLEAR_Z);
#ifdef PS3_SP_LOADER_FIX28
    if(verbose) RuntimeDiag::log("FRAME %s 02: state/color+Z clear returned",tag);
#endif

    auto* vp=(const rsxVertexProgram*)vertex_program_;
    auto* fp=(const rsxFragmentProgram*)fragment_program_;
    rsxLoadVertexProgram(ctx,vp,vertex_ucode_);
#ifdef PS3_SP_LOADER_FIX28
    if(verbose) RuntimeDiag::log("FRAME %s 03: vertex shader loaded; fragment program loads follow each constant update",tag);
#endif

    const float shell_color[4]={0.773f,0.788f,0.804f,0.72f}; // approved V14 translucent light gray
    last_stats_.textured_draw_calls=0;
    last_stats_.shell_draw_calls=0;
    last_stats_.background_draw_calls=0;
#ifdef PS3_GAME_ORBIT_FIX36
    last_stats_.wave_draw_calls=0;
#endif
#ifdef PS3_GAME_ORBIT_FIX30
    if(background_texture_.uploaded){
        if(!draw_orbit_background_ps3()) return false;
        last_stats_.background_draw_calls=1;
        rsxSetDepthTestEnable(ctx,GCM_TRUE);
        rsxSetDepthWriteEnable(ctx,GCM_TRUE);
        rsxSetCullFaceEnable(ctx,GCM_TRUE);
    }
#endif

    const auto order=native_submission_order(plan,mesh);
    for(const auto packet_i:order){
        const auto& packet=plan.packets[packet_i];
        const auto part_i=native_packet_part(packet,packet_i,mesh);
        const auto& gp=gpu_mesh_[part_i];
        const auto& part=mesh.parts[part_i];
#ifdef PS3_SP_LOADER_FIX28
        const auto packet_current=reinterpret_cast<std::uintptr_t>(static_cast<void*>(ctx->current));
        if(packet_current<command_start || packet_current>command_end || command_end-packet_current<2048u){
            last_error_="FIX28 packet has insufficient command space"; return false;
        }
        if(packet.surface!=gp.surface || !gp.vertices || !gp.indices || !gp.index_count){
            last_error_="FIX28 draw packet does not match the frozen GPU mesh"; return false;
        }
#endif
#ifdef PS3_GAME_ORBIT_FIX33
        const GpuTextureStage1* tex=texture_for_art_role(part.texture_role,packet.game_index);
#else
        const GpuTextureStage1* tex=texture_for_game(packet.game_index);
#ifdef PS3_GAME_ORBIT_FIX32
        if(part.texture_role==V14MeshPart::TextureRole::DiscLabel) tex=&disc_label_texture_;
        else if(part.texture_role==V14MeshPart::TextureRole::DiscBack) tex=&disc_back_texture_;
#endif
#endif
        const bool use_tex=packet.mesh_textured &&
#ifdef PS3_GAME_ORBIT_FIX33
            (part.texture_role!=V14MeshPart::TextureRole::Cover ? tex && tex->uploaded :
                surface_uses_texture(packet.surface,tex));
#else
            surface_uses_texture(packet.surface,tex);
#endif
#ifdef PS3_SP_LOADER_FIX28
        if(verbose) RuntimeDiag::log("FRAME %s packet.%u: surface=%u vertices=%u indices=%u use_texture=%u",
                         tag,unsigned(packet_i),unsigned(packet.surface),unsigned(gp.vertex_count),
                         unsigned(gp.index_count),unsigned(use_tex));
#endif

        const auto mvp_rows=mat4_shader_rows(packet.mvp);
        const auto model_rows=mat4_shader_rows(packet.model);
        rsxSetVertexProgramParameter(ctx,vp,(const rsxProgramConst*)vp_mvp_,mvp_rows.data());
        rsxSetVertexProgramParameter(ctx,vp,(const rsxProgramConst*)vp_model_,model_rows.data());
        const V10UvTransform uvx=
#ifdef PS3_GAME_ORBIT_FIX32
            part.texture_role!=V14MeshPart::TextureRole::Cover ? V10UvTransform{} :
#endif
            compute_native_uv_transform(mesh,packet.surface,tex && tex->full_cover);
        const float uv_transform[4]={uvx.scale_u,uvx.scale_v,uvx.bias_u,uvx.bias_v};
        rsxSetVertexProgramParameter(ctx,vp,(const rsxProgramConst*)vp_uv_transform_,uv_transform);
        // Covers are opaque; the shell keeps its approved alpha and does not
        // overwrite depth. Hidden reverse faces are culled using repaired indices.
        const bool shell=mesh.jfx ? (part.material==V14MeshPart::Material::ClearPlastic
#ifdef PS3_GAME_ORBIT_FIX32
            || part.material==V14MeshPart::Material::DiscGlass
#endif
        ) : !packet.mesh_textured;
#ifdef PS3_GAME_ORBIT_FIX31
        const bool translucent=shell || packet.visibility<.99999f;
#else
        const bool translucent=shell;
#endif
        rsxSetCullFace(ctx,packet.plastic_pass==1 ? GCM_CULL_FRONT : GCM_CULL_BACK);
        rsxSetBlendEnable(ctx,translucent ? GCM_TRUE : GCM_FALSE);
        rsxSetDepthWriteEnable(ctx,translucent ? GCM_FALSE : GCM_TRUE);
        const float use_texture[4]={use_tex?1.0f:0.0f,0,0,0};
        const float alpha[4]={
#ifdef PS3_GAME_ORBIT_FIX31
            (shell ? packet.alpha : 1.0f)*packet.visibility,
#else
            packet.alpha,
#endif
            0,0,0};
        const float* base_color=mesh.jfx ? part.color.data() : shell_color;
#ifdef PS3_GAME_ORBIT_FIX33
        const float artwork_color[4]={1,1,1,1};
        if(use_tex && part.texture_role==V14MeshPart::TextureRole::Inside) base_color=artwork_color;
#endif
        rsxSetFragmentProgramParameter(ctx,fp,(const rsxProgramConst*)fp_base_color_,base_color,fragment_offset_,GCM_LOCATION_RSX);
        rsxSetFragmentProgramParameter(ctx,fp,(const rsxProgramConst*)fp_use_texture_,use_texture,fragment_offset_,GCM_LOCATION_RSX);
        rsxSetFragmentProgramParameter(ctx,fp,(const rsxProgramConst*)fp_alpha_,alpha,fragment_offset_,GCM_LOCATION_RSX);
        // Refresh FP_ADDRESS after its embedded constants have been transferred,
        // including the useTexture change between plastic and cover surfaces.
        rsxLoadFragmentProgramLocation(ctx,fp,fragment_offset_,GCM_LOCATION_RSX);

        if(use_tex) setup_texture_ps3(*tex);
        else rsxTextureControl(ctx,0,GCM_FALSE,0,0,GCM_TEXTURE_MAX_ANISO_1);

        const u8 stride=(u8)sizeof(V14Vertex);
        rsxBindVertexArrayAttrib(ctx,(u8)attr_position_,0,gp.vertex_offset+(u32)offsetof(V14Vertex,x),stride,3,GCM_VERTEX_DATA_TYPE_F32,GCM_LOCATION_RSX);
        rsxBindVertexArrayAttrib(ctx,(u8)attr_normal_,0,gp.vertex_offset+(u32)offsetof(V14Vertex,nx),stride,3,GCM_VERTEX_DATA_TYPE_F32,GCM_LOCATION_RSX);
        rsxBindVertexArrayAttrib(ctx,(u8)attr_uv_,0,gp.vertex_offset+(u32)offsetof(V14Vertex,u),stride,2,GCM_VERTEX_DATA_TYPE_F32,GCM_LOCATION_RSX);
        rsxDrawIndexArray(ctx,GCM_TYPE_TRIANGLES,gp.index_offset,gp.index_count,GCM_INDEX_TYPE_16B,GCM_LOCATION_RSX);
        if(use_tex) ++last_stats_.textured_draw_calls;
        else ++last_stats_.shell_draw_calls;
#ifdef PS3_SP_LOADER_FIX28
        if(verbose) RuntimeDiag::log("FRAME %s packet.%u: draw returned; current=%p",tag,unsigned(packet_i),static_cast<void*>(ctx->current));
#endif
    }

#ifdef PS3_SP_LOADER_FIX28
    last_stats_.hud_draw_calls=0;
    if(!draw_library_hud_ps3()) return false;
    last_stats_.hud_draw_calls=hud_texture_.uploaded ? 1 : 0;
    if(verbose && hud_texture_.uploaded) RuntimeDiag::log("HUD: one indexed draw; texture=%dx%d indices=%u",hud_texture_.width,hud_texture_.height,unsigned(hud_mesh_.index_count));
    const auto before_flip=reinterpret_cast<std::uintptr_t>(static_cast<void*>(ctx->current));
    if(before_flip>command_end || command_end-before_flip<128u){ last_error_="FIX28 flip has insufficient command space"; return false; }
    if(before_flip-command_start+128u>LibraryPairFix28::FrameCommandBudgetBytes){
        last_error_="FIX28 frame exceeds its bounded command budget"; return false;
    }
    if(verbose) RuntimeDiag::log("FRAME %s 10: gcmSetFlip begin; buffer=%u commands_before_flip=%u",tag,unsigned(current_buffer_),unsigned(before_flip-command_start));
#endif
#ifdef PS3_GAME_ORBIT_FIX30
    if(diagnostic_view_label_=="CULL_FRONT"){
        // Target is buffer 0; warmup leaves buffer 1 on screen. The caller's
        // GET/REF/backend-label acknowledgement precedes CPU sample reads.
        rsxFlushBuffer(ctx);
        RuntimeDiag::log("ORBIT CALIBRATION: submitted offscreen; target=%u no flip",unsigned(current_buffer_));
        return true;
    }
#endif
    const s32 flip_rc=gcmSetFlip(ctx,current_buffer_);
#ifdef PS3_SP_LOADER_FIX28
    if(verbose) RuntimeDiag::log("FRAME %s 11: gcmSetFlip returned; rc=%d",tag,int(flip_rc));
#endif
    if(flip_rc!=0){ last_error_="Full-case gcmSetFlip failed"; return false; }
    rsxFlushBuffer(ctx);
#ifdef PS3_SP_LOADER_FIX28
    if(verbose) RuntimeDiag::log("FRAME %s 12: flush returned",tag);
#endif
    if(!wait_for_flip_polling_ps3("FRAME FLIP",10000u)) return false;
#ifdef PS3_SP_LOADER_FIX28
    const auto* pixels=static_cast<volatile u32*>(color_buffer_[current_buffer_]);
    const std::size_t pixel_count=std::size_t(color_pitch_)*height_/sizeof(u32);
    if(verbose) RuntimeDiag::log("FRAME %s 13: flip completed; buffer=%u first=0x%08x center=0x%08x last=0x%08x textured=%u untextured=%u command_bytes=%u",
                     tag,unsigned(current_buffer_),pixels[0],pixels[std::size_t(height_/2)*(color_pitch_/sizeof(u32))+width_/2],pixels[pixel_count-1],
                     unsigned(last_stats_.textured_draw_calls),unsigned(last_stats_.shell_draw_calls),
                     unsigned(reinterpret_cast<std::uintptr_t>(static_cast<void*>(ctx->current))-command_start));
    if(verbose){
    // Sample actual pixels after the completed flip; draw counts alone cannot
    // establish texture visibility. Sparse reads avoid copying the framebuffer.
    u32 grid_hash=2166136261u;
    unsigned blue_samples=0;
    for(int gy=0;gy<17;++gy) for(int gx=0;gx<17;++gx){
        const int x=gx*(width_-1)/16;
        const int y=gy*(height_-1)/16;
        const u32 value=pixels[std::size_t(y)*(color_pitch_/sizeof(u32))+x];
        if((value&0x00ffffffu)==((mesh.jfx ? JfxCaseFix29::Background : CaseRenderFix25::Background)&0x00ffffffu)) ++blue_samples;
        grid_hash=(grid_hash^value)*16777619u;
    }
    if(verbose) RuntimeDiag::log("PIXEL_GRID %s: samples=289 background=%u other=%u hash=0x%08x",
                     tag,blue_samples,289u-blue_samples,grid_hash);
    // Locate nine points on the surface facing the camera in this view.
    // Interpolate all three coordinates: the spine varies along Z as well as Y.
    for(std::size_t i=0;i<plan.packets.size();++i){
        if(plan.packets[i].surface!=diagnostic_sample_surface_) continue;
        const auto& surface_part=mesh.parts[native_packet_part(plan.packets[i],i,mesh)];
        if(surface_part.vertices.size()<4) continue;
        const auto& m=plan.packets[i].mvp.m;
        for(int row=0;row<3;++row) for(int col=0;col<3;++col){
            const float u=0.25f+0.25f*col, v=0.25f+0.25f*row;
            std::array<float,3> point;
            if(mesh.jfx){if(!JfxCaseFix29::surface_point(surface_part,u,v,point)) continue;}
            else point=full_cover_surface_point_fix26(surface_part,u,v);
            const float x=point[0],y=point[1],z=point[2];
            const float cx=m[0]*x+m[4]*y+m[8]*z+m[12];
            const float cy=m[1]*x+m[5]*y+m[9]*z+m[13];
            const float cw=m[3]*x+m[7]*y+m[11]*z+m[15];
            if(cw<=0.0f) continue;
            const int px=static_cast<int>((cx/cw+1.0f)*width_*0.5f);
            const int py=static_cast<int>((1.0f-cy/cw)*height_*0.5f);
            if(px<0 || py<0 || px>=width_ || py>=height_) continue;
            if(verbose) RuntimeDiag::log("SURFACE_PIXEL %s: surface=%u grid=%d,%d xy=%d,%d argb=0x%08x",
                             tag,unsigned(diagnostic_sample_surface_),col,row,px,py,
                             pixels[std::size_t(py)*(color_pitch_/sizeof(u32))+px]);
        }
        break;
    }
    RsxPresentFix20::log_display(tag);
    RsxPresentFix20::log_buffers(tag,color_offset_,width_,height_,color_pitch_);
    }
    ++diagnostic_frame_number_;
#endif
    current_buffer_^=1;
    return true;
}
#endif
