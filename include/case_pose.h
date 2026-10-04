#pragma once

struct CasePose {
    int game_index = -1;
    int relative_slot = 0;
    float x = 0, y = 0, z = 0;
    float yaw_deg = 0, pitch_deg = 0;
    float scale = 1, alpha = 1;
    bool selected = false;
    // Animation visibility is independent of the approved plastic opacity.
    float visibility = 1;
    float inspection_phase = 0;
};
