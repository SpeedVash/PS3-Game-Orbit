#pragma once
#include "render_plan.h"
namespace LibraryPairFix28 {
inline constexpr std::size_t MaxCases=2;
inline constexpr std::size_t FrameCommandBudgetBytes=12288;
// Selected case plus next distinct visible game, using the original side pose.
std::vector<CasePose> poses(const CoverflowState& state,int radius);
}
