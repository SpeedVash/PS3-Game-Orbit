#pragma once
#include "coverflow_state.h"
#include "input_state.h"

struct AppCommands {
    bool mount_selected = false;
    bool rescan_library = false;
    bool request_exit = false;
};

class AppController {
public:
    AppCommands update(CoverflowState& state, const InputFrame& in, float dt_seconds);

private:
    float left_repeat_ = 0.0f;
    float right_repeat_ = 0.0f;
    bool left_axis_latched_ = false;
    bool right_axis_latched_ = false;

    static FilterMode previous_filter(FilterMode f);
    static FilterMode next_filter(FilterMode f);
    static GameEntry* selected_game(CoverflowState& s);
};
