#pragma once
#include <array>
#include <vector>
#include <string>
#include "image_decode.h"
#include "v14_case_mesh.h"

namespace OrbitUiFix30 {
inline constexpr int Width=1280, BackgroundHeight=360,
#ifdef PS3_GAME_ORBIT_FIX32
HudHeight=512;
#else
HudHeight=384;
#endif
// Atlas: 64 px header, 112 px footer, 208 px optional help. Three quads,
// one indexed HUD draw; the central area remains transparent by default.
DecodedImageRGBA background();
DecodedImageRGBA hud(const std::array<std::string,6>& lines
#ifdef PS3_GAME_ORBIT_FIX32
    ,const std::vector<std::string>& rows={},int active_row=-1
#endif
);
DecodedImageRGBA splash(const DecodedImageRGBA* artwork=nullptr);
std::array<V14Vertex,12> hud_vertices();
std::array<V14Vertex,4> background_vertices();
inline constexpr std::array<std::uint16_t,18> HudIndices{{0,1,2,0,2,3,4,5,6,4,6,7,8,9,10,8,10,11}};
inline constexpr std::array<std::uint16_t,6> BackgroundIndices{{0,1,2,0,2,3}};
}
