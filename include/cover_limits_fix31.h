#pragma once
#include "image_decode.h"
namespace CoverLimitsFix31 {
inline constexpr int TextureEdge=1024;
inline constexpr std::size_t EncodedCacheBytes=32u<<20;
inline constexpr std::size_t MaximumDecodePixels=16u<<20;
// Preserve the complete atlas and aspect; interpolate pixels only.
bool fit_texture(DecodedImageRGBA& image);
}
