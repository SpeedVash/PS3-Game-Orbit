#include <bits/stdc++.h>
#define private public
#include "rsx_renderer_v10.h"
#undef private
#include "library_browser_fix28.h"
#include "library_pair_fix28.h"
#include "orbit_flow_fix31.h"
#include "prefetch_policy_fix36.h"

namespace fs=std::filesystem;
#ifdef PS3_GAME_ORBIT_FIX44
static constexpr bool Improved=true;
#else
static constexpr bool Improved=false;
#endif

int main(int argc,char** argv){
    assert(argc==2);std::setvbuf(stdout,nullptr,_IONBF,0);
    const fs::path root=argv[1],scratch=fs::temp_directory_path()/("orbit-cache44-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    fs::create_directories(scratch);
    CoverflowState state;
    for(int i=0;i<32;++i){
        GameEntry g;g.path="/dev_hdd0/GAMES/Game"+std::to_string(i);g.title="Jogo "+std::to_string(i);
        const auto image=scratch/(std::to_string(i)+".png");
        fs::copy_file(root/"tests/assets/full_cover_275x147.png",image);
        g.cover_path=g.inside_cover_path=g.disc_art_path=image.string();g.cover_kind=GameCoverKind::FullCover;
        state.games.push_back(g);state.visible.push_back(i);
    }
    auto mesh=build_v14_case_mesh();RsxStage1 stage;assert(stage.init());
    RsxRendererV10 renderer;assert(renderer.init(stage,mesh,false));CoverCache cache(15,16u<<20);
    auto view=[&](OrbitLayout layout,int selected){state.layout=layout;state.selected=selected;state.inspection_phase=0;state.inspection_target=false;OrbitFlowFix31::reset(state);renderer.apply_cache_policy(state,cache);};
    auto prepare=[&](){assert(renderer.sync_visible_covers(state,cache,1));assert(renderer.sync_inspection_art(state,true));};
    auto warm=[&](){for(int n=0;n<40;++n){renderer.prefetch_nearby_cover(state,cache);renderer.prefetch_inspection_art(state);}};
    auto check_bounds=[&](){
        std::unordered_set<int> pinned;for(const auto& p:LibraryPairFix28::poses(state,1))pinned.insert(p.game_index);
        assert(renderer.gpu_cover_count()<=std::max(renderer.resident_limit(),pinned.size()));
        assert(renderer.inside_cache_count()<=renderer.resident_limit() && renderer.disc_cache_count()<=renderer.resident_limit());
        assert(renderer.gpu_cache_bytes()<=RsxRendererV10::GpuCoverBudgetBytes);
        assert(renderer.inside_cache_bytes()<=RsxRendererV10::InspectionArtBudgetBytes && renderer.disc_cache_bytes()<=RsxRendererV10::InspectionArtBudgetBytes);
        assert(cache.size()<=renderer.resident_limit());
    };

    // A short layout/navigation round trip must reuse all three image roles
    // when the previous game still fits inside the destination cache.
    view(OrbitLayout::Classic,10);prepare();
    const auto front=renderer.covers_.at(10).texture.gpu_ptr;
    const auto inside=renderer.inside_cache_.entries.at(10).texture.gpu_ptr;
    const auto disc=renderer.disc_cache_.entries.at(10).texture.gpu_ptr;
    view(OrbitLayout::List,17);prepare();
    assert(renderer.has_cover_texture(10)==Improved);
    assert(renderer.has_inside_texture(10)==Improved && renderer.has_disc_art_texture(10)==Improved);
    const auto misses=renderer.gpu_cache_misses()+renderer.inside_cache_misses()+renderer.disc_cache_misses();
    view(OrbitLayout::Classic,10);prepare();
    const auto return_misses=renderer.gpu_cache_misses()+renderer.inside_cache_misses()+renderer.disc_cache_misses()-misses;
    assert(return_misses==(Improved?0u:4u));
    if(Improved){assert(renderer.covers_.at(10).texture.gpu_ptr==front);assert(renderer.inside_cache_.entries.at(10).texture.gpu_ptr==inside);assert(renderer.disc_cache_.entries.at(10).texture.gpu_ptr==disc);}
    check_bounds();
    std::cout<<"PASS: layout/navigation round trip; additional image preparations="<<return_misses<<"\n";

    // Resize the actual encoded cache after making the nearest five files
    // its oldest entries. Removing their source bytes proves memory reuse.
    CoverCache cpu(15);
    for(int i=0;i<15;++i)assert(cpu.get_or_load(state.games[std::size_t(i)]));
#ifdef PS3_GAME_ORBIT_FIX44
    std::unordered_set<std::string> preferred;
    for(int i=0;i<5;++i)preferred.insert(state.games[std::size_t(i)].cover_path);
    cpu.set_limit(5,preferred);
#else
    cpu.set_limit(5);
#endif
    unsigned retained=0;
    for(int i=0;i<5;++i){const fs::path path=state.games[std::size_t(i)].cover_path,hidden=path.string()+".hidden";
        fs::rename(path,hidden);const auto* image=cpu.get_or_load(state.games[std::size_t(i)]);
        if(image && image->valid())++retained;
        fs::rename(hidden,path);
    }
    assert(retained==(Improved?5u:0u));assert(cpu.size()==5);
    const auto* loaded=cpu.get_or_load(state.games[5]);assert(loaded && loaded->valid() && loaded->path==state.games[5].cover_path && cpu.size()==5);
    std::cout<<"PASS: encoded cache shrink; common files retained without disk reads="<<retained<<"/5; inserted image remains valid\n";

    // A fully populated Spine cache shrinks to the nearest five, preserving
    // existing texture allocations and the selected interior/disc.
    renderer.clear_cover_textures();cache.clear();view(OrbitLayout::Spine,10);prepare();warm();
    std::vector<int> candidates{10};const auto neighbors=PrefetchPolicyFix36::candidates(state);candidates.insert(candidates.end(),neighbors.begin(),neighbors.end());
    for(int gi:candidates){const auto& game=state.games[std::size_t(gi)];
        assert(renderer.load_cached_inspection_art(renderer.inside_cache_,gi,game,game.inside_cover_path,GameCoverKind::FullCover));
        assert(renderer.load_cached_inspection_art(renderer.disc_cache_,gi,game,game.disc_art_path,GameCoverKind::FrontOnly));
    }
    assert(renderer.gpu_cover_count()==15 && renderer.inside_cache_count()==15 && renderer.disc_cache_count()==15);
    std::unordered_map<int,std::array<void*,3>> pointers;
    for(int gi=8;gi<=12;++gi)pointers[gi]={{renderer.covers_.at(gi).texture.gpu_ptr,renderer.inside_cache_.entries.at(gi).texture.gpu_ptr,renderer.disc_cache_.entries.at(gi).texture.gpu_ptr}};
    view(OrbitLayout::List,10);
    assert(renderer.gpu_cover_count()==5 && renderer.inside_cache_count()==5 && renderer.disc_cache_count()==5);
    for(const auto& item:pointers){const int gi=item.first;assert(renderer.covers_.at(gi).texture.gpu_ptr==item.second[0]);assert(renderer.inside_cache_.entries.at(gi).texture.gpu_ptr==item.second[1]);assert(renderer.disc_cache_.entries.at(gi).texture.gpu_ptr==item.second[2]);}
    check_bounds();puts("PASS: populated Spine -> List preserves common allocations; exact 5/5/5 limit");

    // A case fading out remains alive until the transition no longer asks
    // for it. The temporary cover count obeys the existing pinned-case rule.
    view(OrbitLayout::Spine,10);prepare();warm();
    const auto fading=state.flow;state.layout=OrbitLayout::List;renderer.apply_cache_policy(state,cache);
    for(const auto& p:fading)assert(renderer.has_cover_texture(p.game_index));
    check_bounds();OrbitFlowFix31::reset(state);renderer.apply_cache_policy(state,cache);check_bounds();assert(renderer.gpu_cover_count()==5);
    puts("PASS: fading textures survive layout transition and shrink after its final frame");

    // Real invalidation is still authoritative, including masks for optional
    // art; a cache hit must never resurrect an explicitly reloaded texture.
    const auto old_misses=renderer.gpu_cache_misses()+renderer.inside_cache_misses()+renderer.disc_cache_misses();
    renderer.invalidate_game_art(10,7);cache.invalidate(state.games[10].cover_path);
    assert(!renderer.has_cover_texture(10) && !renderer.has_inside_texture(10) && !renderer.has_disc_art_texture(10));prepare();
    assert(renderer.gpu_cache_misses()+renderer.inside_cache_misses()+renderer.disc_cache_misses()==old_misses+3);
    for(int n=0;n<96;++n){view(n%3==0?OrbitLayout::Classic:n%3==1?OrbitLayout::Spine:OrbitLayout::List,(n*7)%32);prepare();warm();check_bounds();}
    puts("PASS: reload masks, 96 layout/navigation changes, wraparound and unchanged byte/count budgets");
    state.visible.clear();state.flow.clear();renderer.apply_cache_policy(state,cache);
    assert(renderer.gpu_cover_count()==0 && renderer.inside_cache_count()==0 && renderer.disc_cache_count()==0);
    if(Improved)assert(cache.size()==0);
    renderer.shutdown();stage.shutdown();fs::remove_all(scratch);
    puts("PASS: empty library and shutdown release retained resources");
}
