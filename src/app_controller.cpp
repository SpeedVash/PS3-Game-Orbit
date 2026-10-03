#include "app_controller.h"
#include <algorithm>
#include <cmath>

static float apply_deadzone(float v, float dz) {
    if (std::fabs(v) <= dz) return 0.0f;
    const float sign = v < 0.0f ? -1.0f : 1.0f;
    return sign * (std::fabs(v) - dz) / (1.0f - dz);
}

FilterMode AppController::previous_filter(FilterMode f) {
    switch (f) {
        case FilterMode::All: return FilterMode::Favorites;
        case FilterMode::HDD: return FilterMode::All;
        case FilterMode::USB: return FilterMode::HDD;
        case FilterMode::Favorites: return FilterMode::USB;
    }
    return FilterMode::All;
}

FilterMode AppController::next_filter(FilterMode f) {
    switch (f) {
        case FilterMode::All: return FilterMode::HDD;
        case FilterMode::HDD: return FilterMode::USB;
        case FilterMode::USB: return FilterMode::Favorites;
        case FilterMode::Favorites: return FilterMode::All;
    }
    return FilterMode::All;
}

GameEntry* AppController::selected_game(CoverflowState& s) {
    if (s.visible.empty() || s.selected < 0 || s.selected >= (int)s.visible.size()) return nullptr;
    const int gi = s.visible[s.selected];
    if (gi < 0 || gi >= (int)s.games.size()) return nullptr;
    return &s.games[gi];
}

AppCommands AppController::update(CoverflowState& s, const InputFrame& in, float dt) {
    AppCommands cmd{};
    if (!in.connected) return cmd;

    if (in.l1.pressed) {
        s.filter = previous_filter(s.filter);
        rebuild_visible(s);
    }
    if (in.r1.pressed) {
        s.filter = next_filter(s.filter);
        rebuild_visible(s);
    }

    if (in.square.pressed) {
        s.inspect_mode = !s.inspect_mode;
        if (!s.inspect_mode) {
            s.center_yaw_deg = 0.0f;
            s.center_pitch_deg = -5.0f;
        }
    }

    if (in.r3.pressed) {
        s.center_yaw_deg = 0.0f;
        s.center_pitch_deg = -5.0f;
    }

    if (in.triangle.pressed) {
        if (GameEntry* g = selected_game(s)) {
            g->favorite = !g->favorite;
            if (s.filter == FilterMode::Favorites && !g->favorite) rebuild_visible(s);
        }
    }

    if (in.cross.pressed && !s.visible.empty()) cmd.mount_selected = true;
    if (in.start.pressed) cmd.rescan_library = true;

    if (in.circle.pressed) {
        if (s.inspect_mode) {
            s.inspect_mode = false;
            s.center_yaw_deg = 0.0f;
            s.center_pitch_deg = -5.0f;
        } else {
            cmd.request_exit = true;
        }
    }

    if (s.inspect_mode) {
        const float rx = apply_deadzone(in.right_x, 0.18f);
        const float ry = apply_deadzone(in.right_y, 0.18f);
        s.center_yaw_deg += rx * 150.0f * dt;
        s.center_pitch_deg += ry * 70.0f * dt;
        s.center_pitch_deg = std::clamp(s.center_pitch_deg, -28.0f, 22.0f);
        if (s.center_yaw_deg > 3600.0f || s.center_yaw_deg < -3600.0f)
            s.center_yaw_deg = std::fmod(s.center_yaw_deg, 360.0f);
        return cmd;
    }

    // Digital navigation has immediate edge response.
    if (in.left.pressed) move_selection(s, -1);
    if (in.right.pressed) move_selection(s, +1);

    // Left stick adds Aurora-like held navigation with a short repeat delay.
    const float lx = apply_deadzone(in.left_x, 0.42f);
    if (lx < -0.01f) {
        right_axis_latched_ = false; right_repeat_ = 0.0f;
        left_repeat_ -= dt;
        if (!left_axis_latched_ || left_repeat_ <= 0.0f) {
            move_selection(s, -1);
            left_axis_latched_ = true;
            left_repeat_ = 0.18f;
        }
    } else if (lx > 0.01f) {
        left_axis_latched_ = false; left_repeat_ = 0.0f;
        right_repeat_ -= dt;
        if (!right_axis_latched_ || right_repeat_ <= 0.0f) {
            move_selection(s, +1);
            right_axis_latched_ = true;
            right_repeat_ = 0.18f;
        }
    } else {
        left_axis_latched_ = right_axis_latched_ = false;
        left_repeat_ = right_repeat_ = 0.0f;
    }

    return cmd;
}
