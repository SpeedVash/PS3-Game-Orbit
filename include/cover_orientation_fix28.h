#pragma once
#include "cover_image.h"
#include "image_decode.h"

// EXIF orientations 1..8. Raster row zero always becomes the top of the image.
unsigned read_cover_orientation_fix28(const CoverImage& image);
bool orient_cover_rgba_fix28(DecodedImageRGBA& image,unsigned orientation,std::string& error);
bool uses_full_cover_layout_fix28(const CoverImage& image,unsigned manual_orientation=1);
const char* cover_orientation_name_fix28(unsigned orientation);
// FIX30: honor EXIF, then normalize a portrait full wrap clockwise. Front-only
// artwork remains untouched; there are no user-facing orientation options.
unsigned default_cover_orientation_fix30(const CoverImage& image);
