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
    evict_if_needed();
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

void CoverCache::clear() { entries_.clear(); clock_ = 0; }

void CoverCache::evict_if_needed() {
    while (entries_.size() > max_items_) {
        auto victim = entries_.end();
        unsigned long long best = std::numeric_limits<unsigned long long>::max();
        for (auto it = entries_.begin(); it != entries_.end(); ++it) {
            if (it->second.stamp < best) { best = it->second.stamp; victim = it; }
        }
        if (victim == entries_.end()) break;
        entries_.erase(victim);
    }
}
