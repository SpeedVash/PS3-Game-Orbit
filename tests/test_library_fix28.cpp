#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include "library_scanner.h"
#include "cover_resolver.h"
#include "library_browser_fix28.h"
#include "library_hud_fix28.h"
#include "full_cover_case_fix26.h"
#include "safe_boot.h"
#define private public
#include "rsx_renderer_v10.h"
#undef private

namespace fs=std::filesystem;
static void file(const fs::path& path,const std::string& data){
    fs::create_directories(path.parent_path());std::ofstream stream(path,std::ios::binary);stream<<data;
}
static void write_sfo(const fs::path& path){
    std::vector<unsigned char> data(256,0);
    auto w16=[&](unsigned p,unsigned v){data[p]=v;data[p+1]=v>>8;};
    auto w32=[&](unsigned p,unsigned v){for(unsigned i=0;i<4;++i)data[p+i]=v>>(i*8);};
    const std::string title="Ação e coração",id="BLES00001";
    data[1]='P';data[2]='S';data[3]='F';w32(4,0x101);w32(8,52);w32(12,68);w32(16,2);
    const char keys[]="TITLE\0TITLE_ID\0";std::memcpy(data.data()+52,keys,sizeof(keys));
    w16(20,0);w16(22,0x204);w32(24,title.size()+1);w32(28,32);w32(32,0);
    w16(36,6);w16(38,0x204);w32(40,id.size()+1);w32(44,16);w32(48,32);
    std::memcpy(data.data()+68,title.c_str(),title.size()+1);std::memcpy(data.data()+100,id.c_str(),id.size()+1);
    file(path,std::string(reinterpret_cast<const char*>(data.data()),data.size()));
}
int main(int argc,char** argv){
    assert(argc==2);const fs::path project=argv[1];
    char temp[]="/tmp/ps3-loader-fix28-XXXXXX";assert(mkdtemp(temp));const fs::path dir=temp;
    const auto hdd=dir/"hdd",usb=dir/"usb",covers=dir/"covers";
    const auto fixture=project/"pkgfiles/USRDIR/FULL_COVER_FIX26_CONTINUA.png";
    file(hdd/"PS3ISO/Iso_BLES00002.ISO","");file(usb/"PS3ISO/Zebra.iso","");
    fs::create_directories(hdd/"PS3ISO/Ghost.iso");
    write_sfo(hdd/"GAMES/folder/PS3_GAME/PARAM.SFO");
    fs::copy_file(fixture,hdd/"GAMES/folder/PS3_GAME/ICON0.PNG");
    file(hdd/"GAMES/fake/PS3_GAME","not a directory");
    file(hdd/"GAMEZ/Other_BLES00003/PS3_DISC.SFB","");
    fs::create_directories(covers);fs::copy_file(fixture,covers/"BLES00002.PNG");
    fs::copy_file(project/"tests/assets/front_64x96.jpg",covers/"Zebra.jpg");
    LibraryScanner scanner;auto games=scanner.scan_locations({{hdd.string(),GameSource::HDD},{usb.string(),GameSource::USB}});
    assert(games.size()==4 && games[0].title=="Ação e coração" && games[0].title_id=="BLES00001");
    assert(games[0].source==GameSource::HDD && games.back().source==GameSource::USB);
    CoverResolver resolver(covers.string());
    for(auto& game:games){
        const auto cover=resolver.resolve(game);game.cover_path=cover.path;
        game.cover_kind=cover.path.empty() ? GameCoverKind::None : cover.is_full_cover ? GameCoverKind::FullCover : GameCoverKind::FrontOnly;
    }
    assert(games[0].cover_kind==GameCoverKind::FrontOnly);
    assert(games[1].cover_path==(covers/"BLES00002.PNG").string());
    assert(games[2].cover_path.empty() && games[3].cover_path==(covers/"Zebra.jpg").string());
    LibraryBrowserFix28 browser(fixture.string());browser.replace_catalog(games);
    InputFrame input;input.connected=true;input.right.pressed=true;
    assert(!browser.update(input,0).mount_selected && browser.state().selected==1);
    input={};input.connected=true;input.cross.pressed=true;
    assert(!browser.update(input,0).mount_selected && browser.automatic());
    input.cross.pressed=false;input.right_x=1;
    browser.update(input,0.1f);assert(!browser.automatic() && browser.state().center_yaw_deg>28);
    input={};input.connected=true;input.select.pressed=true;browser.update(input,0);
    assert(browser.state().center_yaw_deg==152);
    input={};input.connected=true;input.triangle.pressed=true;browser.update(input,0);
    assert(current_game(browser.state())->favorite && browser.state().center_yaw_deg==152);
    const auto favorite_path=current_game(browser.state())->path;
    browser.replace_catalog(games);assert(current_game(browser.state())->path==favorite_path && current_game(browser.state())->favorite);
    input={};input.connected=true;input.l1.pressed=true;browser.update(input,0);
    assert(browser.state().filter==FilterMode::Favorites && browser.state().visible.size()==1);
    input={};input.connected=true;input.triangle.pressed=true;browser.update(input,0);
    assert(browser.state().visible.empty() && browser.render_state().visible.size()==1);
    assert(current_game(browser.render_state())->cover_path.empty());
    assert(browser.hud_lines("SEM CAPA")[1]=="Nenhum jogo neste filtro");
    input={};input.connected=true;input.r1.pressed=true;browser.update(input,0);
    assert(browser.state().filter==FilterMode::All && browser.state().visible.size()==4);
    input={};input.connected=true;input.start.pressed=true;assert(browser.update(input,0).rescan_library);
    input={};input.connected=true;input.circle.pressed=true;assert(browser.update(input,0).request_exit);
    browser.replace_catalog({});assert(browser.state().games.empty());
    assert(current_game(browser.render_state())->cover_path==fixture.string());
    auto lines=browser.hud_lines("CAPA DE TESTE");
    auto bitmap=rasterize_library_hud_fix28(lines);assert(bitmap.valid() && bitmap.width==1024 && bitmap.height==192);
    auto unicode=lines;unicode[1]="Ação, coração, ê, Ã, õ";
    auto latin=lines;latin[1]="A??o, cora??o, ?, ?, ?";
    assert(rasterize_library_hud_fix28(unicode).rgba!=rasterize_library_hud_fix28(latin).rgba);
    unicode[1]=std::string(10000,'W');assert(rasterize_library_hud_fix28(unicode).rgba.size()==1024u*192*4);
    unicode[1]=std::string("\xf0\x80\x80",3);assert(rasterize_library_hud_fix28(unicode).valid());
    const auto verts=library_hud_vertices_fix28();
    for(const auto& v:verts){assert(std::fabs(v.y)>=0.73f && std::fabs(v.x)<1);assert(std::fabs(v.nx*v.nx+v.ny*v.ny+v.nz*v.nz-1)<0.0001f);}
    RsxStage1 stage;GpuTextureStage1 overlay;
    assert(!stage.prepare_overlay(bitmap,overlay));assert(stage.init());assert(stage.prepare_overlay(bitmap,overlay));
    const auto* argb=static_cast<const unsigned char*>(overlay.gpu_ptr);
    assert(argb[0]==bitmap.rgba[3] && argb[1]==bitmap.rgba[0] && argb[2]==bitmap.rgba[1] && argb[3]==bitmap.rgba[2]);
    stage.release_cover(overlay);assert(!overlay.gpu_ptr);
    RsxRendererV10 renderer;const auto mesh=build_full_cover_case_fix26();assert(renderer.init(stage,mesh,false));
    assert(renderer.set_library_hud(lines));const auto* first_hud=renderer.hud_texture_.gpu_ptr;
    assert(renderer.set_library_hud(lines) && renderer.hud_texture_.gpu_ptr==first_hud);
    lines[1]="Outra seleção";assert(renderer.set_library_hud(lines) && renderer.hud_texture_.gpu_ptr!=first_hud);
    auto state=make_safe_boot_state(fixture.string());CoverCache cache(3);
    assert(renderer.sync_visible_covers(state,cache,0) && renderer.gpu_cover_count()==1);
    assert(renderer.render(state,mesh,0));assert(renderer.last_stats().draw_calls==6 && renderer.last_stats().hud_draw_calls==1);
    state.games[0].cover_path=(dir/"missing.png").string();
    assert(renderer.sync_visible_covers(state,cache,0) && renderer.gpu_cover_count()==0);
    const auto encoded=load_cover_file(fixture.string(),GameCoverKind::FullCover);
    file(dir/"truncated.png",std::string(reinterpret_cast<const char*>(encoded.encoded.data()),33));
    state.games[0].cover_path=(dir/"truncated.png").string();
    assert(renderer.sync_visible_covers(state,cache,0) && renderer.gpu_cover_count()==0);
    state.games[0].cover_path.clear();
    assert(renderer.sync_visible_covers(state,cache,0) && renderer.gpu_cover_count()==0);
    state.games[0].cover_path=games[0].cover_path;state.games[0].cover_kind=GameCoverKind::FrontOnly;
    assert(renderer.sync_visible_covers(state,cache,0) && renderer.gpu_cover_count()==1 && !renderer.covers_[0].texture.full_cover);
    renderer.clear_cover_textures();assert(renderer.gpu_cover_count()==0 && renderer.hud_texture_.uploaded);
    renderer.shutdown();stage.shutdown();
    const auto dump=project/"build-host/FIX28_HUD_PREVIEW.rgba";
    std::ofstream(dump,std::ios::binary).write(reinterpret_cast<const char*>(bitmap.rgba.data()),bitmap.rgba.size());
    fs::remove_all(dir);
    puts("FIX28 host: HDD/USB ISO/GAMES/GAMEZ scan, SFO accents, cover priority, front-only fallback, browse/filter/empty view, session favorites/rescan, rotation and no mount command passed");
    puts("FIX28 host: bounded Latin-1 HUD raster, screen panels, RGBA/ARGB upload, unchanged HUD reuse, stale-cover release and six-case/one-HUD frame plan passed");
}
