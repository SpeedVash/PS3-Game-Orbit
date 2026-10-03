#pragma once
#include <array>
#include "library_browser_fix28.h"
#include "image_decode.h"
#include "v14_case_mesh.h"

// Six bounded text rows in one 1024x192 texture. One indexed draw shows both
// panels; the middle of the screen continues to contain only the validated case.
DecodedImageRGBA rasterize_library_hud_fix28(const LibraryHudLinesFix28& lines);
#ifdef PS3_GAME_ORBIT_FIX30
#include "orbit_ui_fix30.h"
std::array<V14Vertex,12> library_hud_vertices_fix28();
inline constexpr auto LibraryHudIndicesFix28=OrbitUiFix30::HudIndices;
#else
std::array<V14Vertex,8> library_hud_vertices_fix28();
inline constexpr std::array<std::uint16_t,12> LibraryHudIndicesFix28{{0,1,2,0,2,3,4,5,6,4,6,7}};
#endif
