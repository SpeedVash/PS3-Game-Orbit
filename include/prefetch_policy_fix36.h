#pragma once
#ifdef PS3_GAME_ORBIT_FIX36
#include "coverflow_state.h"
#include <cstdint>
namespace PrefetchPolicyFix36 {
std::vector<int> candidates(const CoverflowState& state);
constexpr std::uint64_t InspectionIdleUs=900000,SliceIntervalUs=200000;
}
#endif
