#pragma once
#ifdef PS3_GAME_ORBIT_FIX37
#include "image_decode.h"
namespace ArtworkResizeFix37 {
inline constexpr int CoverWidth=1000,CoverHeight=550,DiscEdge=500;
bool resize(const DecodedImageRGBA& source,int width,int height,DecodedImageRGBA& target);
bool encode(const DecodedImageRGBA& image,bool disc,std::vector<std::uint8_t>& bytes);
}
#endif
