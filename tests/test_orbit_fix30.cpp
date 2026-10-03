#include <cassert>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <string>
#include <algorithm>
#include "orbit_ui_fix30.h"
#include "library_browser_fix28.h"
#include "library_hud_fix28.h"
#include "cover_orientation_fix28.h"
#include "preferences_fix29.h"
#include "rsx_stage1.h"
#include "image_decode.h"
#include "project_identity.h"

static void save(const char* path,const DecodedImageRGBA& p){std::ofstream f(path,std::ios::binary);f.write(reinterpret_cast<const char*>(p.rgba.data()),p.rgba.size());}
int main(int argc,char** argv){
    assert(argc==3);const std::string root=argv[1],out=argv[2];
    LibraryBrowserFix28 browser("");std::vector<GameEntry> games(2);
    games[0].title="NINJA GAIDEN Σ";games[0].path="/dev_hdd0/GAMES/Ninja Gaiden Sigma [BLES00072]";games[0].title_id="BLES00072";games[0].cover_orientation=6;
    games[1].title="DiRT 3";games[1].path="/dev_hdd0/GAMES/DiRT 3 [BLES01287]";
    browser.replace_catalog(games);assert(browser.state().games[0].cover_orientation==1);
    InputFrame pad;pad.connected=true;pad.right_x=1;browser.update(pad,.1f);const auto yaw=browser.state().center_yaw_deg;
    pad.right_x=0;pad.select.pressed=true;browser.update(pad,0);assert(browser.help_open() && browser.state().center_yaw_deg==yaw);
    pad.select.pressed=false;pad.l3.pressed=pad.square.pressed=true;browser.update(pad,0);
    assert(browser.state().games[0].cover_orientation==1 && browser.state().center_yaw_deg==yaw);
    pad={};pad.connected=true;pad.select.pressed=true;browser.update(pad,0);assert(!browser.help_open());
    pad.select.pressed=false;pad.up.pressed=true;browser.update(pad,.1f);assert(browser.automatic());
    pad.up.pressed=false;pad.r3.pressed=true;browser.update(pad,0);assert(!browser.automatic() && browser.state().center_yaw_deg==28);
    pad.r3.pressed=false;pad.cross.pressed=true;assert(browser.update(pad,0).mount_selected);
    const auto lines=browser.hud_lines("CAPA INTEIRA");assert(lines[0]=="PS3 Game Orbit" && lines[1]=="NINJA GAIDEN Σ");
    for(const auto& line:lines){assert(line.find("SELECT: verso")==std::string::npos && line.find("NORMAL")==std::string::npos && line.find("CAPA INTEIRA")==std::string::npos);}
    puts("PASS: normal artwork default, removed manual orientation/front/back shortcuts, SELECT help preserves rotation, UP autogiro, R3 reset and X mount");
    const auto hud=OrbitUiFix30::hud(lines);assert(hud.width==1280 && hud.height==384);
    bool intermediate=false;unsigned ink=0;
    for(std::size_t i=3;i<hud.rgba.size();i+=4){if(hud.rgba[i]>0 && hud.rgba[i]<255) intermediate=true;if(hud.rgba[i])++ink;}
    assert(intermediate && ink<1280*176/5);
    for(std::size_t i=176*hud.pitch+3;i<hud.rgba.size();i+=4) assert(hud.rgba[i]==0);
    save((out+"/HUD.rgba").c_str(),hud);
    pad.cross.pressed=false;pad.select.pressed=true;browser.update(pad,0);
    const auto help=OrbitUiFix30::hud(browser.hud_lines(""));assert(help.rgba[178*help.pitch+54*4+3]>0);save((out+"/AJUDA.rgba").c_str(),help);
    auto long_lines=lines;long_lines[1]=std::string(8000,'W')+" São Paulo Σ";long_lines[3]="\xf0\x28\x8c\x28";
    const auto bounded=OrbitUiFix30::hud(long_lines);assert(bounded.valid());
    puts("PASS: antialiased UTF-8 Latin/Greek text, sparse transparent chrome, hidden help area and bounded long/malformed text");
    const auto vertices=library_hud_vertices_fix28();assert(vertices.size()==12 && LibraryHudIndicesFix28.size()==18);
    for(const auto& v:vertices) assert(v.x>=-1 && v.x<=1 && v.y>=-1 && v.y<=1 && v.u>=0 && v.u<=1 && v.v>=0 && v.v<=1);
    const auto bg=OrbitUiFix30::background();assert(bg.width==1280 && bg.height==360 && bg.valid());save((out+"/FUNDO.rgba").c_str(),bg);
    const auto fallback=OrbitUiFix30::splash();assert(fallback.width==1280 && fallback.height==720);
    for(std::size_t i=0;i<fallback.rgba.size();i+=4) assert(!(fallback.rgba[i]>240 && fallback.rgba[i+1]<32 && fallback.rgba[i+2]>240));
    auto splash_file=load_cover_file(root+"/pkgfiles/USRDIR/ORBIT_SPLASH.png",GameCoverKind::None);
    DecodedImageRGBA art;std::string error;assert(decode_cover_rgba(splash_file,art,error));
    const auto splash=OrbitUiFix30::splash(&art);save((out+"/INICIO.rgba").c_str(),splash);
    puts("PASS: wave background, three screen-bounded HUD quads, branded startup and neutral missing-artwork startup fallback");
    auto normal=load_cover_file(root+"/pkgfiles/USRDIR/FULL_COVER_FIX26_CONTINUA.png",GameCoverKind::FullCover);
    auto portrait=load_cover_file(root+"/tests/fixtures/orbit30_full_portrait.png",GameCoverKind::FullCover);
    assert(default_cover_orientation_fix30(normal)==1 && default_cover_orientation_fix30(portrait)==6);
    auto front=portrait;front.kind=GameCoverKind::FrontOnly;assert(default_cover_orientation_fix30(front)==1);
    RsxStage1 stage;assert(stage.init());GpuTextureStage1 tex;assert(stage.prepare_cover(portrait,tex,2));
    DecodedImageRGBA expected;assert(decode_cover_rgba(normal,expected,error));
    assert(tex.full_cover && tex.width==expected.width && tex.height==expected.height);
    const auto* bytes=static_cast<const unsigned char*>(tex.gpu_ptr);
    for(int y=0;y<tex.height;++y) for(int x=0;x<tex.width;++x){const auto* a=bytes+std::size_t(y)*tex.pitch+x*4;const auto* b=expected.rgba.data()+std::size_t(y)*expected.pitch+x*4;assert(a[0]==b[3] && a[1]==b[0] && a[2]==b[1] && a[3]==b[2]);}
    stage.release_cover(tex);stage.shutdown();
    puts("PASS: portrait full wrap normalizes automatically; every uploaded ARGB pixel matches normal full artwork; front-only artwork remains front-only");
    LibraryBrowserFix28 empty(root+"/pkgfiles/USRDIR/FULL_COVER_FIX26_CONTINUA.png",root+"/pkgfiles/USRDIR/FULL_COVER_FIX28_SEGUNDA.png");
    const auto blank=empty.render_state();assert(empty.state().games.empty() && blank.games.size()==1 && blank.games.front().cover_path.empty());
    puts("PASS: empty library shows a neutral blank case, no colored diagnostic covers and no mountable demo entries");
}
