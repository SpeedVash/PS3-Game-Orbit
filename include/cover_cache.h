#pragma once
#include <unordered_map>
#include <string>
#include "cover_image.h"
#include "coverflow_state.h"

class CoverCache {
public:
    explicit CoverCache(size_t max_items = 7) : max_items_(max_items) {}

    const CoverImage* get_or_load(const GameEntry& game);
    void warm_visible_neighborhood(const CoverflowState& state, int radius = 2);
    void clear();
    size_t size() const { return entries_.size(); }

private:
    struct Entry { CoverImage image; unsigned long long stamp = 0; };
    size_t max_items_;
    unsigned long long clock_ = 0;
    std::unordered_map<std::string, Entry> entries_;
    void evict_if_needed();
};
