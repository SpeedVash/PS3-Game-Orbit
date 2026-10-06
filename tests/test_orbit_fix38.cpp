#include <bits/stdc++.h>
#include <sys/stat.h>
#define private public
#include "rsx_renderer_v10.h"
#undef private
#include "orbit_settings_fix37.h"
#include "file_store_fix38.h"
#include "background_settings_fix36.h"
#include "orbit_ui_fix30.h"
#include "preferences_fix29.h"
#include "layout_settings_fix31.h"
#include "artwork_resize_fix37.h"
#include "game_tools_fix35.h"
#include "library_browser_fix28.h"
#include "prefetch_policy_fix36.h"
#include "orbit_flow_fix31.h"
#include "jfx_case_fix29.h"
#include "case_model_fix37.h"
#include "library_pair_fix28.h"

namespace fs=std::filesystem;
static bool fail_commit_once=false;
extern "C" int __real_rename(const char*,const char*);
extern "C" int __wrap_rename(const char* from,const char* to){
    if(fail_commit_once && std::string(from).size()>=4 && std::string(from).substr(std::string(from).size()-4)==".tmp"){
        fail_commit_once=false;errno=EIO;return -1;
    }
    struct stat st{};if(stat(to,&st)==0){errno=EEXIST;return -1;}
    return __real_rename(from,to);
}

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
    const auto scratch=fs::temp_directory_path()/("orbit38-"+std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
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
    settings.layout=OrbitLayout::Spine;settings.animated_background=true;settings.zoom=.60f;
    assert(settings.save(config) && restored.load(config) && restored.layout==OrbitLayout::Spine && restored.animated_background);
    settings.layout=OrbitLayout::Classic;settings.zoom=.65f;
    assert(settings.save(config) && restored.load(config) && restored.layout==OrbitLayout::Classic && std::fabs(restored.zoom-.65f)<.0001f);
    const auto complete=bytes(config);fail_commit_once=true;settings.layout=OrbitLayout::List;
    assert(!settings.save(config) && bytes(config)==complete && restored.load(config) && restored.layout==OrbitLayout::Classic);
    assert(!fs::exists(config+".tmp"));
    assert(settings.save(config));fs::remove(config);assert(restored.load(config)); // Interrupted rename: recover the previous complete backup.
    assert(settings.save(config) && restored.load(config) && restored.layout==OrbitLayout::List);
    PreferencesFix29 pref;auto pref_state=browser.state();pref_state.games[0].favorite=true;pref_state.selected=0;
    pref.capture(pref_state);const auto pref_path=(scratch/"state.dat").string();assert(pref.save(pref_path));
    pref_state.games[0].favorite=false;pref_state.selected=1;pref_state.filter=FilterMode::All;pref.capture(pref_state);assert(pref.save(pref_path));
    PreferencesFix29 loaded_pref;assert(loaded_pref.load(pref_path) && loaded_pref.selected_path()==pref_state.games[1].path);
    auto persisted_games=pref_state.games;persisted_games[0].favorite=true;loaded_pref.apply(persisted_games);assert(!persisted_games[0].favorite);
    LayoutSettingsFix31 ls;ls.capture(OrbitLayout::Spine);const auto lp=(scratch/"layout.dat").string();assert(ls.save(lp));ls.capture(OrbitLayout::List);assert(ls.save(lp));LayoutSettingsFix31 lr;assert(lr.load(lp) && lr.layout()==OrbitLayout::List);
    BackgroundSettingsFix36 bs;const auto bp=(scratch/"background.dat").string();bs.capture(true);assert(bs.save(bp));bs.capture(false);assert(bs.save(bp));BackgroundSettingsFix36 br;assert(br.load(bp) && !br.animated());
    GameToolsFix35::Names names;const auto np=(scratch/"names.dat").string();assert(names.set(pref_state.games[0].path,"Primeiro") && names.save(np));assert(names.set(pref_state.games[0].path,"Segundo") && names.save(np));
    GameToolsFix35::Names read_names;assert(read_names.load(np));read_names.apply(persisted_games);assert(persisted_games[0].title=="Segundo");
    puts("PASS: LV2 no-overwrite rename simulation, repeated settings/preferences/layout/background/name commits, failed-commit rollback and interrupted-save backup recovery");
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
    assert(std::fabs(CaseModelFix37::opening_angle(1)-160)<.0001f);
    assert(mesh.parts[0].vertices.size()>original.parts[0].vertices.size());
    using Position=std::array<long,3>;std::vector<Position> baked,approved;
    const auto quantize=[](const V14Vertex& v){return Position{{std::lround(v.x*10000),std::lround(v.y*10000),std::lround(v.z*10000)}};};
    for(const auto& v:mesh.parts[0].vertices)baked.push_back(quantize(v));
    for(const auto& part:mesh.parts)if(part.joint!=V14MeshPart::Joint::Closed && part.material==V14MeshPart::Material::ClearPlastic)
        for(const auto& v:part.vertices)approved.push_back(quantize(v));
    std::sort(baked.begin(),baked.end());std::sort(approved.begin(),approved.end());assert(baked==approved);
    assert(mesh.parts[0].flex_rest.empty());
    const auto extracted=CaseModelFix37::joint_transform(V14MeshPart::Joint::Disc,1);
    assert(extracted.m[12]==96 && std::fabs(extracted.m[14]-24.7f)<.0001f);
    bool inner_rim=false,outer_rim=false;
    for(const auto& part:mesh.parts)if(part.joint==V14MeshPart::Joint::Disc){
        if(part.material==V14MeshPart::Material::Disc && part.texture_role==V14MeshPart::TextureRole::None)
            for(const auto& v:part.vertices)assert(std::fabs(v.nz)<.00001f); // No opaque front over the label.
        if(part.material==V14MeshPart::Material::DiscGlass){assert(part.color[3]<.3f);
            for(const auto& v:part.vertices){const float rad=std::hypot(v.x,v.y);assert((rad>=7.499f && rad<=8.001f) || (rad>=59.499f && rad<=60.001f));inner_rim|=rad<8.01f;outer_rim|=rad>59.49f;}
        }
    }
    assert(inner_rim && outer_rim);
    puts("PASS: closed geometry baked from the exact approved model at rest; no model swap, 160-degree stop, farther/forward disc and label unobstructed by gray faces");
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
        if(p.texture_role==V14MeshPart::TextureRole::DiscLabel)for(const auto& v:p.vertices){const float r=std::hypot(v.x,v.y);assert(r>=7.999f && r<=59.501f);}
        if(p.texture_role==V14MeshPart::TextureRole::Inside && p.joint!=V14MeshPart::Joint::Spine){
            const auto part=std::find_if(original.parts.begin(),original.parts.end(),[&](const V14MeshPart& x){return x.surface==p.surface;});assert(part!=original.parts.end() && part->vertices.size()==p.vertices.size());
            for(std::size_t i=0;i<p.vertices.size();++i){assert(p.vertices[i].x==part->vertices[i].x && p.vertices[i].y==part->vertices[i].y && std::fabs(p.vertices[i].u+part->vertices[i].u-1)<.0001f);}
        }
    }
    puts("PASS: opening 0..160 degrees, continuous outer/inside seams, full-size inside artwork, finite geometry and 0.5-mm inner and outer clear disc rims");
