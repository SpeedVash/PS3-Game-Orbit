#include <bits/stdc++.h>
#define private public
#include "rsx_renderer_v10.h"
#undef private
#include "orbit_settings_fix37.h"
#include "artwork_resize_fix37.h"
#include "game_tools_fix35.h"
#include "library_browser_fix28.h"
#include "prefetch_policy_fix36.h"
#include "orbit_flow_fix31.h"
#include "jfx_case_fix29.h"
#include "case_model_fix37.h"
#include "library_pair_fix28.h"

namespace fs=std::filesystem;
static InputFrame input(){InputFrame f;f.connected=true;return f;}
static std::vector<GameEntry> catalog(int n,const std::string& cover){
    std::vector<GameEntry> games(n);
    for(int i=0;i<n;++i){auto& g=games[i];g.path="/dev_hdd0/GAMES/Game"+std::to_string(i);g.title="Jogo "+std::to_string(i);g.cover_path=g.inside_cover_path=g.disc_art_path=cover;g.cover_kind=GameCoverKind::FullCover;}
    return games;
}
static DecodedImageRGBA decode(const fs::path& path){DecodedImageRGBA image;std::string error;assert(decode_cover_rgba(load_cover_file(path.string(),GameCoverKind::FullCover),image,error));return image;}
static std::string bytes(const fs::path& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
[[maybe_unused]] static std::array<float,3> world(const V14Vertex& v,const Mat4& m){return {{m.m[0]*v.x+m.m[4]*v.y+m.m[8]*v.z+m.m[12],m.m[1]*v.x+m.m[5]*v.y+m.m[9]*v.z+m.m[13],m.m[2]*v.x+m.m[6]*v.y+m.m[10]*v.z+m.m[14]}};}

int main(int argc,char** argv){
    assert(argc==2);std::setvbuf(stdout,nullptr,_IONBF,0);const fs::path root=argv[1];
    const auto scratch=fs::temp_directory_path()/("orbit37-"+std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
    fs::create_directories(scratch/"covers");const std::string image=(root/"tests/assets/full_cover_275x147.png").string();
    LibraryBrowserFix28 browser("");browser.replace_catalog(catalog(57,image));
    auto f=input();f.start.pressed=true;auto command=browser.update(f,0);assert(browser.state().menu.open && browser.state().menu.homebrew && !command.rescan_library);
    f=input();f.down.pressed=true;browser.update(f,0);f=input();f.cross.pressed=true;browser.update(f,0);assert(!browser.state().menu.animated_background && browser.take_preferences_changed());
    f=input();f.down.pressed=true;browser.update(f,0);f=input();f.cross.pressed=true;assert(browser.update(f,0).import_all_usb);
    f=input();f.down.pressed=true;browser.update(f,0);f=input();f.cross.pressed=true;browser.update(f,0);assert(!browser.state().menu.remember_last_game && browser.take_preferences_changed());
    f=input();f.triangle.pressed=true;browser.update(f,0);assert(browser.state().menu.open && !browser.state().menu.homebrew && browser.state().menu.selected==0);
    f=input();f.cross.pressed=true;assert(browser.update(f,0).game_menu_action==GameMenuActionFix35::Rename);
    f=input();f.down.pressed=true;browser.update(f,0);f=input();f.cross.pressed=true;assert(browser.update(f,0).game_menu_action==GameMenuActionFix35::ImportUsb);
    f=input();f.down.pressed=true;browser.update(f,0);f=input();f.cross.pressed=true;assert(browser.update(f,0).game_menu_action==GameMenuActionFix35::ReloadCovers);
    f=input();f.down.pressed=true;browser.update(f,0);f=input();f.cross.pressed=true;browser.update(f,0);assert(!browser.state().menu.open && current_game(browser.state())->favorite);
    f=input();f.start.pressed=true;browser.update(f,0);f=input();f.cross.pressed=true;assert(browser.update(f,0).rescan_library);
    f=input();f.l3.pressed=true;browser.update(f,0);assert(browser.state().inspection_target);
    f=input();f.cross.pressed=true;assert(browser.update(f,.1f).mount_selected);
    f=input();f.triangle.pressed=true;browser.update(f,0);assert(browser.state().menu.open && !browser.state().menu.homebrew);
    puts("PASS: separate four-row TRIANGLE/START menus, every action, favorites, animation/last-game toggles and webMAN mount while open");

    browser.restore_layout(OrbitLayout::List);browser.restore_view(.73f,341,-12,true,false);
    OrbitSettingsFix37 settings;settings.capture(browser.state(),browser.scale(),browser.automatic());
    const auto config=(scratch/"settings.dat").string();assert(settings.save(config));OrbitSettingsFix37 restored;
    assert(restored.load(config) && restored.layout==OrbitLayout::List && !restored.animated_background && !restored.remember_last_game && restored.automatic_rotation && std::fabs(restored.zoom-.73f)<.0001f);
    auto data=bytes(config);data.back()='X';std::ofstream(config)<<data;assert(!restored.load(config) && restored.layout==OrbitLayout::List);
    std::ofstream(config)<<std::string(5000,'x');assert(!restored.load(config));assert(!settings.save((scratch/"missing/settings.dat").string()));assert(!fs::exists(config+".tmp"));
    puts("PASS: settings round trip, inspection-safe saved pose, checksum corruption/oversize rejection and atomic-write failure handling");

    const auto usb=scratch/"usb",dest=scratch/"covers";fs::create_directories(usb/"PS3COVERS");
    const auto copy=[&](const fs::path& to){fs::copy_file(image,to,fs::copy_options::overwrite_existing);};
    GameEntry iso;iso.format=GameFormat::ISO;iso.path="/dev_hdd0/PS3ISO/My ISO Game.ISO";
    copy(usb/"My ISO Game.jpg");assert(!GameToolsFix35::import_usb(iso,{usb.string()},dest.string()).ok);
    copy(usb/"PS3COVERS/My ISO Game.jpg");copy(dest/"My ISO Game_INSIDE.jpg");copy(dest/"My ISO Game_DISC.png");
    const auto before_inside=bytes(dest/"My ISO Game_INSIDE.jpg"),before_disc=bytes(dest/"My ISO Game_DISC.png");
    auto result=GameToolsFix35::import_usb(iso,{usb.string()},dest.string());assert(result.ok && result.updated_mask==1);
    auto cover=decode(dest/"My ISO Game.jpg");assert(cover.width==1000 && cover.height==550);
    assert(bytes(dest/"My ISO Game_INSIDE.jpg")==before_inside && bytes(dest/"My ISO Game_DISC.png")==before_disc);
    fs::remove(usb/"PS3COVERS/My ISO Game.jpg");copy(usb/"PS3COVERS/My ISO Game_INSIDE.PNG");
    result=GameToolsFix35::import_usb(iso,{usb.string()},dest.string());assert(result.ok && result.updated_mask==2);
    auto inside=decode(dest/"My ISO Game_INSIDE.jpg");assert(inside.width==1000 && inside.height==550 && bytes(dest/"My ISO Game_DISC.png")==before_disc);
    copy(usb/"PS3COVERS/My ISO Game_DISC.png");std::ofstream(usb/"PS3COVERS/My ISO Game_INSIDE.PNG")<<"broken";
    result=GameToolsFix35::import_usb(iso,{usb.string()},dest.string());assert(result.ok && result.updated_mask==4);
    auto disc=decode(dest/"My ISO Game_DISC.png");assert(disc.width==500 && disc.height==500);
    iso.title_id="BLES01287";result=GameToolsFix35::import_usb(iso,{usb.string()},dest.string());assert(result.ok && result.updated_mask==4 && fs::exists(dest/"BLES01287_DISC.png"));
    DecodedImageRGBA pixels;pixels.width=2;pixels.height=1;pixels.pitch=8;pixels.rgba={255,0,0,255,0,0,0,0};
    DecodedImageRGBA resized;assert(ArtworkResizeFix37::resize(pixels,500,500,resized));assert(resized.rgba[(250*500+250)*4]==255 && resized.rgba[(250*500+499)*4+3]==0);
    std::vector<std::uint8_t> encoded;assert(ArtworkResizeFix37::encode(resized,true,encoded));std::ofstream png(scratch/"alpha.png",std::ios::binary);png.write(reinterpret_cast<const char*>(encoded.data()),encoded.size());png.close();
    auto alpha=decode(scratch/"alpha.png");assert(alpha.width==500 && alpha.rgba[(250*500+499)*4+3]==0);
    puts("PASS: partial USB/PS3COVERS and ISO stem/ID imports, exact 1000x550/500x500 output, corrupt-role isolation and PNG alpha without dark fringe");

#ifndef PS3_GAME_ORBIT_FLOW_ONLY
    const auto original=JfxCaseFix29::build();auto mesh=CaseModelFix37::build(original);
    assert(mesh.parts.size()<64 && mesh.parts[1].indices==original.parts[1].indices);
    unsigned checked=0;
    for(float phase:{0.f,.3f,.425f,.55f,.65f,1.f}){
        CaseModelFix37::deform_spine(mesh,phase);
        for(const auto& p:mesh.parts){
            assert(p.vertices.size()<65536 && p.indices.size()%3==0);
            for(auto i:p.indices)assert(i<p.vertices.size());
            for(const auto& v:p.vertices)assert(std::isfinite(v.x+v.y+v.z+v.nx+v.ny+v.nz));
            if(p.material!=V14MeshPart::Material::Paper || (p.joint!=V14MeshPart::Joint::Base && p.joint!=V14MeshPart::Joint::Lid))continue;
            for(const auto& v:p.vertices){if(std::fabs(v.x-CaseModelFix37::HingeX)>.001f)continue;
                for(const auto& spine:mesh.parts){if(spine.joint!=V14MeshPart::Joint::Spine || spine.material!=p.material || spine.texture_role!=p.texture_role)continue;
                    for(const auto& s:spine.vertices){if(std::fabs(s.u-v.u)>.00001f || std::fabs(s.v-v.v)>.00001f)continue;
                        const auto a=world(v,CaseModelFix37::joint_transform(p.joint,phase)),b=world(s,CaseModelFix37::joint_transform(spine.joint,phase));
                        float gap=0;for(unsigned k=0;k<3;++k)gap+=(a[k]-b[k])*(a[k]-b[k]);assert(std::sqrt(gap)<.02f);++checked;
                    }
                }
            }
        }
    }
    assert(checked>24);
    for(const auto& p:mesh.parts){
        if(p.texture_role==V14MeshPart::TextureRole::DiscLabel)for(const auto& v:p.vertices){const float r=std::hypot(v.x,v.y);assert(r>=7.49f && r<=59.601f);}
        if(p.texture_role==V14MeshPart::TextureRole::Inside && p.joint!=V14MeshPart::Joint::Spine){
            const auto part=std::find_if(original.parts.begin(),original.parts.end(),[&](const V14MeshPart& x){return x.surface==p.surface;});assert(part!=original.parts.end() && part->vertices.size()==p.vertices.size());
            for(std::size_t i=0;i<p.vertices.size();++i){assert(p.vertices[i].x==part->vertices[i].x && p.vertices[i].y==part->vertices[i].y && std::fabs(p.vertices[i].u+part->vertices[i].u-1)<.0001f);}
        }
    }
    puts("PASS: opening 0..180 degrees, continuous outer/inside seams, full-size inside artwork, finite geometry and 0.4-mm clear disc rim");
#else
    auto mesh=build_v14_case_mesh();
#endif
    RsxStage1 stage;assert(stage.init());RsxRendererV10 renderer;assert(renderer.init(stage,mesh,false));CoverCache cache(15,16u<<20);
    CoverflowState state;state.games=catalog(57,image);rebuild_visible(state);state.selected=28;
    for(const auto layout:{OrbitLayout::Spine,OrbitLayout::Classic,OrbitLayout::List}){
        state.layout=layout;state.inspection_target=false;state.inspection_phase=0;OrbitFlowFix31::reset(state);renderer.apply_cache_policy(state,cache);
        const std::size_t limit=layout==OrbitLayout::Spine?15:layout==OrbitLayout::Classic?7:5;
        assert(renderer.resident_limit()==limit && PrefetchPolicyFix36::candidates(state).size()==limit-1);
        assert(OrbitFlowFix31::poses(state).size()==(layout==OrbitLayout::Spine?7:layout==OrbitLayout::Classic?2:1));
        assert(renderer.sync_visible_covers(state,cache,1));for(int i=0;i<40;++i)renderer.prefetch_nearby_cover(state,cache);
        for(int i=0;i<40;++i)renderer.prefetch_inspection_art(state);
        assert(renderer.gpu_cover_count()==limit);
        assert(renderer.inside_cache_count()<=(layout==OrbitLayout::Spine?15:limit) && renderer.disc_cache_count()<=(layout==OrbitLayout::Spine?15:limit));
        if(layout!=OrbitLayout::Spine)assert(renderer.inside_cache_count()==limit && renderer.disc_cache_count()==limit);
        const auto misses=renderer.gpu_cache_misses()+renderer.inside_cache_misses()+renderer.disc_cache_misses();
        for(int i=0;i<40;++i){renderer.prefetch_nearby_cover(state,cache);renderer.prefetch_inspection_art(state);}
        assert(misses==renderer.gpu_cache_misses()+renderer.inside_cache_misses()+renderer.disc_cache_misses());
        for(int index:{29,0,56,7}){state.selected=index;OrbitFlowFix31::reset(state);renderer.apply_cache_policy(state,cache);assert(renderer.sync_visible_covers(state,cache,1));
            for(int i=0;i<40;++i){renderer.prefetch_nearby_cover(state,cache);renderer.prefetch_inspection_art(state);}
            assert(renderer.gpu_cover_count()<=limit && renderer.inside_cache_count()<=limit && renderer.disc_cache_count()<=limit);
        }
    }
    puts("PASS: real resident cache limits 15/15/15, 7/7/7, 5/5/5; 3 visible Spine neighbors each side; layout changes, wrapping and stable prefetch without repeated decoding");
    browser.replace_catalog(catalog(57,image));for(int i=0;i<57;++i)browser.entry(i)->favorite=false;browser.restore_selection(FilterMode::Favorites,"");
    auto empty=browser.render_state();assert(empty.visible.empty() && LibraryPairFix28::poses(empty,1).empty());
    renderer.apply_cache_policy(empty,cache);assert(renderer.gpu_cover_count()==0 && renderer.inside_cache_count()==0 && renderer.disc_cache_count()==0);
    assert(renderer.render(empty,mesh,1) && renderer.last_stats().draw_calls==0);
#ifndef PS3_GAME_ORBIT_FLOW_ONLY
    assert(validate_native_plan_fix32(renderer.last_frame_plan(),mesh));
    state.selected=0;state.layout=OrbitLayout::Classic;
    for(float phase:{0.f,.3f,.425f,.55f,.65f,1.f}){state.inspection_target=phase>0;state.inspection_phase=phase;OrbitFlowFix31::reset(state);
        assert(renderer.sync_case_animation(mesh,phase));const auto plan=build_v10_frame_plan(LibraryPairFix28::poses(state,1),mesh);assert(validate_native_plan_fix32(plan,mesh));}
#endif
    f=input();f.triangle.pressed=true;browser.update(f,0);assert(!browser.state().menu.open);f=input();f.start.pressed=true;browser.update(f,0);assert(browser.state().menu.open && browser.state().menu.homebrew);
    assert(renderer.set_library_hud(browser.hud_lines(""),&browser.state()));
    renderer.shutdown();stage.shutdown();fs::remove_all(scratch);puts("PASS: empty filtered library has no case; START remains available; v1.4 host suite complete");
}
