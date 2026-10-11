#include "cover_cache.h"
#include <limits>

const CoverImage* CoverCache::get_or_load(const GameEntry& game) {
    if (game.cover_path.empty()) return nullptr;
    const auto key=game.cover_path+"\n"+std::to_string(static_cast<int>(game.cover_kind));
    auto it = entries_.find(key);
    if (it != entries_.end()) {
        it->second.stamp = ++clock_;
        return &it->second.image;
    }

    CoverImage img = load_cover_file(game.cover_path, game.cover_kind);
    if (!img.valid()) return nullptr;
    Entry e{std::move(img), ++clock_};
    auto inserted = entries_.emplace(key, std::move(e));
    encoded_bytes_+=inserted.first->second.image.encoded.size();
    evict_if_needed(&key);
    return &inserted.first->second.image;
}

void CoverCache::warm_visible_neighborhood(const CoverflowState& s, int radius) {
    if (s.visible.empty()) return;
    const int n = (int)s.visible.size();
    for (int d = -radius; d <= radius; ++d) {
        int vi = (s.selected + d) % n;
        if (vi < 0) vi += n;
        const int gi = s.visible[vi];
        if (gi >= 0 && gi < (int)s.games.size()) get_or_load(s.games[gi]);
    }
}

void CoverCache::clear() { entries_.clear(); clock_ = 0;encoded_bytes_=0;
#ifdef PS3_GAME_ORBIT_FIX44
    preferred_paths_.clear();
#endif
}

void CoverCache::evict_if_needed(const std::string* loading_key) {
    while (entries_.size() > max_items_ || (max_bytes_ && encoded_bytes_>max_bytes_ && entries_.size()>1)) {
        auto victim = entries_.end();
        unsigned long long best = std::numeric_limits<unsigned long long>::max();
        for (auto it = entries_.begin(); it != entries_.end(); ++it) {
            // The caller must receive a live image even when preference-based
            // eviction would otherwise choose the record it just inserted.
            if(loading_key && it->first==*loading_key)continue;
#ifdef PS3_GAME_ORBIT_FIX44
            const bool preferred=preferred_paths_.count(it->second.image.path)!=0;
            const bool victim_preferred=victim!=entries_.end() && preferred_paths_.count(victim->second.image.path)!=0;
            if(victim!=entries_.end() && preferred!=victim_preferred){
                if(!preferred){best=it->second.stamp;victim=it;}
                continue;
            }
#endif
            if (it->second.stamp < best) { best = it->second.stamp; victim = it; }
        }
        if (victim == entries_.end()) break;
        encoded_bytes_-=victim->second.image.encoded.size();entries_.erase(victim);
    }
}

#ifdef PS3_GAME_ORBIT_FIX35
void CoverCache::invalidate(const std::string& path){
    for(auto it=entries_.begin();it!=entries_.end();){if(it->second.image.path==path){encoded_bytes_-=it->second.image.encoded.size();it=entries_.erase(it);}else ++it;}
}
#endif
