#pragma once
#include <unordered_map>
#include <string>
#include "cover_image.h"
#include "coverflow_state.h"

class CoverCache {
public:
    explicit CoverCache(size_t max_items = 7,size_t max_bytes = 0) : max_items_(max_items ? max_items : 1),max_bytes_(max_bytes) {}

    const CoverImage* get_or_load(const GameEntry& game);
    void warm_visible_neighborhood(const CoverflowState& state, int radius = 2);
    void clear();
#ifdef PS3_GAME_ORBIT_FIX37
    void set_limit(size_t items){max_items_=items?items:1;evict_if_needed();}
#endif
#ifdef PS3_GAME_ORBIT_FIX35
    void invalidate(const std::string& path);
#endif
    size_t size() const { return entries_.size(); }
    size_t encoded_bytes() const { return encoded_bytes_; }

private:
    struct Entry { CoverImage image; unsigned long long stamp = 0; };
    size_t max_items_;
    size_t max_bytes_=0,encoded_bytes_=0;
    unsigned long long clock_ = 0;
    std::unordered_map<std::string, Entry> entries_;
    void evict_if_needed();
};
