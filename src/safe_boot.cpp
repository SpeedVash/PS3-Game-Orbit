#include "safe_boot.h"
#include <algorithm>
#include <cmath>

namespace {
float apply_deadzone(float v, float dz) {
    if (std::fabs(v) <= dz) return 0.0f;
    const float sign = v < 0.0f ? -1.0f : 1.0f;
    return sign * (std::fabs(v) - dz) / (1.0f - dz);
}
}

CoverflowState make_safe_boot_state(const std::string& full_cover_path) {
    CoverflowState state;
    GameEntry test;
    test.title = "PS3_SP_LOADER SAFE BOOT";
    test.title_id = "PSSP00001";
    test.path.clear();
    test.cover_path = full_cover_path;
    test.cover_kind = GameCoverKind::FullCover;
    test.source = GameSource::HDD;
    test.format = GameFormat::Folder;
    state.games.push_back(std::move(test));
    state.visible.push_back(0);
    state.selected = 0;
    state.filter = FilterMode::All;
    state.center_yaw_deg = 28.0f;
    state.center_pitch_deg = -5.0f;
    state.inspect_mode = true;
    return state;
}

SafeBootAction update_safe_boot(CoverflowState& state, const InputFrame& in, float dt) {
    if (in.circle.pressed) return SafeBootAction::Exit;
    if (in.cross.pressed) return SafeBootAction::ContinueNormal;

    if (in.r3.pressed) {
        state.center_yaw_deg = 28.0f;
        state.center_pitch_deg = -5.0f;
    }
    if (in.left.pressed) state.center_yaw_deg -= 15.0f;
    if (in.right.pressed) state.center_yaw_deg += 15.0f;

    const float rx = apply_deadzone(in.right_x, 0.18f);
    const float ry = apply_deadzone(in.right_y, 0.18f);
    state.center_yaw_deg += rx * 150.0f * dt;
    state.center_pitch_deg += ry * 70.0f * dt;
    state.center_pitch_deg = std::clamp(state.center_pitch_deg, -28.0f, 22.0f);
    if (state.center_yaw_deg > 3600.0f || state.center_yaw_deg < -3600.0f)
        state.center_yaw_deg = std::fmod(state.center_yaw_deg, 360.0f);
    return SafeBootAction::Stay;
}
