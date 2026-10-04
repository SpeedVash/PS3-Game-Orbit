#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "cover_image.h"

struct DecodedImageRGBA {
    int width = 0;
    int height = 0;
    int pitch = 0; // bytes per row in rgba
    std::vector<std::uint8_t> rgba;

    bool valid() const {
        return width > 0 && height > 0 && pitch >= width * 4 &&
               rgba.size() >= static_cast<std::size_t>(pitch) * static_cast<std::size_t>(height);
    }
};

// Decodes an already-loaded CoverImage to 8-bit RGBA.
// Host builds use libpng/libjpeg. PS3 builds use PSL1GHT pngdec/jpgdec helpers.
bool decode_cover_rgba(const CoverImage& src, DecodedImageRGBA& out, std::string& error);

#ifdef PS3_GAME_ORBIT_FIX35
struct DecodedImageARGB {
    int width=0,height=0,pitch=0;
    std::vector<std::uint8_t> argb;
    bool valid() const {return width>0 && height>0 && pitch>=width*4 && argb.size()>=std::size_t(pitch)*height;}
};
bool init_image_decoders_fix35();
void shutdown_image_decoders_fix35();
bool decode_cover_argb_fix35(const CoverImage& src,DecodedImageARGB& out,std::string& error);
#endif
