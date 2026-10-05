#pragma once
#ifdef PS3_GAME_ORBIT_FIX36
#include "image_decode.h"
#include "v14_case_mesh.h"
namespace OrbitBackgroundFix36 {
constexpr unsigned Ribbons=3,Segments=96,VerticesPerRibbon=(Segments+1)*2,IndicesPerRibbon=Segments*6;
struct Mesh{std::vector<V14Vertex> vertices;std::vector<std::uint16_t> indices;};
Mesh build(float phase=6);
void update(Mesh& mesh,float phase);
DecodedImageRGBA gradient();
DecodedImageRGBA ribbon_texture();
}
#endif
