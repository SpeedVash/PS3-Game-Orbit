#pragma once
#include "render_plan.h"
#ifdef PS3_GAME_ORBIT_FIX31
#include "orbit_flow_fix31.h"
#endif
namespace LibraryPairFix28 {
#ifdef PS3_GAME_ORBIT_FIX31
inline constexpr std::size_t MaxCases=OrbitFlowFix31::MaxCases;
inline constexpr std::size_t FrameCommandBudgetBytes=OrbitFlowFix31::FrameCommandBudgetBytes;
#else
inline constexpr std::size_t MaxCases=2;
inline constexpr std::size_t FrameCommandBudgetBytes=12288;
#endif
// Selected case plus next distinct visible game, using the original side pose.
std::vector<CasePose> poses(const CoverflowState& state,int radius);
}
