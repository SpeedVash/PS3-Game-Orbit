#include <bits/stdc++.h>
#define private public
#include "rsx_renderer_v10.h"
#undef private
#include "library_pair_fix28.h"
#include "orbit_flow_fix31.h"
#include "performance_fix35.h"
#include "project_identity.h"

namespace fs=std::filesystem;
namespace {
std::atomic<bool> entered{false},release_gate{false};
std::atomic<unsigned> off_thread{0};
std::thread::id render_thread;
ArtworkRequestFix45 job(ArtworkLoaderFix45& loader,int index,const std::string& path){
    ArtworkRequestFix45 out;out.game_index=index;out.game_path="Game"+std::to_string(index);
    out.path=path;out.kind=GameCoverKind::FullCover;out.generation=loader.generation();return out;
}
PreparedArtworkFix45 gated(const ArtworkRequestFix45& request){
    assert(std::this_thread::get_id()!=render_thread);++off_thread;
    if(request.path=="gate"){
        entered.store(true);while(!release_gate.load())std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    PreparedArtworkFix45 out;out.request=request;out.width=2;out.height=1;out.pitch=64;
    out.argb.assign(64,std::uint8_t(request.game_index));return out;
}
template<class Predicate> void wait_for(Predicate predicate){
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(10);
    while(!predicate()){
        assert(std::chrono::steady_clock::now()<deadline);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
void worker_checks(){
    render_thread=std::this_thread::get_id();
    ArtworkLoaderFix45 loader(gated);assert(loader.start());
    assert(loader.request({job(loader,0,"gate")}));wait_for([]{return entered.load();});
    // Reading stays blocked, while the main can change priority and poll.
    wait_for([&]{return loader.request({job(loader,3,"new"),job(loader,2,"next")});});
    assert(!loader.take());
    release_gate.store(true);wait_for([&]{return loader.peak_ready()==2;});
    std::unique_ptr<PreparedArtworkFix45> first,second;
    wait_for([&]{if(!first)first=loader.take();if(!second)second=loader.take();return first && second;});
    assert(first && second && first->request.game_index==3 && second->request.game_index==2);
    assert(loader.discarded()==1 && loader.peak_ready()==2 && off_thread.load()==3);
    loader.stop();assert(!loader.running() && !loader.take());

    entered.store(false);release_gate.store(false);
    ArtworkLoaderFix45 replacement(gated);assert(replacement.start());
    assert(replacement.request({job(replacement,5,"gate")}));wait_for([]{return entered.load();});
    replacement.cancel();const auto current=job(replacement,5,"same-index-new-catalog");
    wait_for([&]{return replacement.request({current});});
    release_gate.store(true);std::unique_ptr<PreparedArtworkFix45> fresh;
    wait_for([&]{fresh=replacement.take();return bool(fresh);});
    assert(fresh->request==current && replacement.discarded()==1);
    replacement.stop();
    puts("PASS: read/decode on worker; main reprioritizes and polls during blocked I/O; ready <=2; stale/in-flight generation discarded; worker joins");

    const auto before=OrbitPerformanceFix35::metric(OrbitPerformanceFix35::Kind::Read);
    std::vector<std::thread> threads;
    for(int t=0;t<8;++t)threads.emplace_back([]{for(int i=0;i<10000;++i)OrbitPerformanceFix35::record(OrbitPerformanceFix35::Kind::Read,7);});
    for(auto& thread:threads)thread.join();
    const auto after=OrbitPerformanceFix35::metric(OrbitPerformanceFix35::Kind::Read);
    assert(after.count-before.count==80000 && after.total_us-before.total_us==560000 && after.max_us>=7);
    puts("PASS: concurrent performance counters preserve all 80000 updates");
}
void pixel_checks(const fs::path& root,RsxStage1& stage){
    for(const auto& fixture:{root/"tests/assets/full_cover_275x147.png",root/"tests/assets/cache36_vertical.png",root/"build-host/disc45.png",root/"build-host/large45.png",root/"build-host/photo45.jpg"}){
        for(auto role:{ArtworkRoleFix45::Cover,ArtworkRoleFix45::Inside,ArtworkRoleFix45::Disc}){
            ArtworkRequestFix45 request;request.path=fixture.string();request.game_path="pixels";request.role=role;
            request.kind=role==ArtworkRoleFix45::Disc?GameCoverKind::FrontOnly:GameCoverKind::FullCover;
            const auto prepared=prepare_artwork_fix45(request);assert(prepared.valid());
            const auto image=load_cover_file(request.path,request.kind);GpuTextureStage1 baseline,streamed;
            assert(role==ArtworkRoleFix45::Disc?stage.prepare_disc_artwork(image,baseline):stage.prepare_cover(image,baseline));
            assert(stage.begin_artwork_upload(prepared,streamed));assert(!streamed.uploaded);
            std::size_t copied=0,slices=0;
            while(!streamed.uploaded){
                const auto bytes=stage.advance_artwork_upload(prepared,streamed,copied,RsxRendererV10::ArtworkUploadSliceBytes);
                assert(bytes>0 && bytes<=RsxRendererV10::ArtworkUploadSliceBytes);++slices;
                if(copied<prepared.argb.size())assert(!streamed.uploaded);
            }
            assert(slices==(prepared.argb.size()+RsxRendererV10::ArtworkUploadSliceBytes-1)/RsxRendererV10::ArtworkUploadSliceBytes);
            assert(streamed.width==baseline.width && streamed.height==baseline.height && streamed.pitch==baseline.pitch && streamed.full_cover==baseline.full_cover);
            assert(std::memcmp(baseline.gpu_ptr,streamed.gpu_ptr,prepared.argb.size())==0);
            if(role==ArtworkRoleFix45::Disc)for(int y=0;y<prepared.height;++y)for(int x=0;x<prepared.width;++x)assert(prepared.argb[std::size_t(y)*prepared.pitch+x*4]==255);
            stage.release_cover(baseline);stage.release_cover(streamed);
        }
    }
    puts("PASS: exact ARGB parity with approved upload: wrap/portrait/EXIF JPEG/alpha disc/large image; opaque silver substrate; <=256 KiB per slice; no half texture exposed");
}
}
int main(int argc,char** argv){
    assert(argc==2 && std::string(ProjectIdentity::Version)=="1.6");std::setvbuf(stdout,nullptr,_IONBF,0);
    {
        HudCacheFix35 hud;LibraryHudLinesFix28 lines{};lines[0]="PS3 Game Orbit";lines[1]="Jogo";lines[4]="|CLASSIC";
        hud.update(lines,{},-1,{});const auto old=hud.image().rgba;
        lines[4]+="|DISC";const auto dirty=hud.update(lines,{},-1,{});
        assert(!dirty.empty() && hud.image().rgba!=old);
        lines[4]="|CLASSIC";hud.update(lines,{},-1,{});assert(hud.image().rgba==old);
        puts("PASS: the disc XMB button and normal mount button refresh correctly in the cached HUD");
    }

    const fs::path root=argv[1],scratch=fs::temp_directory_path()/("orbit-artwork45-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    fs::create_directories(scratch/"art");worker_checks();
    RsxStage1 stage;assert(stage.init());pixel_checks(root,stage);
    auto mesh=build_v14_case_mesh();RsxRendererV10 renderer;assert(renderer.init(stage,mesh,false));
    CoverCache cpu(15,16u<<20);CoverflowState state;
    for(int i=0;i<40;++i){
        GameEntry game;game.path="/dev_hdd0/GAMES/Game"+std::to_string(i);game.title="Jogo "+std::to_string(i);game.cover_kind=GameCoverKind::FullCover;
        const auto path=scratch/"art"/(std::to_string(i)+".png");fs::copy_file(root/"tests/assets/full_cover_275x147.png",path);
        game.cover_path=game.inside_cover_path=game.disc_art_path=path.string();state.games.push_back(game);state.visible.push_back(i);
    }
    state.games.back().source=GameSource::Disc;state.games.back().format=GameFormat::Disc;
    state.games.back().path="/dev_bdvd";state.games.back().title_id="BLES00039";
    state.games.back().library_key="/dev_bdvd/BLES00039";
    state.selected=10;state.layout=OrbitLayout::Classic;OrbitFlowFix31::reset(state);
    assert(renderer.start_streamed_artwork());
    auto bounds=[&]{
        assert(renderer.resident_limit()==15);
        std::unordered_set<int> pins;for(const auto& pose:LibraryPairFix28::poses(state,1))pins.insert(pose.game_index);
        assert(renderer.gpu_cover_count()<=std::max(std::size_t(15),pins.size()));
        assert(renderer.inside_cache_count()<=15 && renderer.disc_cache_count()<=15);
        const auto pending_bytes=gpu_texture_storage_bytes(renderer.pending_artwork_texture_);
        const auto pending_role=renderer.pending_artwork_?renderer.pending_artwork_->request.role:ArtworkRoleFix45::Cover;
        assert(renderer.gpu_cache_bytes()+(pending_role==ArtworkRoleFix45::Cover?pending_bytes:0)<=RsxRendererV10::GpuCoverBudgetBytes);
        assert(renderer.inside_cache_bytes()+(pending_role==ArtworkRoleFix45::Inside?pending_bytes:0)<=RsxRendererV10::InspectionArtBudgetBytes);
        assert(renderer.disc_cache_bytes()+(pending_role==ArtworkRoleFix45::Disc?pending_bytes:0)<=RsxRendererV10::InspectionArtBudgetBytes);
        assert(renderer.last_artwork_upload_bytes()<=RsxRendererV10::ArtworkUploadSliceBytes);
        assert(renderer.artwork_loader_->peak_ready()<=2);
    };
    auto tick=[&]{
        renderer.apply_cache_policy(state,cpu);assert(renderer.service_streamed_artwork(state));
        assert(renderer.sync_visible_covers(state,cpu,1));assert(renderer.sync_inspection_art(state,true));
        assert(renderer.render(state,mesh,1));bounds();
    };
    auto warm=[&]{wait_for([&]{tick();const auto p=renderer.streamed_preload_progress(state);return p.first==p.second;});};
    warm();assert(renderer.streamed_preload_progress(state)==std::make_pair(std::size_t(45),std::size_t(45)));
    assert(renderer.gpu_cover_count()==15 && renderer.inside_cache_count()==15 && renderer.disc_cache_count()==15);
    const auto reads=OrbitPerformanceFix35::metric(OrbitPerformanceFix35::Kind::Read).count;
    const auto decodes=OrbitPerformanceFix35::metric(OrbitPerformanceFix35::Kind::Decode).count;
    std::unordered_map<int,void*> cover_ptr,inside_ptr,disc_ptr;
    for(int i=3;i<=17;++i){cover_ptr[i]=renderer.covers_.at(i).texture.gpu_ptr;inside_ptr[i]=renderer.inside_cache_.entries.at(i).texture.gpu_ptr;disc_ptr[i]=renderer.disc_cache_.entries.at(i).texture.gpu_ptr;}
    fs::rename(scratch/"art",scratch/"hidden");
    for(int n=0;n<100;++n){
        state.layout=n%3==0?OrbitLayout::Spine:n%3==1?OrbitLayout::List:OrbitLayout::Classic;OrbitFlowFix31::advance(state,1.0f/60);
        tick();
        for(int i=3;i<=17;++i){assert(renderer.covers_.at(i).texture.gpu_ptr==cover_ptr[i]);assert(renderer.inside_cache_.entries.at(i).texture.gpu_ptr==inside_ptr[i]);assert(renderer.disc_cache_.entries.at(i).texture.gpu_ptr==disc_ptr[i]);}
    }
    assert(OrbitPerformanceFix35::metric(OrbitPerformanceFix35::Kind::Read).count==reads && OrbitPerformanceFix35::metric(OrbitPerformanceFix35::Kind::Decode).count==decodes);
    fs::rename(scratch/"hidden",scratch/"art");
    puts("PASS: startup 45/45 images; common 15/15/15 retained through 100 layout changes; same GPU pointers and zero new reads/decodes with source files removed");

    for(int n=0;n<200;++n){
        state.selected=(10+n)%40;state.navigation_direction=n%2?1:-1;
        state.layout=n%3==0?OrbitLayout::Spine:n%3==1?OrbitLayout::List:OrbitLayout::Classic;
        OrbitFlowFix31::advance(state,1.0f/60);tick();
    }
    state.selected=30;state.layout=OrbitLayout::Spine;OrbitFlowFix31::reset(state);warm();
    assert(renderer.has_cover_texture(30) && renderer.has_inside_texture(30) && renderer.has_disc_art_texture(30));
    state.inspection_target=true;state.inspection_phase=1;tick();
    assert(renderer.texture_for_art_role(V14MeshPart::TextureRole::Inside,30)==&renderer.inside_cache_.entries.at(30).texture);
    puts("PASS: rapid navigation/wrapping/fading/layout changes; background queue settles on selected game and correct inside/disc binding within role byte budgets");

    const auto resident=renderer.gpu_cover_count();auto manager=state;manager.visible.clear();manager.flow.clear();manager.flow_initialized=false;
    assert(renderer.service_streamed_artwork(manager));assert(renderer.gpu_cover_count()==resident);
    tick();
    const auto decode_before=renderer.artwork_loader_->decoded();renderer.invalidate_game_art(30,2);
    assert(renderer.has_cover_texture(30) && !renderer.has_inside_texture(30) && renderer.has_disc_art_texture(30));
    warm();assert(renderer.artwork_loader_->decoded()>decode_before && renderer.has_inside_texture(30));
    auto& game=state.games[30];game.cover_path=(scratch/"missing.png").string();renderer.invalidate_game_art(30,1);
    warm();assert(renderer.streamed_cover_failed(30,game) && !renderer.has_cover_texture(30) && renderer.has_inside_texture(30) && renderer.has_disc_art_texture(30));
    game.cover_path=game.inside_cover_path;renderer.invalidate_game_art(30,1);warm();assert(renderer.has_cover_texture(30));
    renderer.clear_cover_textures();state.games[30].path="replacement-catalog";warm();assert(renderer.covers_.at(30).game_path=="replacement-catalog");
    puts("PASS: file manager pauses without clearing cache; per-role reload; missing artwork completes with neutral fallback; catalog replacement cannot bind stale results");

    CoverflowState empty;renderer.apply_cache_policy(empty,cpu);assert(renderer.service_streamed_artwork(empty));
    assert(renderer.streamed_preload_progress(empty)==std::make_pair(std::size_t(0),std::size_t(0)) && renderer.gpu_cover_count()==0);
    renderer.stop_streamed_artwork();assert(!renderer.streamed_artwork_active());renderer.shutdown();stage.shutdown();
    fs::remove_all(scratch);puts("PASS: empty library skips warmup; worker stops before decoder/GPU shutdown");
}
