#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include "rsx_renderer_v10.h"

namespace CaseRenderFix25 {
inline constexpr std::size_t FrameCommandBudgetBytes = 8192;
inline constexpr std::size_t InitialFrameGuardBytes = 16384;
inline constexpr std::uint32_t Background = 0xff16202b;

// Correct only the GPU index copy; positions, dimensions, normals and UVs stay frozen.
std::vector<std::uint16_t> outward_indices(const V14MeshPart& part);
// Opaque cover first, then translucent plastic from far to near.
std::vector<std::size_t> submission_order(const V10FramePlan& plan,const V14CaseMesh& mesh);
}
