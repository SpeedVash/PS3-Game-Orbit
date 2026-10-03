#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "game_entry.h"
#include "full_cover_layout.h"

enum class CoverFileFormat { Unknown, PNG, JPEG };

struct CoverImage {
    std::string path;
    GameCoverKind kind = GameCoverKind::None;
    CoverFileFormat format = CoverFileFormat::Unknown;
    int width = 0;
    int height = 0;
    std::vector<std::uint8_t> encoded;

    bool valid() const { return !encoded.empty() && width > 0 && height > 0; }
    float aspect() const { return height > 0 ? (float)width / (float)height : 0.0f; }
    bool looks_like_canonical_full_cover(float tolerance = 0.08f) const;
};

CoverImage load_cover_file(const std::string& path, GameCoverKind kind);
