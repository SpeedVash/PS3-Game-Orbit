#include <bits/stdc++.h>
#define private public
#include "rsx_renderer_v10.h"
#undef private
#include "library_browser_fix28.h"
#include "orbit_flow_fix31.h"
#include "layout_settings_fix31.h"
#include "case_animation_fix32.h"
#include "jfx_case_fix29.h"
#include "orbit_ui_fix30.h"
#include "image_decode.h"

static std::vector<GameEntry> catalog(int n,const std::string& cover="") {
    std::vector<GameEntry> games(n);
    for(int i=0;i<n;++i) {
        games[i].path="GAME"+std::to_string(i);games[i].title="Jogo "+std::to_string(i+1);
        games[i].cover_path=cover;games[i].cover_kind=GameCoverKind::FrontOnly;
    }
    return games;
}
static InputFrame idle(){InputFrame p;p.connected=true;return p;}
static void settle(LibraryBrowserFix28& b,int frames=150){for(int i=0;i<frames;++i)b.update(idle(),1.0f/60);}
#ifndef PS3_GAME_ORBIT_FLOW_ONLY
static double area(const V14MeshPart& p) {
    double result=0;
    for(std::size_t i=0;i<p.indices.size();i+=3) {
        const auto& a=p.vertices[p.indices[i]];const auto& b=p.vertices[p.indices[i+1]];const auto& c=p.vertices[p.indices[i+2]];
        double u[3]={b.x-a.x,b.y-a.y,b.z-a.z},v[3]={c.x-a.x,c.y-a.y,c.z-a.z};
        double x=u[1]*v[2]-u[2]*v[1],y=u[2]*v[0]-u[0]*v[2],z=u[0]*v[1]-u[1]*v[0];
        result+=std::sqrt(x*x+y*y+z*z)*.5;
    }
    return result;
}
#endif
int main(int argc,char** argv) {
    assert(argc==2);std::setvbuf(stdout,nullptr,_IONBF,0);const std::string root=argv[1];
    LibraryBrowserFix28 b("");b.replace_catalog(catalog(57));
    auto p=idle();p.square.pressed=true;b.update(p,0);assert(b.state().layout==OrbitLayout::Spine);settle(b);
    b.update(p,0);assert(b.state().layout==OrbitLayout::List);settle(b);
    auto poses=OrbitFlowFix31::poses(b.state());assert(poses.size()==1 && poses[0].x==80 && poses[0].selected);
    p=idle();p.down.pressed=true;b.update(p,0);assert(b.state().selected==1 && !b.automatic());
    p=idle();p.up.pressed=true;b.update(p,0);assert(b.state().selected==0 && !b.automatic());
    p=idle();p.left_y=1;b.update(p,.1f);assert(b.state().selected==1);
    settle(b);p=idle();p.square.pressed=true;b.update(p,0);assert(b.state().layout==OrbitLayout::Classic);settle(b);
    puts("PASS: all three layouts, nine-row list navigation with D-pad/vertical analog and minimum-size selected case on the right");

    b.restore_layout(OrbitLayout::List);b.restore_selection(FilterMode::All,"GAME28");
    const auto selected=b.state().selected;const auto yaw=b.state().center_yaw_deg;
    p=idle();p.l3.pressed=true;b.update(p,0);assert(b.state().inspection_target && b.state().inspection_phase==0);
    for(int frame=0;frame<86;++frame) {
        p=idle();p.right.pressed=p.down.pressed=p.square.pressed=p.start.pressed=p.triangle.pressed=p.r1.pressed=true;
        auto command=b.update(p,1.0f/60);
        assert(b.state().selected==selected && b.state().layout==OrbitLayout::List && b.state().filter==FilterMode::All);
        assert(!command.mount_selected && !command.rescan_library && !command.request_exit);
    }
    assert(b.state().inspection_phase==1 && !b.automatic());
    poses=OrbitFlowFix31::poses(b.state());assert(poses.size()==1 && poses[0].inspection_phase==1);
    p=idle();p.cross.pressed=true;auto command=b.update(p,0);assert(!command.mount_selected && !b.state().inspection_target);
    settle(b);assert(b.state().inspection_phase==0 && b.state().center_yaw_deg==yaw);
    p=idle();p.l3.pressed=true;b.update(p,0);settle(b,40);const float partial=b.state().inspection_phase;
    b.update(p,0);assert(!b.state().inspection_target && b.state().inspection_phase==partial);
    settle(b,10);const float reversed=b.state().inspection_phase;assert(reversed<partial);
    b.update(p,0);assert(b.state().inspection_target && b.state().inspection_phase==reversed);
    p=idle();p.circle.pressed=true;command=b.update(p,0);assert(!command.request_exit);settle(b);
    p=idle();p.l3.pressed=p.circle.pressed=true;b.update(p,0);assert(b.state().inspection_phase==0 && b.state().center_yaw_deg==yaw);
    p=idle();p.cross.pressed=true;assert(b.update(p,0).mount_selected);
    for(int n:{0,1,2,8}) {LibraryBrowserFix28 small("");small.replace_catalog(catalog(n));small.restore_layout(OrbitLayout::List);
        assert(OrbitFlowFix31::poses(small.render_state()).size()==1);}
    puts("PASS: L3 opening/reversal, X/Circle visual return, browse/mount/rescan/filter locks during inspection and safe empty/short lists");

#ifndef PS3_GAME_ORBIT_FLOW_ONLY
    const auto original=JfxCaseFix29::build();const auto mesh=CaseAnimationFix32::build(original);
    assert(original.parts.size()==5 && mesh.parts.size()>original.parts.size());double closed_area=0,split_area=0;
    for(std::size_t i=0;i<5;++i) {const auto& a=original.parts[i];const auto& c=mesh.parts[i];
        assert(a.indices==c.indices && a.vertices.size()==c.vertices.size());
        assert(!std::memcmp(a.vertices.data(),c.vertices.data(),a.vertices.size()*sizeof(V14Vertex)));closed_area+=area(a);}
    unsigned disc_triangles=0;
    for(std::size_t i=5;i<mesh.parts.size();++i) {const auto& part=mesh.parts[i];
        assert(part.vertices.size()<=65535 && part.indices.size()%3==0);
        for(auto index:part.indices) assert(index<part.vertices.size());
        for(const auto& v:part.vertices) {
            const double length=std::sqrt(v.nx*v.nx+v.ny*v.ny+v.nz*v.nz);assert(std::isfinite(length) && std::abs(length-1)<.002);
            assert(std::isfinite(v.u) && std::isfinite(v.v));
        }
        if(part.texture_role==V14MeshPart::TextureRole::Cover) split_area+=area(part);
        if(part.joint==V14MeshPart::Joint::Disc) disc_triangles+=part.indices.size()/3;
    }
    assert(std::abs(split_area-closed_area)<closed_area*.00001 && disc_triangles==1056);
    for(float phase:{0.f,.2f,.4f,.65f,.76f,1.f}) {
        const auto h=CaseAnimationFix32::joint_transform(V14MeshPart::Joint::Lid,phase);
        assert(std::abs(h.m[0]*CaseAnimationFix32::HingeX+h.m[12]-CaseAnimationFix32::HingeX)<.00001);
        assert(std::abs(h.m[2]*CaseAnimationFix32::HingeX+h.m[14])<.00001);
    }
    assert(CaseAnimationFix32::disc_slide(.76f)==0 && CaseAnimationFix32::lid_angle(.65f)==-115);
    for(float phase=0;phase<=1.01f;phase+=.05f) {
        CasePose pose;pose.game_index=0;pose.selected=true;pose.scale=.55f;pose.z=72;pose.inspection_phase=std::min(phase,1.f);
        auto plan=build_v10_frame_plan({pose},mesh);assert(validate_native_plan_fix32(plan,mesh));
        for(const auto& packet:plan.packets) assert((mesh.parts[packet.mesh_part].joint!=V14MeshPart::Joint::Closed)==(pose.inspection_phase>.2f));
        auto bad=plan;bad.packets.pop_back();assert(!validate_native_plan_fix32(bad,mesh));
        bad=plan;bad.packets.push_back(plan.packets[0]);assert(!validate_native_plan_fix32(bad,mesh));
        bad=plan;bad.packets[0].plastic_pass=0;assert(!validate_native_plan_fix32(bad,mesh));
    }
    puts("PASS: closed mesh preserved byte-for-byte; articulated halves preserve surface area/UVs; fixed hinge, original 1,056-triangle disc and complete native packet validation");
#else
    const auto mesh=build_v14_case_mesh();
    puts("PASS: public subset uses the legacy test mesh; no licensed JFX geometry needed");
#endif

    RsxStage1 stage;assert(stage.init());RsxRendererV10 r;assert(r.init(stage,mesh,false));
    CoverCache cache(32,16u<<20);LibraryBrowserFix28 textures("");
    textures.replace_catalog(catalog(57,root+"/tests/assets/cache32_1024.png"));textures.restore_layout(OrbitLayout::List);
    assert(r.sync_visible_covers(textures.render_state(),cache,1));const auto* first=r.covers_.at(0).texture.gpu_ptr;
    textures.restore_selection(FilterMode::All,"GAME1");assert(r.sync_visible_covers(textures.render_state(),cache,1));
    const auto misses=r.gpu_cache_misses();textures.restore_selection(FilterMode::All,"GAME0");assert(r.sync_visible_covers(textures.render_state(),cache,1));
    assert(r.gpu_cache_misses()==misses && r.gpu_cache_hits()>0 && r.covers_.at(0).texture.gpu_ptr==first);
    for(int i=2;i<57;++i) {textures.restore_selection(FilterMode::All,"GAME"+std::to_string(i));
        assert(r.sync_visible_covers(textures.render_state(),cache,1) && r.has_cover_texture(i));
        assert(r.gpu_cache_bytes()<=r.GpuCoverBudgetBytes && r.gpu_cover_count()<=r.MaxResidentCovers);
    }
    assert(r.gpu_cache_evictions()>0 && r.gpu_cover_count()==16);
    textures.restore_layout(OrbitLayout::Spine);assert(r.sync_visible_covers(textures.render_state(),cache,1));
    std::map<int,void*> pinned;for(const auto& pose:OrbitFlowFix31::poses(textures.state()))pinned[pose.game_index]=r.covers_.at(pose.game_index).texture.gpu_ptr;
    for(int i=0;i<10;++i)r.prefetch_nearby_cover(textures.state(),cache);
    for(const auto& kv:pinned)assert(r.covers_.at(kv.first).texture.gpu_ptr==kv.second);
    assert(r.gpu_cache_bytes()<=r.GpuCoverBudgetBytes);
    r.clear_cover_textures();assert(r.gpu_cache_bytes()==0 && r.gpu_cover_count()==0);
    textures.replace_catalog(catalog(57,root+"/tests/assets/full_cover_275x147.png"));textures.restore_layout(OrbitLayout::List);
    for(int i=0;i<57;++i) {
        textures.restore_selection(FilterMode::All,"GAME"+std::to_string(i));
        assert(r.sync_visible_covers(textures.render_state(),cache,1));
    }
    assert(r.gpu_cover_count()==32 && r.gpu_cache_bytes()<r.GpuCoverBudgetBytes);
    puts("PASS: 57 small textures enforce the separate 32-cover LRU limit without approaching the byte budget");
    r.clear_cover_textures();
    auto invalid=catalog(1,root+"/tests/assets/missing.png");textures.replace_catalog(invalid);
    assert(r.sync_visible_covers(textures.render_state(),cache,1));const auto failed=r.gpu_cache_misses();
    assert(r.sync_visible_covers(textures.render_state(),cache,1) && r.gpu_cache_misses()==failed && !r.has_cover_texture(0));
    puts("PASS: actual GPU pointer reuse and LRU eviction over 57 full-size textures; 64 MiB bound, visible Spine surfaces pinned during prefetch, and failed-artwork retry suppression");

    textures.replace_catalog(catalog(57));textures.restore_layout(OrbitLayout::List);
    assert(r.set_library_hud(textures.hud_lines(""),&textures.state()));
    const auto* hudptr=r.hud_texture_.gpu_ptr;const auto oldrows=r.hud_rows_;assert(oldrows.size()==9 && r.hud_active_row_==0);
    textures.restore_selection(FilterMode::All,"GAME56");assert(r.set_library_hud(textures.hud_lines(""),&textures.state()));
    assert(r.hud_texture_.gpu_ptr==hudptr && r.hud_rows_!=oldrows && r.hud_active_row_==8);
    auto rgba=OrbitUiFix30::hud(textures.hud_lines(""),r.hud_rows_,r.hud_active_row_);
    const auto* argb=static_cast<const unsigned char*>(hudptr);const auto k=std::size_t(186)*rgba.pitch+90*4;
    const auto j=std::size_t(186)*r.hud_texture_.pitch+90*4;
    assert(argb[j]==rgba.rgba[k+3] && argb[j+1]==rgba.rgba[k] && argb[j+2]==rgba.rgba[k+1] && argb[j+3]==rgba.rgba[k+2]);
    auto tmp=std::filesystem::temp_directory_path()/"orbit32-layout.dat";LayoutSettingsFix31 saved;saved.capture(OrbitLayout::List);assert(saved.save(tmp.string()));
    LayoutSettingsFix31 loaded;assert(loaded.load(tmp.string()) && loaded.layout()==OrbitLayout::List);std::filesystem::remove(tmp);
    r.shutdown();stage.shutdown();
    puts("PASS: unchanged HUD allocation with updated nine-row window/highlight, exact ARGB bytes and persistent List layout");
}
