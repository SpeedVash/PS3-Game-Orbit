#pragma once
#include "render_plan.h"
#include <cstddef>

namespace OrbitFlowFix31 {
inline constexpr std::size_t MaxCases = 14;
inline constexpr int SpineRadius = 5;
inline constexpr std::size_t FrameCommandBudgetBytes = 57344;
inline constexpr std::size_t InitialFrameGuardBytes = 61440;
inline constexpr float MinimumScale = 0.55f;
// Aura Normal: camera -5, center -1.85, neighbors -.63/.62,
// spacing .1, yaw -1.7/-1.5 radians. Convert to our +Z camera/JFX units.
std::vector<CasePose> targets(const CoverflowState& state, int radius = 1);
std::vector<CasePose> poses(const CoverflowState& state, int radius = 1);
void reset(CoverflowState& state);
bool can_admit(const CoverflowState& state);
// Fading exits keep their own lifetime when navigation changes again.
void advance(CoverflowState& state, float dt);
const char* layout_name(OrbitLayout layout);
}
