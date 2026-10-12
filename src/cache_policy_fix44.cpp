#include "rsx_renderer_v10.h"
#ifdef PS3_GAME_ORBIT_FIX44
#include "prefetch_policy_fix36.h"
#include "library_pair_fix28.h"
#include <algorithm>
#include <limits>

void RsxRendererV10::apply_cache_policy(const CoverflowState& state,CoverCache& cache){
    resident_limit_=
#ifdef PS3_GAME_ORBIT_FIX45
        15;
#else
        state.layout==OrbitLayout::Classic?7:state.layout==OrbitLayout::List?5:15;
#endif
    preferred_covers_.clear();inspection_cache_pins_.clear();
    const auto* selected=current_game(state);
    if(!selected){clear_cover_textures();cache.clear();cache.set_limit(resident_limit_);return;}

    const int selected_index=state.visible[std::size_t(state.selected)];
    preferred_covers_.insert(selected_index);inspection_cache_pins_.insert(selected_index);
    for(int gi:PrefetchPolicyFix36::candidates(state))preferred_covers_.insert(gi);
    std::unordered_set<int> pinned{selected_index};
    for(const auto& p:LibraryPairFix28::poses(state,1)){
        pinned.insert(p.game_index);
        if(p.inspection_phase>0)inspection_cache_pins_.insert(p.game_index);
    }

    // Keep encoded files that will be useful in the destination layout before
    // reducing the CPU cache. No reread or texture upload is needed for hits.
    std::unordered_set<std::string> paths;
    for(int gi:preferred_covers_)if(gi>=0 && gi<int(state.games.size()))paths.insert(state.games[std::size_t(gi)].cover_path);
    for(int gi:pinned)if(gi>=0 && gi<int(state.games.size()))paths.insert(state.games[std::size_t(gi)].cover_path);
    cache.set_limit(resident_limit_,paths);

    // Recent textures survive until capacity or byte pressure needs their
    // space. Fading cases remain pinned until their last acknowledged frame.
    trim_cover_cache(pinned);
    for(auto* art:{&inside_cache_,&disc_cache_}){
        while(art->entries.size()>std::max(resident_limit_,inspection_cache_pins_.size()) || art->bytes>InspectionArtBudgetBytes){
            auto victim=art->entries.end();auto oldest=std::numeric_limits<std::uint64_t>::max();
            for(auto it=art->entries.begin();it!=art->entries.end();++it){
                if(inspection_cache_pins_.count(it->first))continue;
                const bool preferred=preferred_covers_.count(it->first)!=0;
                const bool victim_preferred=victim!=art->entries.end() && preferred_covers_.count(victim->first)!=0;
                if(victim!=art->entries.end() && preferred!=victim_preferred){
                    if(!preferred){victim=it;oldest=it->second.stamp;}
                    continue;
                }
                if(it->second.stamp<oldest){victim=it;oldest=it->second.stamp;}
            }
            if(victim==art->entries.end())break;
            art->bytes-=std::min(art->bytes,gpu_texture_storage_bytes(victim->second.texture));
            stage1_->release_cover(victim->second.texture);art->entries.erase(victim);++art->evictions;
        }
        for(auto it=art->failed.begin();it!=art->failed.end();){
            if(!preferred_covers_.count(it->first) && !inspection_cache_pins_.count(it->first))it=art->failed.erase(it);
            else ++it;
        }
    }
}
#endif
