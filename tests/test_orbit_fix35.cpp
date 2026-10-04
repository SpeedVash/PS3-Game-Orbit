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
#include "cover_resolver.h"
#include "game_tools_fix35.h"
#include "performance_fix35.h"
#include "runtime_diag.h"

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
    p=idle();p.cross.pressed=true;auto command=b.update(p,0);assert(command.mount_selected && b.state().inspection_target);
    b.set_operation_status("Montando",true);p=idle();p.cross.pressed=p.l3.pressed=true;
    command=b.update(p,0);assert(!command.mount_selected && b.state().inspection_target);
    p=idle();p.circle.pressed=true;command=b.update(p,0);assert(command.request_exit && b.state().inspection_target);
    b.set_operation_status("",false);p=idle();p.circle.pressed=p.cross.pressed=true;
    command=b.update(p,0);assert(!command.mount_selected && !command.request_exit && !b.state().inspection_target);
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
    puts("PASS: L3 opening/reversal, X mounts while open, Circle closes or cancels busy mount, browse/rescan/filter locks during inspection and safe empty/short lists");

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

    unsigned inside_parts=0;
    for(const auto& part:mesh.parts)if(part.texture_role==V14MeshPart::TextureRole::Inside){
        ++inside_parts;assert(part.textured && part.vertices.size()==4);
        const bool strip=part.vertices[0].x==CaseAnimationFix32::HingeX;
        const bool lid=part.joint==V14MeshPart::Joint::Lid;
        for(const auto& v:part.vertices){
            assert(v.v==(v.y<0 ? 1.f : 0.f));
            if(strip){assert(v.u>=JfxCaseFix29::BackEnd && v.u<=JfxCaseFix29::FrontBegin);assert(v.x==CaseAnimationFix32::HingeX ? v.u==.5f : true);}
            else if(lid){assert(v.u<=JfxCaseFix29::BackEnd);assert(v.u==(v.x<0 ? JfxCaseFix29::BackEnd : 0.f));}
            else {assert(v.u>=JfxCaseFix29::FrontBegin);assert(v.u==(v.x<0 ? JfxCaseFix29::FrontBegin : 1.f));}
        }
    }
    assert(inside_parts==4);
    unsigned disc_parts=0;float minimum_radius=100;
    for(const auto& part:mesh.parts)if(part.joint==V14MeshPart::Joint::Disc){
        ++disc_parts;assert(part.material!=V14MeshPart::Material::DiscGlass && part.color[3]==1);
        for(const auto& v:part.vertices)minimum_radius=std::min(minimum_radius,std::sqrt(v.x*v.x+v.y*v.y));
    }
    assert(disc_parts==7 && std::abs(minimum_radius-7.5f)<.001f);
    puts("PASS: all seven disc surfaces are opaque; 15 mm physical center hole and approved 1,056 triangles are preserved");
    puts("PASS: Inside Cover Full left/center/right atlas, reversed lid U, upright vertical UVs and both center strips");

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
    CoverCache cache(15,16u<<20);LibraryBrowserFix28 textures("");
    textures.replace_catalog(catalog(57,root+"/tests/assets/cache32_1024.png"));textures.restore_layout(OrbitLayout::List);
    assert(r.sync_visible_covers(textures.render_state(),cache,1));const auto* first=r.covers_.at(0).texture.gpu_ptr;
    textures.restore_selection(FilterMode::All,"GAME1");assert(r.sync_visible_covers(textures.render_state(),cache,1));
    const auto misses=r.gpu_cache_misses();textures.restore_selection(FilterMode::All,"GAME0");assert(r.sync_visible_covers(textures.render_state(),cache,1));
    assert(r.gpu_cache_misses()==misses && r.gpu_cache_hits()>0 && r.covers_.at(0).texture.gpu_ptr==first);
    for(int i=2;i<57;++i) {textures.restore_selection(FilterMode::All,"GAME"+std::to_string(i));
        assert(r.sync_visible_covers(textures.render_state(),cache,1) && r.has_cover_texture(i));
        assert(r.gpu_cache_bytes()<=r.GpuCoverBudgetBytes && r.gpu_cover_count()<=r.MaxResidentCovers);
    }
    assert(r.gpu_cache_evictions()>0 && r.gpu_cover_count()==15);
    textures.restore_layout(OrbitLayout::Spine);assert(r.sync_visible_covers(textures.render_state(),cache,1));
    std::map<int,void*> pinned;for(const auto& pose:OrbitFlowFix31::poses(textures.state()))pinned[pose.game_index]=r.covers_.at(pose.game_index).texture.gpu_ptr;
    for(int i=0;i<10;++i)r.prefetch_nearby_cover(textures.state(),cache);
    for(const auto& kv:pinned)assert(r.covers_.at(kv.first).texture.gpu_ptr==kv.second);
    assert(r.gpu_cache_bytes()<=r.GpuCoverBudgetBytes && pinned.size()==11 && r.gpu_cover_count()<=15);
    for(int i=0;i<50;++i)r.prefetch_nearby_cover(textures.state(),cache);
    const auto before_prefetch=r.gpu_cache_misses();for(int i=0;i<100;++i)assert(!r.prefetch_nearby_cover(textures.state(),cache));
    assert(r.gpu_cache_misses()==before_prefetch);
    for(int i=0;i<3;++i){p=idle();p.right.pressed=true;textures.update(p,1.0f/60);assert(r.sync_visible_covers(textures.state(),cache,1));}
    assert(OrbitFlowFix31::poses(textures.state()).size()==14 && r.gpu_cover_count()<=15);
    assert(!r.prefetch_nearby_cover(textures.state(),cache));
    for(const auto& pose:OrbitFlowFix31::poses(textures.state()))assert(r.has_cover_texture(pose.game_index));
    settle(textures);assert(r.sync_visible_covers(textures.state(),cache,1) && r.gpu_cover_count()<=15);
    textures.restore_layout(OrbitLayout::List);assert(r.sync_visible_covers(textures.state(),cache,1) && r.gpu_cover_count()==15);
    for(int i=0;i<50;++i)r.prefetch_nearby_cover(textures.state(),cache);
    const auto stable=r.gpu_cache_misses();for(int i=0;i<500;++i)assert(!r.prefetch_nearby_cover(textures.state(),cache));
    assert(r.gpu_cache_misses()==stable && r.gpu_cover_count()<=15);
    puts("PASS: 11 Spine and 14 fading textures stay pinned within 15 slots; idle prefetch reaches a stable set without decode/eviction churn");
    r.clear_cover_textures();assert(r.gpu_cache_bytes()==0 && r.gpu_cover_count()==0);
    textures.replace_catalog(catalog(57,root+"/tests/assets/full_cover_275x147.png"));textures.restore_layout(OrbitLayout::List);
    for(int i=0;i<57;++i) {
        textures.restore_selection(FilterMode::All,"GAME"+std::to_string(i));
        assert(r.sync_visible_covers(textures.render_state(),cache,1));
    }
    assert(r.gpu_cover_count()==15 && r.gpu_cache_bytes()<r.GpuCoverBudgetBytes);
    puts("PASS: 57 small textures enforce the separate 15-cover LRU limit without approaching the byte budget");
    r.clear_cover_textures();
    auto invalid=catalog(1,root+"/tests/assets/missing.png");textures.replace_catalog(invalid);
    assert(r.sync_visible_covers(textures.render_state(),cache,1));const auto failed=r.gpu_cache_misses();
    assert(r.sync_visible_covers(textures.render_state(),cache,1) && r.gpu_cache_misses()==failed && !r.has_cover_texture(0));
    puts("PASS: actual GPU pointer reuse and LRU eviction over 57 full-size textures; 64 MiB bound, visible Spine surfaces pinned during prefetch, and failed-artwork retry suppression");


    const auto scratch=std::filesystem::temp_directory_path()/("orbit35-art-tests-"+std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
    std::filesystem::remove_all(scratch);std::filesystem::create_directories(scratch/"covers");
    std::filesystem::create_directories(scratch/"local/Folder Game");
    const auto makefile=[&](const std::filesystem::path& path){std::filesystem::copy_file(root+"/tests/assets/full_cover_275x147.png",path,std::filesystem::copy_options::overwrite_existing);};
    makefile(scratch/"covers/BLES01287_INSIDE.jpg");makefile(scratch/"covers/BLES01287_DISC.PNG");
    makefile(scratch/"covers/Iso Game_INSIDE.png");makefile(scratch/"local/Folder Game/Folder Game_DISC.jpeg");
    GameEntry named;named.path=(scratch/"local/Iso Game.iso").string();named.title_id="BLES01287";named.format=GameFormat::ISO;
    CoverResolver resolver((scratch/"covers").string());resolver.resolve_inspection_art(named);
    assert(named.inside_cover_path==(scratch/"covers/BLES01287_INSIDE.jpg").string() && named.disc_art_path==(scratch/"covers/BLES01287_DISC.PNG").string());
    assert(resolver.resolve(named).path.empty()); // Optional suffixes never replace the exterior cover.
    makefile(scratch/"covers/BLES01287_INSIDE.png");resolver.resolve_inspection_art(named);
    assert(named.inside_cover_path==(scratch/"covers/BLES01287_INSIDE.png").string());
    named.title_id="";resolver.resolve_inspection_art(named);assert(named.inside_cover_path==(scratch/"covers/Iso Game_INSIDE.png").string() && named.disc_art_path.empty());
    named.path=(scratch/"local/Folder Game").string();named.format=GameFormat::Folder;resolver.resolve_inspection_art(named);
    assert(named.inside_cover_path.empty() && named.disc_art_path==(scratch/"local/Folder Game/Folder Game_DISC.jpeg").string());
    CoverCache encoded(15,16u<<20);
    for(int i=0;i<17;++i){const auto path=scratch/("cover"+std::to_string(i)+".png");makefile(path);GameEntry g;g.cover_path=path.string();g.cover_kind=GameCoverKind::FullCover;assert(encoded.get_or_load(g));}
    assert(encoded.size()==15 && encoded.encoded_bytes()<16u<<20);
    puts("PASS: independent TITLE_ID suffix resolver, extension preference, ISO/folder fallbacks and strict fifteen-entry encoded cache");

    r.clear_cover_textures();
    auto artgames=catalog(57,root+"/tests/assets/cache32_1024.png");
    for(auto& g:artgames){g.inside_cover_path=root+"/assets/examples/INSIDE_COVER_FULL_EXEMPLO.png";g.disc_art_path=root+"/pkgfiles/USRDIR/ORBIT_DISC_LABEL.png";}
    CoverflowState inspection;inspection.games=artgames;inspection.layout=OrbitLayout::List;rebuild_visible(inspection);
    assert(r.sync_visible_covers(inspection,cache,1) && r.sync_inspection_art(inspection));
    assert(r.inside_cache_count()==0 && r.disc_cache_count()==0);
    inspection.inspection_target=true;assert(r.sync_inspection_art(inspection) && r.has_inside_texture(0) && r.has_disc_art_texture(0));
    assert(r.inside_cache_.entries.at(0).texture.full_cover && !r.disc_cache_.entries.at(0).texture.full_cover);
    const auto* insideptr=r.inside_cache_.entries.at(0).texture.gpu_ptr;const auto* discptr=r.disc_cache_.entries.at(0).texture.gpu_ptr;
    assert(r.texture_for_art_role(V14MeshPart::TextureRole::Inside,0)==&r.inside_cache_.entries.at(0).texture);
    assert(!r.texture_for_art_role(V14MeshPart::TextureRole::Inside,1));
    assert(r.texture_for_art_role(V14MeshPart::TextureRole::DiscLabel,1)==&r.disc_label_texture_);
    inspection.inspection_target=false;inspection.selected=1;
    assert(r.sync_inspection_art(inspection) && r.has_inside_texture(0) && r.has_disc_art_texture(0) && !r.has_inside_texture(1));
    const auto imisses=r.inside_cache_misses(),dmisses=r.disc_cache_misses();
    inspection.selected=0;inspection.inspection_target=true;
    for(int i=0;i<50;++i)assert(r.sync_inspection_art(inspection));
    assert(r.inside_cache_misses()==imisses && r.disc_cache_misses()==dmisses);
    assert(r.inside_cache_.entries.at(0).texture.gpu_ptr==insideptr && r.disc_cache_.entries.at(0).texture.gpu_ptr==discptr);
    for(int i=1;i<15;++i){inspection.selected=i;assert(r.sync_visible_covers(inspection,cache,1) && r.sync_inspection_art(inspection));}
    assert(r.gpu_cover_count()==15 && r.inside_cache_count()==15 && r.disc_cache_count()==15);
    inspection.selected=0;assert(r.sync_inspection_art(inspection));
    inspection.selected=15;assert(r.sync_inspection_art(inspection));
    assert(r.has_inside_texture(0) && r.has_disc_art_texture(0) && !r.has_inside_texture(1) && !r.has_disc_art_texture(1));
    assert(r.inside_cache_.entries.at(0).texture.gpu_ptr==insideptr && r.disc_cache_.entries.at(0).texture.gpu_ptr==discptr);
    inspection.selected=16;inspection.games[16].inside_cover_path="";assert(r.sync_inspection_art(inspection));
    assert(r.has_inside_texture(2) && !r.has_disc_art_texture(2) && !r.has_inside_texture(16) && r.has_disc_art_texture(16));
    inspection.selected=17;inspection.games[17].disc_art_path="";assert(r.sync_inspection_art(inspection));
    assert(r.has_inside_texture(17) && !r.has_disc_art_texture(17) && r.disc_cache_count()==15);
    for(int i=18;i<57;++i){inspection.selected=i;assert(r.sync_visible_covers(inspection,cache,1) && r.sync_inspection_art(inspection));
        assert(r.gpu_cover_count()<=15 && r.inside_cache_count()<=15 && r.disc_cache_count()<=15);
        assert(r.gpu_cache_bytes()<=r.GpuCoverBudgetBytes && r.inside_cache_bytes()<=r.InspectionArtBudgetBytes && r.disc_cache_bytes()<=r.InspectionArtBudgetBytes);
    }
    assert(r.gpu_cover_count()==15 && r.inside_cache_count()==15 && r.disc_cache_count()==15 && r.inside_cache_.evictions>0 && r.disc_cache_.evictions>0);
    puts("PASS: three independent 15-image caches over 57 games, real pointer reuse after selection/closing, LRU touches, per-type eviction and byte bounds");
    inspection.selected=24;inspection.games[24].inside_cover_path=(scratch/"late.png").string();inspection.games[24].disc_art_path=(scratch/"bad.png").string();
    std::ofstream(scratch/"bad.png")<<"corrupted optional art";
    assert(r.sync_inspection_art(inspection) && !r.has_inside_texture(24) && !r.has_disc_art_texture(24));
    const auto badim=r.inside_cache_misses(),baddm=r.disc_cache_misses();
    makefile(scratch/"late.png");assert(r.sync_inspection_art(inspection) && !r.has_inside_texture(24));
    assert(r.inside_cache_misses()==badim && r.disc_cache_misses()==baddm);
    r.clear_cover_textures();assert(r.inside_cache_count()==0 && r.disc_cache_count()==0 && r.inside_cache_bytes()==0 && r.disc_cache_bytes()==0);
    assert(r.sync_inspection_art(inspection) && r.has_inside_texture(24) && !r.has_disc_art_texture(24));
    inspection.games[24].path="REPLACED GAME";inspection.games[24].inside_cover_path="";inspection.games[24].disc_art_path="";
    assert(r.sync_inspection_art(inspection) && !r.has_inside_texture(24) && !r.has_disc_art_texture(24));
    r.clear_cover_textures();std::filesystem::remove_all(scratch);
    puts("PASS: missing/corrupt artwork does not occupy cache slots, repeated failures are suppressed, refresh releases all textures, and replaced index/path never binds stale art");

    textures.replace_catalog(catalog(57));textures.restore_layout(OrbitLayout::List);
    assert(r.set_library_hud(textures.hud_lines(""),&textures.state()));
    const auto* hudptr=r.hud_texture_.gpu_ptr;const auto oldrows=r.hud_rows_;assert(oldrows.size()==9 && r.hud_active_row_==0);
    textures.restore_selection(FilterMode::All,"GAME56");assert(r.set_library_hud(textures.hud_lines(""),&textures.state()));
    assert(r.hud_texture_.gpu_ptr==hudptr && r.hud_rows_!=oldrows && r.hud_active_row_==8);
    auto rgba=OrbitUiFix30::hud(textures.hud_lines(""),r.hud_rows_,r.hud_active_row_);
    const auto* argb=static_cast<const unsigned char*>(hudptr);const auto k=std::size_t(186)*rgba.pitch+90*4;
    const auto j=std::size_t(186)*r.hud_texture_.pitch+90*4;
    assert(argb[j]==rgba.rgba[k+3] && argb[j+1]==rgba.rgba[k] && argb[j+2]==rgba.rgba[k+1] && argb[j+3]==rgba.rgba[k+2]);
    auto tmp=std::filesystem::temp_directory_path()/"orbit35-layout.dat";LayoutSettingsFix31 saved;saved.capture(OrbitLayout::List);assert(saved.save(tmp.string()));
    LayoutSettingsFix31 loaded;assert(loaded.load(tmp.string()) && loaded.layout()==OrbitLayout::List);std::filesystem::remove(tmp);
    // Cover reload must invalidate only the selected game in all three caches.
    inspection.selected=0;inspection.games[0].path="GAME0";inspection.games[0].inside_cover_path=root+"/assets/examples/INSIDE_COVER_FULL_EXEMPLO.png";
    inspection.games[0].disc_art_path=root+"/pkgfiles/USRDIR/ORBIT_DISC_LABEL.png";
    r.sync_visible_covers(inspection,cache,1);r.sync_inspection_art(inspection);
    inspection.selected=1;r.sync_visible_covers(inspection,cache,1);r.sync_inspection_art(inspection);
    const auto* retained_cover=r.covers_.at(1).texture.gpu_ptr;
    const auto* retained_inside=r.inside_cache_.entries.at(1).texture.gpu_ptr;
    r.invalidate_game_art(0);
    assert(!r.has_cover_texture(0) && !r.has_inside_texture(0) && !r.has_disc_art_texture(0));
    assert(r.covers_.at(1).texture.gpu_ptr==retained_cover && r.inside_cache_.entries.at(1).texture.gpu_ptr==retained_inside);

    LibraryBrowserFix28 menus("");menus.replace_catalog(catalog(57));settle(menus);
    auto button=idle();button.start.pressed=true;auto menu_cmd=menus.update(button,0);
    assert(menus.state().menu.open && !menu_cmd.rescan_library);
    button=idle();button.right.pressed=button.square.pressed=button.triangle.pressed=true;menus.update(button,0);
    assert(menus.state().selected==0 && menus.state().layout==OrbitLayout::Classic && !menus.state().games[0].favorite);
    button=idle();button.cross.pressed=true;menu_cmd=menus.update(button,0);
    assert(menu_cmd.game_menu_action==GameMenuActionFix35::Rename && !menu_cmd.mount_selected);
    button=idle();button.down.pressed=true;menus.update(button,0);button=idle();button.cross.pressed=true;
    assert(menus.update(button,0).game_menu_action==GameMenuActionFix35::ReloadCovers);
    button=idle();button.down.pressed=true;menus.update(button,0);button=idle();button.cross.pressed=true;
    assert(menus.update(button,0).game_menu_action==GameMenuActionFix35::ImportUsb);
    menus.menu_status("Teste",true);button=idle();button.circle.pressed=button.down.pressed=true;menus.update(button,0);
    assert(menus.state().menu.open && menus.state().menu.selected==2);
    menus.menu_status("");button=idle();button.circle.pressed=true;menu_cmd=menus.update(button,0);
    assert(!menus.state().menu.open && !menu_cmd.request_exit);
    button=idle();button.start.pressed=true;menus.update(button,0);
    for(int i=0;i<3;++i){button=idle();button.down.pressed=true;menus.update(button,0);}
    button=idle();button.cross.pressed=true;menu_cmd=menus.update(button,0);
    assert(menu_cmd.rescan_library && !menus.state().menu.open);
    puts("PASS: START modal actions, navigation/mount/favorite locks, busy keyboard guard and library refresh");

    menus.restore_layout(OrbitLayout::Classic);menus.restore_selection(FilterMode::All,"GAME0");
    assert(r.set_library_hud(menus.hud_lines(""),&menus.state()));
    const auto head=std::vector<unsigned char>(r.hud_cache_.image().rgba.begin(),r.hud_cache_.image().rgba.begin()+1280*64*4);
    menus.restore_selection(FilterMode::All,"GAME1");assert(r.set_library_hud(menus.hud_lines(""),&menus.state()));
    assert(r.hud_cache_.last_changed_bytes()==1280u*112*4 && std::equal(head.begin(),head.end(),r.hud_cache_.image().rgba.begin()));
    assert(r.hud_cache_.image().rgba==OrbitUiFix30::hud(menus.hud_lines("")).rgba);
    menus.restore_layout(OrbitLayout::List);assert(r.set_library_hud(menus.hud_lines(""),&menus.state()));
    menus.restore_selection(FilterMode::All,"GAME2");assert(r.set_library_hud(menus.hud_lines(""),&menus.state()));
    assert(r.hud_cache_.last_changed_bytes()<1280u*512*4);
    assert(r.hud_cache_.image().rgba==OrbitUiFix30::hud(menus.hud_lines(""),r.hud_rows_,r.hud_active_row_).rgba);
    button=idle();button.start.pressed=true;menus.update(button,0);
    assert(r.set_library_hud(menus.hud_lines(""),&menus.state()));HudCacheFix35 fresh;
    fresh.update(menus.hud_lines(""),r.hud_rows_,r.hud_active_row_,menus.state().menu);
    assert(fresh.image().rgba==r.hud_cache_.image().rgba);
    button=idle();button.circle.pressed=true;menus.update(button,0);button=idle();button.select.pressed=true;menus.update(button,0);
    assert(r.set_library_hud(menus.hud_lines(""),&menus.state()));
    for(int y=176;y<512;++y)for(int x=610;x<1216;++x){const auto k=std::size_t(y)*5120+x*4;for(int c=0;c<4;++c)assert(r.hud_cache_.image().rgba[k+c]==0);}
    assert(OrbitPerformanceFix35::metric(OrbitPerformanceFix35::Kind::Read).count>0 && OrbitPerformanceFix35::metric(OrbitPerformanceFix35::Kind::Decode).count>0 && OrbitPerformanceFix35::metric(OrbitPerformanceFix35::Kind::Upload).count>0 && OrbitPerformanceFix35::metric(OrbitPerformanceFix35::Kind::Interface).count>0);
    puts("PASS: partial footer/list repaint matches a full raster, header remains untouched, partial ARGB upload preserves allocation, menu clears correctly, help contains controls only, and four timing counters run");

    const auto tools=std::filesystem::temp_directory_path()/("orbit35-tools-"+std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(tools/"usb0");std::filesystem::create_directories(tools/"usb1");std::filesystem::create_directories(tools/"covers");
    GameToolsFix35::Names overrides;
    const std::string path="/dev_hdd0/PS3ISO/BLES01287 Game.iso",name="Ação — Edição Brasileira";
    assert(overrides.set(path,name) && !overrides.set(path,"   ") && !overrides.set(path,"Linha\nOutra") && !overrides.set(path,std::string(97,'A')));
    const auto wide=GameToolsFix35::utf16(name);assert(GameToolsFix35::utf8(wide.data(),wide.size())==name);
    assert(overrides.save((tools/"names.dat").string()));GameToolsFix35::Names loaded_names;assert(loaded_names.load((tools/"names.dat").string()));
    GameEntry usb_game;usb_game.path=path;usb_game.title="Original";usb_game.title_id="BLES01287";
    std::vector<GameEntry> title_games{usb_game};loaded_names.apply(title_games);assert(title_games[0].title==name && title_games[0].path==path && title_games[0].title_id=="BLES01287");
    const auto copy_image=[&](const std::filesystem::path& dest){std::filesystem::copy_file(root+"/tests/assets/full_cover_275x147.png",dest,std::filesystem::copy_options::overwrite_existing);};
    const auto original_dest=tools/"covers/BLES01287.jpg";copy_image(original_dest);
    for(const auto& n:{"BLES01287.jpg","BLES01287_INSIDE.jpg","BLES01287_DISC.png"})copy_image(tools/"usb1"/n);
    copy_image(tools/"usb0/BLES01287.jpg");
    std::ofstream(tools/"usb1/BLES01287_DISC.png")<<"broken";
    auto imported=GameToolsFix35::import_usb(usb_game,{(tools/"usb0").string(),(tools/"usb1").string()},(tools/"covers").string());
    assert(!imported.ok && std::filesystem::file_size(original_dest)>100 && !std::filesystem::exists(tools/"covers/BLES01287_INSIDE.jpg"));
    copy_image(tools/"usb1/BLES01287_DISC.png");copy_image(tools/"covers/BLES01287_INSIDE.png");
    imported=GameToolsFix35::import_usb(usb_game,{(tools/"usb0").string(),(tools/"usb1").string()},(tools/"covers").string());
    assert(imported.ok && !std::filesystem::exists(tools/"covers/BLES01287_INSIDE.png"));
    GameToolsFix35::resolve_art(usb_game,(tools/"covers").string());
    assert(usb_game.inside_cover_path==(tools/"covers/BLES01287_INSIDE.jpg").string() && usb_game.disc_art_path==(tools/"covers/BLES01287_DISC.png").string());
    usb_game.title_id="../bad";assert(!GameToolsFix35::import_usb(usb_game,{(tools/"usb1").string()},(tools/"covers").string()).ok);
    for(const auto& item:std::filesystem::directory_iterator(tools/"covers"))assert(item.path().extension()!=".orbit-new" && item.path().extension()!=".orbit-old");
    std::filesystem::remove_all(tools);
    puts("PASS: UTF-8/UTF-16 title round-trip and persistence keyed by game path, complete-triplet USB import, corrupt image preserves old files, extension precedence cleanup and ID traversal rejection");
    puts("PASS: unchanged HUD allocation with updated nine-row window/highlight, exact ARGB bytes and persistent List layout");
    r.shutdown();stage.shutdown();
}
