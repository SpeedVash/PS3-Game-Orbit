#pragma once
#include <vector>
#include "coverflow_state.h"
#include "case_pose.h"

std::vector<CasePose> build_coverflow_render_plan(const CoverflowState& state, int radius = 2);
