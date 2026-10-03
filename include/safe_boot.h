#pragma once
#include <string>
#include "coverflow_state.h"
#include "input_state.h"

enum class SafeBootAction {
    Stay,
    ContinueNormal,
    Exit
};

// Creates a one-item library that exercises the approved V14 geometry and a
// canonical 275:147 full cover without touching the user's real game library.
CoverflowState make_safe_boot_state(const std::string& full_cover_path);

// Input-only safe-mode controller. RIGHT STICK rotates, LEFT/RIGHT steps yaw,
// R3 resets, CROSS continues to the normal library, CIRCLE exits.
SafeBootAction update_safe_boot(CoverflowState& state, const InputFrame& in, float dt_seconds);