#else
    auto mesh=build_v14_case_mesh();
#endif
    RsxStage1 stage;assert(stage.init());RsxRendererV10 renderer;assert(renderer.init(stage,mesh,false));CoverCache cache(15,16u<<20);
    // Actual PNG -> ARGB GPU bytes and current-game role binding, after importing by ISO stem and ID.
    GameToolsFix35::resolve_art(iso,dest.string());assert(iso.disc_art_path==(dest/"BLES01287_DISC.png").string());
    CoverflowState actual;actual.games={iso};rebuild_visible(actual);actual.inspection_target=true;
    renderer.apply_cache_policy(actual,cache);assert(renderer.sync_inspection_art(actual) && renderer.has_disc_art_texture(0));
    const auto* bound=renderer.texture_for_art_role(V14MeshPart::TextureRole::DiscLabel,0);
    assert(bound && bound==&renderer.disc_cache_.entries.at(0).texture && bound->width==500 && bound->height==500 && bound->reusable);
    const auto normalized_disc=decode(dest/"BLES01287_DISC.png");const auto* sample=static_cast<const unsigned char*>(bound->gpu_ptr);
    for(const auto& xy:{std::pair<int,int>{100,200},{350,200}}){const auto* expected=normalized_disc.rgba.data()+std::size_t(xy.second)*normalized_disc.pitch+xy.first*4;const auto* gpu=sample+std::size_t(xy.second)*bound->pitch+xy.first*4;assert(gpu[0]==255);for(unsigned c=0;c<3;++c)assert(gpu[c+1]==expected[c]);}
    GpuTextureStage1 alpha_gpu;assert(stage.prepare_disc_artwork(load_cover_file((scratch/"alpha.png").string(),GameCoverKind::FrontOnly),alpha_gpu));
    const auto* transparent=static_cast<const unsigned char*>(alpha_gpu.gpu_ptr)+250*alpha_gpu.pitch+499*4;
    assert(transparent[0]==255 && transparent[1]==211 && transparent[2]==216 && transparent[3]==223);
    stage.release_cover(alpha_gpu);
    puts("PASS: real ID/ISO-disc PNG load, selected-game cache/texture binding, 500px ARGB pixels and opaque label substrate with separate clear rims");
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
    renderer.shutdown();stage.shutdown();fs::remove_all(scratch);puts("PASS: empty filtered library has no case; START remains available; v1.4.1 host suite complete");
}
