#pragma once
#include <vector>
#include "coverflow_state.h"

struct CasePose {
    int game_index = -1;
    int relative_slot = 0;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float yaw_deg = 0.0f;
    float pitch_deg = 0.0f;
    float scale = 1.0f;
    float alpha = 1.0f;
    bool selected = false;
};

std::vector<CasePose> build_coverflow_render_plan(const CoverflowState& state, int radius = 2);
