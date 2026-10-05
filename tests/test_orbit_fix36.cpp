#include <bits/stdc++.h>
#define private public
#include "rsx_renderer_v10.h"
#undef private
#include "background_settings_fix36.h"
#include "prefetch_policy_fix36.h"
#include "game_tools_fix35.h"
#include "library_browser_fix28.h"
#include "orbit_flow_fix31.h"
#include "orbit_ui_fix30.h"
#include "performance_fix35.h"
#include "jfx_case_fix29.h"
#include "case_animation_fix32.h"

namespace fs=std::filesystem;
static InputFrame idle(){InputFrame f;f.connected=true;return f;}
static std::vector<GameEntry> catalog(int n,const std::string& cover){
    std::vector<GameEntry> games(n);
    for(int i=0;i<n;++i){auto& g=games[i];g.path="GAME"+std::to_string(i);g.title="Jogo "+std::to_string(i);g.cover_path=cover;g.cover_kind=GameCoverKind::FullCover;}
    return games;
}
static std::string bytes(const fs::path& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}

int main(int argc,char** argv){
    assert(argc==2);std::setvbuf(stdout,nullptr,_IONBF,0);const fs::path root=argv[1];
    const auto scratch=fs::temp_directory_path()/("orbit36-tests-"+std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
    fs::create_directories(scratch/"covers");
    const auto copy=[&](const fs::path& p){fs::create_directories(p.parent_path());fs::copy_file(root/"tests/assets/full_cover_275x147.png",p,fs::copy_options::overwrite_existing);};
    const std::string image=(root/"tests/assets/full_cover_275x147.png").string();

    GameEntry folder;folder.path="/dev_hdd0/GAMES/My Game";folder.title_id="BLES01287";folder.format=GameFormat::Folder;
    const auto usb=scratch/"usb0",usb1=scratch/"usb1",dest=scratch/"covers";
    const std::vector<std::string> ports={usb.string(),usb1.string()};
    copy(usb/"BLES01287.jpg");copy(usb/"PS3COVERS/nested/BLES01287.jpg");
    assert(!GameToolsFix35::import_usb(folder,ports,dest.string()).ok);
    copy(dest/"BLES01287_INSIDE.png");copy(dest/"BLES01287_DISC.png");
    const auto old_inside=bytes(dest/"BLES01287_INSIDE.png"),old_disc=bytes(dest/"BLES01287_DISC.png");
    copy(usb/"PS3COVERS/BLES01287.JPG");
    auto result=GameToolsFix35::import_usb(folder,ports,dest.string());
    assert(result.ok && result.updated_mask==1 && bytes(dest/"BLES01287.jpg")==bytes(usb/"PS3COVERS/BLES01287.JPG"));
    assert(bytes(dest/"BLES01287_INSIDE.png")==old_inside && bytes(dest/"BLES01287_DISC.png")==old_disc);
    fs::remove(usb/"PS3COVERS/BLES01287.JPG");
    copy(usb1/"PS3COVERS/BLES01287_INSIDE.JPEG");
    result=GameToolsFix35::import_usb(folder,ports,dest.string());
    assert(result.ok && result.updated_mask==2 && !fs::exists(dest/"BLES01287_INSIDE.png") && fs::exists(dest/"BLES01287_INSIDE.jpeg"));
    assert(bytes(dest/"BLES01287_DISC.png")==old_disc);
    fs::remove(usb1/"PS3COVERS/BLES01287_INSIDE.JPEG");
    std::ofstream(usb/"PS3COVERS/BLES01287_INSIDE.jpg")<<"corrupt";
    copy(usb/"PS3COVERS/BLES01287_DISC.PNG");
    result=GameToolsFix35::import_usb(folder,ports,dest.string());
    assert(result.ok && result.updated_mask==4 && fs::exists(dest/"BLES01287_INSIDE.jpeg"));
    assert(result.message.find("erro")!=std::string::npos);
    fs::remove(usb/"PS3COVERS/BLES01287_INSIDE.jpg");fs::remove(usb/"PS3COVERS/BLES01287_DISC.PNG");
    copy(usb/"PS3COVERS/BLES01287.png");
    std::ofstream(dest/"BLES01287.jpg.orbit-old")<<"existing backup";
    const auto old_cover=bytes(dest/"BLES01287.jpg");
    assert(!GameToolsFix35::import_usb(folder,ports,dest.string()).ok);
    assert(bytes(dest/"BLES01287.jpg")==old_cover && !fs::exists(dest/"BLES01287.png.orbit-new"));
    fs::remove(dest/"BLES01287.jpg.orbit-old");
    folder.title_id="../escape";assert(!GameToolsFix35::import_usb(folder,ports,dest.string()).ok);
    puts("PASS: USB/PS3COVERS only; partial roles, uppercase extensions, second port, corrupt-role isolation, old-role preservation and backup conflict rollback");

    GameEntry iso;iso.path="/dev_hdd0/PS3ISO/My ISO Game.ISO";iso.format=GameFormat::ISO;
    copy(usb/"PS3COVERS/My ISO Game.jpg");copy(usb/"PS3COVERS/My ISO Game_INSIDE.jpg");copy(usb/"PS3COVERS/My ISO Game_DISC.png");
    result=GameToolsFix35::import_usb(iso,ports,dest.string());assert(result.ok && result.updated_mask==7);
    GameToolsFix35::resolve_art(iso,dest.string());
    assert(iso.cover_path==(dest/"My ISO Game.jpg").string() && iso.inside_cover_path==(dest/"My ISO Game_INSIDE.jpg").string() && iso.disc_art_path==(dest/"My ISO Game_DISC.png").string());
    iso.title_id="BLUS12345";result=GameToolsFix35::import_usb(iso,ports,dest.string());assert(result.ok && result.updated_mask==7);
    GameToolsFix35::resolve_art(iso,dest.string());assert(iso.cover_path==(dest/"BLUS12345.jpg").string());
    copy(usb/"PS3COVERS/BLUS12345.png");result=GameToolsFix35::import_usb(iso,ports,dest.string());
    assert(result.ok && result.updated_mask==7 && fs::exists(dest/"BLUS12345.png") && !fs::exists(dest/"BLUS12345.jpg"));
    GameToolsFix35::resolve_art(iso,dest.string());assert(iso.cover_path==(dest/"BLUS12345.png").string());
    iso.path="/dev_hdd0/PS3ISO/../.iso";iso.title_id.clear();assert(!GameToolsFix35::import_usb(iso,ports,dest.string()).ok);
    puts("PASS: ISO import with exact stem/spaces/uppercase .ISO; optional ID, ID priority, canonical destination and resolver reload of all roles");

    CoverflowState policy;policy.games=catalog(57,image);rebuild_visible(policy);policy.selected=28;
    for(const auto layout:{OrbitLayout::List,OrbitLayout::Classic,OrbitLayout::Spine}){
        policy.layout=layout;
        for(const int direction:{-1,1}){
            policy.navigation_direction=direction;const auto c=PrefetchPolicyFix36::candidates(policy);
            assert(c.size()==(layout==OrbitLayout::List?3:layout==OrbitLayout::Classic?5:14));
            assert(c.front()==28+direction && c[1]==28-direction);
            assert(std::set<int>(c.begin(),c.end()).size()==c.size() && std::find(c.begin(),c.end(),28)==c.end());
        }
    }
    policy.layout=OrbitLayout::List;policy.selected=0;policy.navigation_direction=-1;
    assert((PrefetchPolicyFix36::candidates(policy)==std::vector<int>{56,1,55}));
    for(const int n:{0,1,2,3,8}){
        policy.games=catalog(n,image);rebuild_visible(policy);policy.selected=0;policy.layout=OrbitLayout::Spine;
        const auto c=PrefetchPolicyFix36::candidates(policy);assert(c.size()==std::size_t(std::max(0,n-1)));
        assert(std::set<int>(c.begin(),c.end()).size()==c.size());
    }
    puts("PASS: layout-aware neighbor counts 3/5/14, direction priority, wrapping, uniqueness and empty/short catalogs");

    BackgroundSettingsFix36 settings;assert(settings.animated());assert(!settings.load((scratch/"missing").string()));
    const auto pref=(scratch/"background.dat").string();settings.capture(false);assert(settings.save(pref));
    BackgroundSettingsFix36 restored;assert(restored.load(pref) && !restored.animated());
    settings.capture(true);assert(settings.save(pref));assert(restored.load(pref) && restored.animated());
    std::ofstream(pref)<<std::string(100,'x');assert(!restored.load(pref) && restored.animated());
    assert(!settings.save((scratch/"nonexistent/background.dat").string()));
    LibraryBrowserFix28 browser("");browser.replace_catalog(catalog(57,image));
    auto input=idle();input.start.pressed=true;browser.update(input,0);assert(browser.state().menu.open);
    input=idle();input.up.pressed=true;browser.update(input,0);assert(browser.state().menu.selected==4);
    input=idle();input.down.pressed=true;browser.update(input,0);assert(browser.state().menu.selected==0);
    for(int i=0;i<3;++i)browser.update(input,0);
    input=idle();input.cross.pressed=true;auto command=browser.update(input,0);
    assert(!browser.state().menu.animated_background && browser.take_preferences_changed());
    assert(command.game_menu_action==GameMenuActionFix35::None && !command.rescan_library);
    input=idle();input.down.pressed=true;browser.update(input,0);input=idle();input.cross.pressed=true;
    assert(browser.update(input,0).rescan_library);
    LibraryBrowserFix28 empty("");empty.replace_catalog({});empty.restore_background(false);
    input=idle();input.start.pressed=true;empty.update(input,0);assert(empty.render_state().menu.open && !empty.render_state().menu.animated_background);
    puts("PASS: persistent on/off default, bounded corrupt-settings handling, five-row START menu, rescan mapping and empty-catalog options");

    auto waves=OrbitBackgroundFix36::build();assert(waves.vertices.size()==582 && waves.indices.size()==1728);
    for(auto i:waves.indices)assert(i<waves.vertices.size());
    const auto* wave_vertices=waves.vertices.data();const auto* wave_indices=waves.indices.data();const auto before=waves.vertices.front().y;
    OrbitBackgroundFix36::update(waves,9);assert(waves.vertices.data()==wave_vertices && waves.indices.data()==wave_indices && waves.vertices.front().y!=before);
    for(const auto& v:waves.vertices)assert(std::isfinite(v.x) && std::isfinite(v.y) && v.u>=0 && v.u<=1.001f && v.v>=0 && v.v<=1);
    assert(OrbitBackgroundFix36::gradient().valid() && OrbitBackgroundFix36::ribbon_texture().valid());

    RsxStage1 stage;assert(stage.init());
    const auto encoded=load_cover_file(image,GameCoverKind::FullCover);
    GpuTextureStage1 a,b;assert(stage.prepare_cover(encoded,a));auto* ptr=a.gpu_ptr;const auto alloc=a.allocation_bytes;
    auto* argb=stage.argb_scratch_.argb.data();stage.release_cover(a);assert(stage.pooled_texture_bytes()==alloc);
    assert(stage.prepare_cover(encoded,b) && b.gpu_ptr==ptr && stage.texture_buffer_reuses()==1 && stage.argb_scratch_.argb.data()==argb);
    stage.release_cover(b);
    const auto large=load_cover_file((root/"tests/assets/cache32_1024.png").string(),GameCoverKind::FrontOnly);
    std::array<GpuTextureStage1,4> blocks;
    for(auto& t:blocks)assert(stage.prepare_cover(large,t));
    for(auto& t:blocks)stage.release_cover(t);
    assert(stage.pooled_texture_bytes()<=RsxStage1::BufferPoolBytes && stage.texture_pool_.size()<=RsxStage1::BufferPoolSlots);
    stage.clear_texture_pool();assert(stage.prepare_cover(large,a));ptr=a.gpu_ptr;stage.release_cover(a);
    assert(stage.prepare_cover(encoded,b) && b.gpu_ptr==ptr && b.allocation_bytes==4u*1024u*1024u && gpu_texture_storage_bytes(b)==b.allocation_bytes);
    stage.release_cover(b);stage.clear_texture_pool();
    DecodedImageRGBA rgba;std::string error;assert(decode_cover_rgba(encoded,rgba,error));auto* scratchptr=rgba.rgba.data();
    assert(decode_cover_rgba(encoded,rgba,error) && rgba.rgba.data()==scratchptr);
    const auto vertical=load_cover_file((root/"tests/assets/cache36_vertical.png").string(),GameCoverKind::FullCover);
    assert(stage.prepare_cover(vertical,b));assert(stage.decode_scratch_.rgba.capacity()>0 && b.width==275 && b.height==147);stage.release_cover(b);
    puts("PASS: real texture-pointer reuse, RGBA/ARGB buffer reuse, 8 MiB/3-slot free-pool bound and actual allocation-byte accounting");

#ifndef PS3_GAME_ORBIT_FLOW_ONLY
    const auto mesh=CaseAnimationFix32::build(JfxCaseFix29::build());
#else
    const auto mesh=build_v14_case_mesh();
#endif
    RsxRendererV10 r;assert(r.init(stage,mesh,false));assert(r.prepare_orbit_background());
    const auto* static_ptr=r.background_texture_.gpu_ptr;const auto phase=r.wave_phase_;
    assert(r.update_orbit_background(false,.02f) && r.wave_phase_==phase);
    auto games=catalog(57,image);for(auto& g:games){g.inside_cover_path=image;g.disc_art_path=image;}
    CoverflowState state;state.games=games;state.layout=OrbitLayout::List;rebuild_visible(state);OrbitFlowFix31::reset(state);
    CoverCache cache(15,16u<<20);assert(r.sync_visible_covers(state,cache,1));
    assert(r.render(state,mesh,1) && r.last_stats().wave_draw_calls==0 && r.background_texture_.gpu_ptr==static_ptr);
    const auto allocations=stage.texture_buffer_allocations();
    assert(r.update_orbit_background(true,.02f) && r.wave_phase_>phase && stage.texture_buffer_allocations()==allocations);
    assert(r.render(state,mesh,1) && r.last_stats().wave_draw_calls==3 && r.last_stats().background_draw_calls==1);
    assert(r.prepare_animated_background() && stage.texture_buffer_allocations()==allocations);
    puts("PASS: 582-vertex/576-triangle background; allocation-free mesh updates, three wave draws, pause when disabled and original static texture retained");

    auto totalmiss=[&](){return r.inside_cache_misses()+r.disc_cache_misses();};
    auto misses=totalmiss();assert(r.prefetch_inspection_art(state) && totalmiss()==misses+1 && r.has_inside_texture(0) && !r.has_disc_art_texture(0));
    misses=totalmiss();assert(r.prefetch_inspection_art(state) && totalmiss()==misses+1 && r.has_disc_art_texture(0));
    auto* inside=r.inside_cache_.entries.at(0).texture.gpu_ptr;auto* disc=r.disc_cache_.entries.at(0).texture.gpu_ptr;
    state.inspection_target=true;misses=totalmiss();assert(r.sync_inspection_art(state) && totalmiss()==misses && r.inside_cache_.entries.at(0).texture.gpu_ptr==inside && r.disc_cache_.entries.at(0).texture.gpu_ptr==disc);
    r.invalidate_game_art(0,2);assert(!r.has_inside_texture(0) && r.has_disc_art_texture(0) && r.has_cover_texture(0));
    assert(r.prefetch_inspection_art(state));assert(r.disc_cache_.entries.at(0).texture.gpu_ptr==disc);
    for(int i=1;i<57;++i){
        state.selected=i;state.inspection_target=false;OrbitFlowFix31::reset(state);
        assert(r.sync_visible_covers(state,cache,1));
        misses=totalmiss();assert(r.prefetch_inspection_art(state) && totalmiss()==misses+1);
        misses=totalmiss();assert(r.prefetch_inspection_art(state) && totalmiss()==misses+1);
        assert(!r.prefetch_inspection_art(state));
        assert(r.gpu_cover_count()<=15 && r.inside_cache_count()<=15 && r.disc_cache_count()<=15);
        assert(r.gpu_cache_bytes()<=r.GpuCoverBudgetBytes && r.inside_cache_bytes()<=r.InspectionArtBudgetBytes && r.disc_cache_bytes()<=r.InspectionArtBudgetBytes);
        assert(stage.pooled_texture_bytes()<=stage.BufferPoolBytes);
    }
    assert(r.gpu_cover_count()==15 && r.inside_cache_count()==15 && r.disc_cache_count()==15 && stage.texture_buffer_reuses()>10);
    state.selected=42;assert(!r.prefetch_inspection_art(state));state.selected=41;
    assert(r.prefetch_inspection_art(state));assert(r.has_inside_texture(42) && r.has_disc_art_texture(42) && !r.has_inside_texture(43));
    state.selected=56;r.invalidate_game_art(56);state.games[56].inside_cover_path=(scratch/"broken.png").string();std::ofstream(scratch/"broken.png")<<"bad";
    assert(r.prefetch_inspection_art(state) && !r.has_inside_texture(56));assert(r.prefetch_inspection_art(state) && r.has_disc_art_texture(56));
    assert(!r.prefetch_inspection_art(state));
    state.games[56].disc_art_path.clear();assert(!r.prefetch_inspection_art(state) && !r.has_disc_art_texture(56));
    puts("PASS: one inspection image per slice, inside before disc, no decode on preloaded L3, per-role invalidation, failure suppression and independent 15/15/15 LRU caches over 57 games");

    r.clear_cover_textures();browser.restore_layout(OrbitLayout::Spine);
    for(int i=0;i<120;++i)browser.update(idle(),1.f/60);
    assert(r.sync_visible_covers(browser.state(),cache,1));std::map<int,void*> pinned;
    for(const auto& pose:OrbitFlowFix31::poses(browser.state()))pinned[pose.game_index]=r.covers_.at(pose.game_index).texture.gpu_ptr;
    assert(pinned.size()==11);
    for(int i=0;i<30;++i)r.prefetch_nearby_cover(browser.state(),cache);
    for(const auto& item:pinned)assert(r.covers_.at(item.first).texture.gpu_ptr==item.second);
    misses=r.gpu_cache_misses();for(int i=0;i<100;++i)assert(!r.prefetch_nearby_cover(browser.state(),cache));
    assert(r.gpu_cache_misses()==misses && r.gpu_cover_count()<=15);
    puts("PASS: visible Spine cases pinned; layout prefetch stabilizes without repeated decode/eviction");

    browser.restore_layout(OrbitLayout::List);input=idle();input.start.pressed=true;browser.update(input,0);
    assert(r.set_library_hud(browser.hud_lines(""),&browser.state()));
    const auto initial=r.hud_cache_.image().rgba;
    input=idle();input.down.pressed=true;for(int i=0;i<3;++i)browser.update(input,0);
    input=idle();input.cross.pressed=true;browser.update(input,0);
    assert(r.set_library_hud(browser.hud_lines(""),&browser.state()));
    assert(r.hud_cache_.image().rgba!=initial);
    assert(std::equal(initial.begin(),initial.begin()+r.hud_cache_.image().pitch*170,r.hud_cache_.image().rgba.begin()));
    puts("PASS: partial interface update redraws the animated-background option while retaining the header");
    r.shutdown();stage.shutdown();assert(stage.pooled_texture_bytes()==0);fs::remove_all(scratch);
    puts("PASS: FIX36 v1.3.3 host regression suite complete; hardware visibility/performance requires console testing");
}
