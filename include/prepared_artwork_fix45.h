#pragma once
#ifdef PS3_GAME_ORBIT_FIX45
#include "cover_image.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

enum class ArtworkRoleFix45 { Cover, Inside, Disc };
struct ArtworkRequestFix45 {
    int game_index=-1;
    std::string game_path,path;
    ArtworkRoleFix45 role=ArtworkRoleFix45::Cover;
    GameCoverKind kind=GameCoverKind::None;
    unsigned orientation=1;
    std::uint64_t generation=0;
    bool operator==(const ArtworkRequestFix45& other) const;
};
struct PreparedArtworkFix45 {
    ArtworkRequestFix45 request;
    int width=0,height=0,pitch=0;
    bool full_cover=false;
    std::vector<std::uint8_t> argb;
    std::string error;
    bool valid() const;
};
// CPU-only: reads, decodes, orients, bounds and formats the entire image.
// No renderer, RSX allocation or browser state is accessed by the worker.
PreparedArtworkFix45 prepare_artwork_fix45(const ArtworkRequestFix45& request);
#endif
