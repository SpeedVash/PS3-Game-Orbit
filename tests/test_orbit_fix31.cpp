#include <cassert>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include "library_browser_fix28.h"
#include "orbit_flow_fix31.h"
#include "layout_settings_fix31.h"
#include "preferences_fix29.h"
#include "cover_limits_fix31.h"
#include "cover_cache.h"
#include "rsx_renderer_v10.h"
#include "jfx_case_fix29.h"

static std::vector<GameEntry> catalog(int n) {
    std::vector<GameEntry> games(n);
    for(int i=0;i<n;++i) {games[i].path="/dev_hdd0/PS3ISO/"+std::to_string(i)+".iso";games[i].title="Jogo "+std::to_string(i);}
    return games;
}
static void settle(LibraryBrowserFix28& browser) {
    InputFrame pad;pad.connected=true;for(int i=0;i<150;++i) browser.update(pad,1.0f/60);
}
static void invariant(const std::vector<CasePose>& poses,int count) {
    assert(poses.size()<=OrbitFlowFix31::MaxCases);std::set<int> games;unsigned selected=0;
    for(const auto& p:poses) {
        assert(p.game_index>=0 && p.game_index<count && games.insert(p.game_index).second);
        assert(std::isfinite(p.x) && std::isfinite(p.yaw_deg) && p.visibility>=0 && p.visibility<=1);
        assert(p.scale>=.55f && p.scale<=.8f);
        if(p.selected) ++selected;
    }
    assert(poses.empty() || selected==1);
}
static void continuity(const std::vector<CasePose>& before,const std::vector<CasePose>& after) {
    for(const auto& p:before) if(p.visibility>0) {
        bool found=false;
        for(const auto& q:after) if(p.game_index==q.game_index) {
            found=true;
            const bool same=p.x==q.x && p.y==q.y && p.z==q.z && p.yaw_deg==q.yaw_deg && p.pitch_deg==q.pitch_deg && p.scale==q.scale && p.visibility==q.visibility;
            if(!same) std::fprintf(stderr,"Continuity game %d: xyz %g/%g/%g -> %g/%g/%g; angles %g/%g -> %g/%g; scale %g -> %g; visibility %g -> %g\n",p.game_index,p.x,p.y,p.z,q.x,q.y,q.z,p.yaw_deg,p.pitch_deg,q.yaw_deg,q.pitch_deg,p.scale,q.scale,p.visibility,q.visibility);
            assert(same);
        }
        assert(found);
    }
    for(const auto& q:after) {
        bool old=false;for(const auto& p:before) if(p.game_index==q.game_index) old=true;
        if(!old) assert(q.visibility==0);
    }
}
int main(int argc,char** argv) {
    std::setvbuf(stdout,nullptr,_IONBF,0);
    assert(argc==2);const std::string root=argv[1];
    LibraryBrowserFix28 browser("");browser.replace_catalog(catalog(40));
    assert(browser.scale()==.55f && browser.state().layout==OrbitLayout::Classic);
    auto initial=OrbitFlowFix31::poses(browser.state());assert(initial.size()==2);
    for(const auto& p:initial) assert(p.scale==.55f);
    InputFrame pad;pad.connected=true;pad.right.pressed=true;
    browser.update(pad,0);auto changed=OrbitFlowFix31::poses(browser.state());
    continuity(initial,changed);assert(changed.size()==3 && browser.state().selected==1);
    pad.right.pressed=false;browser.update(pad,.05f);changed=OrbitFlowFix31::poses(browser.state());
    bool old_fading=false,new_entering=false;
    for(const auto& p:changed) {if(p.game_index==0) old_fading=p.visibility>0 && p.visibility<1;if(p.game_index==2) new_entering=p.visibility>0 && p.visibility<1;}
    assert(old_fading && new_entering);
    const auto intermediate=changed;pad.left.pressed=true;browser.update(pad,0);
    continuity(intermediate,OrbitFlowFix31::poses(browser.state()));
    pad.left.pressed=false;settle(browser);assert(OrbitFlowFix31::poses(browser.state()).size()==2);
    puts("PASS: Classic starts at .55; both covers animate, exits survive, reversing mid-movement preserves every visible pose and opacity");
    initial=OrbitFlowFix31::poses(browser.state());pad.square.pressed=true;browser.update(pad,0);
    assert(browser.state().layout==OrbitLayout::Spine);continuity(initial,OrbitFlowFix31::poses(browser.state()));
    pad.square.pressed=false;settle(browser);const auto spine=OrbitFlowFix31::poses(browser.state());
    assert(spine.size()==11);invariant(spine,40);
    unsigned left=0,right=0;
    for(const auto& p:spine) {
        assert(p.visibility==1 && p.scale==.55f);
        if(p.selected) assert(p.x==0 && p.z==55 && p.yaw_deg==0);
        else {assert(p.z<-160 && p.pitch_deg==0 && p.yaw_deg>85 && p.yaw_deg<98);if(p.x<0) ++left;else ++right;}
    }
    assert(left==5 && right==5);
    puts("PASS: Spine shows five original-UV spines each side and a front-facing selected cover, with the Aurora Normal angles and spacing");
    for(int i=0;i<1200;++i) {
        pad={};pad.connected=true;pad.right.pressed=true;
        if(i%17==0) pad.left.pressed=true;
        const auto previous=OrbitFlowFix31::poses(browser.state());browser.update(pad,0);
        const auto current=OrbitFlowFix31::poses(browser.state());continuity(previous,current);invariant(current,40);
        pad.right.pressed=pad.left.pressed=false;browser.update(pad,.001f);invariant(OrbitFlowFix31::poses(browser.state()),40);
    }
    settle(browser);assert(OrbitFlowFix31::poses(browser.state()).size()==11);
    puts("PASS: 1,200 rapid navigation requests preserve existing visible cases, respect the fourteen-case pool and settle without a stale exit");
    pad={};pad.connected=true;pad.r2.held=true;for(int i=0;i<20;++i) browser.update(pad,.1f);
    assert(browser.scale()==.8f);pad.r2.held=false;pad.r3.pressed=true;browser.update(pad,0);
    assert(browser.scale()==.55f && browser.state().center_yaw_deg==0);
    pad={};pad.connected=true;pad.select.pressed=true;const auto layout=browser.state().layout;browser.update(pad,0);
    assert(browser.help_open() && browser.state().layout==layout);
    browser.set_operation_status("aguarde",true);pad={};pad.connected=true;pad.square.pressed=pad.right.pressed=pad.cross.pressed=pad.start.pressed=true;
    const int selected=browser.state().selected;auto command=browser.update(pad,.016f);
    assert(browser.state().layout==layout && browser.state().selected==selected && !command.mount_selected && !command.rescan_library);
    puts("PASS: R3 restores minimum zoom and the layout front pose; SELECT opens help; mount freezes layout, game changes and rescan");
    for(int n=0;n<=11;++n) {
        LibraryBrowserFix28 small("");small.replace_catalog(catalog(n));small.restore_layout(OrbitLayout::Spine);
        const auto render=small.render_state();const auto poses=OrbitFlowFix31::poses(render);
        assert(poses.size()==std::size_t(n ? n : 1));invariant(poses,n ? n : 1);
        if(n==0) assert(render.games.front().cover_path.empty());
    }
#ifndef PS3_GAME_ORBIT_FLOW_ONLY
    const auto mesh=JfxCaseFix29::build();const auto plan=build_v10_frame_plan(spine,mesh);
    assert(plan.packets.size()==66 && mesh.parts.size()==5);
    unsigned triangles=0;for(const auto& p:mesh.parts) triangles+=p.indices.size()/3;assert(triangles==5597);
    auto uv=compute_native_uv_transform(mesh,V14Surface::CoverBack,true);assert(uv.scale_u==1 && uv.bias_u==0);
    puts("PASS: empty and short libraries have no duplicated games; eleven cases use the exact 5,597-triangle approved model and unchanged readable back UVs");
    RsxStage1 stage;assert(stage.init());RsxRendererV10 renderer;assert(renderer.init(stage,mesh,false));
    CoverCache gpu_cache(15,CoverLimitsFix31::EncodedCacheBytes);LibraryBrowserFix28 textures("");auto covered=catalog(20);
    for(auto& game:covered) {game.cover_path=root+"/pkgfiles/USRDIR/FULL_COVER_FIX26_CONTINUA.png";game.cover_kind=GameCoverKind::FullCover;}
    textures.replace_catalog(covered);assert(renderer.sync_visible_covers(textures.render_state(),gpu_cache,1) && renderer.gpu_cover_count()==2);
    InputFrame next;next.connected=true;next.right.pressed=true;textures.update(next,0);
    assert(renderer.sync_visible_covers(textures.render_state(),gpu_cache,1) && renderer.gpu_cover_count()==3 && renderer.has_cover_texture(0));
    next.right.pressed=false;textures.update(next,.1f);
    assert(renderer.sync_visible_covers(textures.render_state(),gpu_cache,1) && renderer.has_cover_texture(0));
    textures.update(next,.1f);assert(renderer.sync_visible_covers(textures.render_state(),gpu_cache,1) && !renderer.has_cover_texture(0) && renderer.gpu_cover_count()==2);
    textures.restore_layout(OrbitLayout::Spine);assert(renderer.sync_visible_covers(textures.render_state(),gpu_cache,1) && renderer.gpu_cover_count()==11);
    renderer.shutdown();stage.shutdown();
    puts("PASS: actual texture synchronization retains the outgoing game's GPU copy until its fade completes, releases it afterwards and loads eleven Spine covers");
#else
    puts("PASS: empty and short libraries contain no repeated games; this CI subset does not need the licensed model");
#endif
    namespace fs=std::filesystem;const auto temp=fs::temp_directory_path()/"orbit-fix31-test";fs::remove_all(temp);fs::create_directory(temp);
    auto settings_path=(temp/"layout.dat").string();LayoutSettingsFix31 settings;assert(!settings.load(settings_path));settings.capture(OrbitLayout::Spine);assert(settings.save(settings_path));
    LayoutSettingsFix31 restored;assert(restored.load(settings_path) && restored.layout()==OrbitLayout::Spine);
    for(const std::string& bad:std::vector<std::string>{"","PS3_GAME_ORBIT_LAYOUT_V1\nunknown\n","PS3_GAME_ORBIT_LAYOUT_V1\nspine\nextra",std::string(4096,'x')}) {
        std::ofstream(settings_path,std::ios::binary|std::ios::trunc)<<bad;assert(!restored.load(settings_path) && restored.layout()==OrbitLayout::Spine);
    }
    assert(!settings.save((temp/"missing/layout.dat").string()));
    PreferencesFix29 prefs;auto old=catalog(2);old[0].favorite=true;CoverflowState state;state.games=old;state.visible={0,1};prefs.capture(state);
    auto favorites_path=(temp/"state.dat").string();assert(prefs.save(favorites_path));
    settings.save(settings_path);PreferencesFix29 legacy;assert(legacy.load(favorites_path));old[0].favorite=false;legacy.apply(old);assert(old[0].favorite);
    puts("PASS: bounded atomic layout settings reject corrupt data; 1.0 favorites remain readable and independent of the new sidecar");
    DecodedImageRGBA raster;raster.width=2048;raster.height=1024;raster.pitch=raster.width*4;raster.rgba.resize(std::size_t(raster.pitch)*raster.height);
    for(int y=0;y<raster.height;++y) for(int x=0;x<raster.width;++x) {const auto k=std::size_t(y)*raster.pitch+x*4;raster.rgba[k]=x<raster.width/2 ? 240 : 20;raster.rgba[k+1]=y<raster.height/2 ? 30 : 220;raster.rgba[k+2]=70;raster.rgba[k+3]=255;}
    assert(CoverLimitsFix31::fit_texture(raster) && raster.width==1024 && raster.height==512);
    assert(raster.rgba[0]==240 && raster.rgba[1023*4]==20 && raster.rgba[511*raster.pitch+1]==220 && raster.rgba.back()==255);
    auto first=load_cover_file(root+"/pkgfiles/USRDIR/FULL_COVER_FIX26_CONTINUA.png",GameCoverKind::FullCover);
    auto second=load_cover_file(root+"/pkgfiles/USRDIR/FULL_COVER_FIX28_SEGUNDA.png",GameCoverKind::FullCover);
    CoverCache cache(15,std::max(first.encoded.size(),second.encoded.size())+1);GameEntry a,b;a.cover_path=first.path;a.cover_kind=first.kind;b.cover_path=second.path;b.cover_kind=second.kind;
    assert(cache.get_or_load(a) && cache.get_or_load(b));assert(cache.size()==1 && cache.encoded_bytes()<=std::max(first.encoded.size(),second.encoded.size())+1);
    cache.clear();assert(cache.size()==0 && cache.encoded_bytes()==0);fs::remove_all(temp);
    puts("PASS: bounded cover cache and 1024-edge thumbnails preserve the complete atlas, aspect, color regions and alpha");
}
