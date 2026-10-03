#pragma once
#include <array>
#include "v14_case_mesh.h"

namespace JfxCaseFix29 {
inline constexpr std::size_t OriginalTriangles=5597;
inline constexpr std::size_t DrawsPerCase=6; // 3 paper ranges, logo, 2 plastic passes.
inline constexpr float BackEnd=(0.4648f-0.0469f)/(0.951f-0.0469f);
inline constexpr float FrontBegin=(0.5332f-0.0469f)/(0.951f-0.0469f);
inline constexpr std::uint32_t Background=0xff252525;
V14CaseMesh build();
// A point on a source triangle, interpolated in normalized full-cover UVs.
bool surface_point(const V14MeshPart& part,float local_u,float v,std::array<float,3>& result);
}
