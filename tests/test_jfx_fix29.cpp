#include <cassert>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <filesystem>
#include "jfx_case_fix29.h"
#include "rsx_renderer_v10.h"
#include "case_render_fix25.h"
#include "library_pair_fix28.h"
#include "library_browser_fix28.h"
#include "preferences_fix29.h"

int main(){
    const auto mesh=JfxCaseFix29::build();assert(mesh.jfx && mesh.parts.size()==5);
    unsigned triangles=0;
    for(const auto& p:mesh.parts){triangles+=p.indices.size()/3;for(auto i:p.indices) assert(i<p.vertices.size());}
    assert(triangles==5597 && mesh.parts[0].indices.size()/3==2642 && mesh.parts[4].indices.size()/3==2925);
    assert((mesh.parts[0].color==std::array<float,4>{{1,1,1,.14f}}));
    for(std::size_t i=1;i<4;++i) for(float u:{.25f,.5f,.75f}) for(float v:{.25f,.5f,.75f}){
        std::array<float,3> point;assert(JfxCaseFix29::surface_point(mesh.parts[i],u,v,point));
        assert(std::fabs(point[1])<85 && std::fabs(point[2])<7);
    }
    const auto full=compute_native_uv_transform(mesh,V14Surface::CoverBack,true);
    assert(full.scale_u==1 && full.bias_u==0 && full.scale_v==1);
    const auto front=compute_native_uv_transform(mesh,V14Surface::CoverFront,false);
    assert(std::fabs(JfxCaseFix29::FrontBegin*front.scale_u+front.bias_u)<1e-6f);
    assert(std::fabs(front.scale_u+front.bias_u-1)<1e-6f);
    CoverflowState state;state.games.resize(2);state.visible={0,1};
    for(int i=0;i<2;++i){state.games[i].path="/dev_hdd0/PS3ISO/BLES0123"+std::to_string(i)+".iso";state.games[i].title="Game "+std::to_string(i);}
    state.center_yaw_deg=28;state.center_pitch_deg=-5;
    for(int yaw=0;yaw<360;yaw+=5){
        state.center_yaw_deg=float(yaw);
        const auto plan=build_v10_frame_plan(LibraryPairFix28::poses(state,1),mesh);
        assert(plan.packets.size()==12);
        const auto order=native_submission_order(plan,mesh);assert(order.size()==12);
        unsigned far=0,near=0;bool plastic=false;
        for(auto index:order){const auto& p=plan.packets[index];assert(p.mesh_part<5);
            if(p.plastic_pass){plastic=true;if(p.plastic_pass==1) ++far;else ++near;}
            else assert(!plastic);
        }
        assert(far==2 && near==2);
        const auto stats=summarize_v10_submission(plan,mesh);assert(stats.draw_calls==12 && stats.visible_cases==2);
    }
    puts("PASS: 72 two-case poses, original mesh counts, neutral plastic, opaque-first/two-pass order, readable original back and front-only UV transform");
    LibraryBrowserFix28 browser("");browser.replace_catalog(state.games);
    InputFrame input;input.connected=true;input.cross.pressed=true;
    auto cmd=browser.update(input,.016f);assert(cmd.mount_selected && !browser.automatic());
    input={};input.connected=true;input.up.pressed=true;browser.update(input,.016f);assert(browser.automatic());
    input={};input.connected=true;input.right.pressed=true;browser.update(input,.016f);
    assert(browser.state().selected==1 && browser.state().transition==0);
    const auto entering=LibraryPairFix28::poses(browser.state(),1);assert(entering.size()==2 && entering[0].x==185);
    input={};input.connected=true;for(int i=0;i<15;++i) browser.update(input,.016f);
    assert(browser.state().transition==1 && LibraryPairFix28::poses(browser.state(),1)[0].x==0);
    browser.set_operation_status("aguarde",true);input.right.pressed=true;input.cross.pressed=true;input.start.pressed=true;input.l3.pressed=true;
    cmd=browser.update(input,.016f);assert(!cmd.mount_selected && !cmd.rescan_library && browser.state().selected==1 && current_game(browser.state())->cover_orientation==1);
    input={};input.connected=true;input.circle.pressed=true;assert(browser.update(input,.016f).request_exit);
    puts("PASS: X mounts, up toggles auto rotation, bounded transition settles; selection/rescan/orientation remain frozen during mount and circle exits");
    namespace fs=std::filesystem;const auto root=fs::temp_directory_path()/"ps3-sp-fix29-preferences";fs::remove_all(root);fs::create_directory(root);
    const auto path=(root/"state.dat").string();
    PreferencesFix29 prefs;assert(!prefs.load(path));state.filter=FilterMode::USB;
    state.games[0].favorite=true;state.games[0].cover_orientation=6;
    state.games[1].path="/dev_usb000/GAMES/Jogo ç [BLES01234]";state.games[1].favorite=true;state.games[1].cover_orientation=3;
    prefs.capture(state);assert(prefs.save(path));PreferencesFix29 loaded;assert(loaded.load(path));
    auto games=state.games;for(auto& g:games){g.favorite=false;g.cover_orientation=1;}loaded.apply(games);
    assert(games[0].favorite && games[0].cover_orientation==6 && games[1].favorite && games[1].cover_orientation==3 && loaded.filter()==FilterMode::USB);
    auto absent=state;absent.games.pop_back();absent.visible={0};loaded.capture(absent);assert(loaded.save(path));PreferencesFix29 restored;assert(restored.load(path));restored.apply(games);assert(games[1].favorite && games[1].cover_orientation==3);
    std::ofstream(path,std::ios::binary|std::ios::trunc)<<"PS3_SP_PREFS29_V1 0\nfilter 8\n";assert(!restored.load(path));restored.apply(games);assert(games[1].favorite);
    assert(!restored.save((root/"missing/state.dat").string()));fs::remove_all(root);
    puts("PASS: atomic preferences round-trip, accents, orientations, saved filter/selection, disconnected USB retention, corrupt-file rejection and write failure");
}
