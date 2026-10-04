#include "rsx_renderer_v10.h"
#ifdef PS3_GAME_ORBIT_FIX32
#include "library_pair_fix28.h"
#include "runtime_diag.h"
#include <limits>
#include <algorithm>
#ifdef PS3_GAME_ORBIT_FIX34
#include "orbit_flow_fix31.h"
#endif

#ifdef PS3_GAME_ORBIT_FIX33
void RsxRendererV10::trim_cover_cache(const std::unordered_set<int>& pinned) {
    const auto limit=std::max(MaxResidentCovers,pinned.size());
    while(covers_.size()>limit || gpu_cache_bytes_>GpuCoverBudgetBytes) {
        auto victim=covers_.end();auto oldest=std::numeric_limits<std::uint64_t>::max();
        for(auto it=covers_.begin();it!=covers_.end();++it)
            if(!pinned.count(it->first) && it->second.stamp<oldest) {victim=it;oldest=it->second.stamp;}
        if(victim==covers_.end()) break;
        release_cover_record(victim->second);covers_.erase(victim);++gpu_cache_evictions_;
    }
}
#endif

bool RsxRendererV10::reserve_cover_cache(const std::unordered_set<int>& pinned,std::size_t bytes) {
    const auto limit=
#ifdef PS3_GAME_ORBIT_FIX33
        std::max(MaxResidentCovers,pinned.size());
#else
        MaxResidentCovers;
#endif
    while(gpu_cache_bytes_+bytes>GpuCoverBudgetBytes || covers_.size()>=limit) {
        auto victim=covers_.end();auto oldest=std::numeric_limits<std::uint64_t>::max();
        for(auto it=covers_.begin();it!=covers_.end();++it) {
            if(!pinned.count(it->first) && it->second.stamp<oldest) {victim=it;oldest=it->second.stamp;}
        }
        if(victim==covers_.end()) return false;
        release_cover_record(victim->second);covers_.erase(victim);++gpu_cache_evictions_;
    }
    return true;
}
bool RsxRendererV10::load_gpu_cover(int gi,const GameEntry& game,CoverCache& cache,const std::unordered_set<int>& pinned) {
    auto it=covers_.find(gi);
    if(it!=covers_.end() && it->second.path==game.cover_path && it->second.kind==game.cover_kind &&
       it->second.orientation==game.cover_orientation && it->second.texture.uploaded) {
        it->second.stamp=++gpu_cache_clock_;++gpu_cache_hits_;return true;
    }
    if(it!=covers_.end()) {release_cover_record(it->second);covers_.erase(it);}
    if(game.cover_path.empty() || failed_gpu_covers_.count(gi)) return false;
    // Reserve the bounded worst-case texture BEFORE allocating; visible/fading
    // cases are pinned. Main calls this only between acknowledged RSX frames.
    constexpr std::size_t WorstTextureBytes=1024u*1024u*4u;
    if(!reserve_cover_cache(pinned,WorstTextureBytes)) return false;
    const auto* image=cache.get_or_load(game);
    ++gpu_cache_misses_;
    GpuCoverRecord fresh;fresh.path=game.cover_path;fresh.kind=game.cover_kind;
    fresh.orientation=game.cover_orientation;
    if(!image || !image->valid() || !stage1_->prepare_cover(*image,fresh.texture,game.cover_orientation)) {
        failed_gpu_covers_.insert(gi);
        RuntimeDiag::log("COVER FALLBACK: game=%d path=%s error=%s",gi,game.cover_path.c_str(),stage1_->last_error().c_str());
        return false;
    }
    fresh.stamp=++gpu_cache_clock_;
    gpu_cache_bytes_+=std::size_t(fresh.texture.pitch)*std::size_t(fresh.texture.height);
    covers_.emplace(gi,std::move(fresh));return true;
}
bool RsxRendererV10::prefetch_nearby_cover(const CoverflowState& state,CoverCache& cache) {
    if(!ready_ || !stage1_ || state.visible.empty() || state.inspection_target || state.inspection_phase>0) return false;
    std::unordered_set<int> pinned;
    for(const auto& p:LibraryPairFix28::poses(state,1)) pinned.insert(p.game_index);
#ifdef PS3_GAME_ORBIT_FIX34
    std::unordered_set<int> targets;
    for(const auto& p:OrbitFlowFix31::targets(state,1)) targets.insert(p.game_index);
    if(pinned!=targets) return false;
#endif
#ifdef PS3_GAME_ORBIT_FIX33
    if(pinned.size()>=MaxResidentCovers) return false;
#endif
    const int n=int(state.visible.size());
    for(int distance=1;distance<=
#ifdef PS3_GAME_ORBIT_FIX34
        7
#elif defined(PS3_GAME_ORBIT_FIX33)
        4
#else
        7
#endif
        ;++distance) for(int sign:{state.navigation_direction,-state.navigation_direction}) {
        int vi=(state.selected+distance*sign)%n;if(vi<0) vi+=n;
        const int gi=state.visible[std::size_t(vi)];
        const auto& g=state.games[std::size_t(gi)];
        if(covers_.count(gi) || g.cover_path.empty() || failed_gpu_covers_.count(gi)) continue;
        return load_gpu_cover(gi,g,cache,pinned);
    }
    return false;
}
#endif
