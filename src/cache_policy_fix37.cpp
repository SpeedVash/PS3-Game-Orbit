#include "rsx_renderer_v10.h"
#ifdef PS3_GAME_ORBIT_FIX37
#include "prefetch_policy_fix36.h"
#include "library_pair_fix28.h"
#include <algorithm>

void RsxRendererV10::apply_cache_policy(const CoverflowState& state,CoverCache& cache){
    resident_limit_=state.layout==OrbitLayout::Classic?7:state.layout==OrbitLayout::List?5:15;
    cache.set_limit(resident_limit_);
    if(state.layout==OrbitLayout::Spine && !state.visible.empty()){
        std::unordered_set<int> pinned;
        for(const auto& p:LibraryPairFix28::poses(state,1))pinned.insert(p.game_index);
        trim_cover_cache(pinned);return;
    }
    std::unordered_set<int> window;
    if(current_game(state))window.insert(state.visible[std::size_t(state.selected)]);
    for(int gi:PrefetchPolicyFix36::candidates(state))window.insert(gi);
    auto visible=window;
    // Preserve a fading case until its final acknowledged frame. It is not
    // allowed to enlarge the idle prefetch window or evict the current game.
    for(const auto& p:LibraryPairFix28::poses(state,1))visible.insert(p.game_index);
    for(auto it=covers_.begin();it!=covers_.end();){
        if(!visible.count(it->first)){release_cover_record(it->second);it=covers_.erase(it);++gpu_cache_evictions_;}
        else ++it;
    }
    for(auto* art:{&inside_cache_,&disc_cache_}){
        for(auto it=art->entries.begin();it!=art->entries.end();){
            if(!window.count(it->first)){
                art->bytes-=std::min(art->bytes,gpu_texture_storage_bytes(it->second.texture));
                stage1_->release_cover(it->second.texture);it=art->entries.erase(it);++art->evictions;
            }else ++it;
        }
        for(auto it=art->failed.begin();it!=art->failed.end();){
            if(!window.count(it->first))it=art->failed.erase(it);else ++it;
        }
    }
    trim_cover_cache(visible);
}
#endif
