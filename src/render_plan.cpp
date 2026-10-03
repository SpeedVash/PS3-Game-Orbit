#include "render_plan.h"
#include <cmath>

std::vector<CasePose> build_coverflow_render_plan(const CoverflowState& s, int radius) {
    std::vector<CasePose> out;
    if (s.visible.empty()) return out;
    const int n = (int)s.visible.size();
    for (int slot = -radius; slot <= radius; ++slot) {
        int vi = (s.selected + slot) % n;
        if (vi < 0) vi += n;
        CasePose p{};
        p.game_index = s.visible[vi];
        p.relative_slot = slot;
        p.selected = slot == 0;
        if (slot == 0) {
            p.z = 55.0f;
            p.scale = 0.72f; // Leave space around the selected case at 720p.
            p.alpha = 1.0f;
            p.yaw_deg = s.center_yaw_deg;
            p.pitch_deg = s.center_pitch_deg;
        } else {
            const float side = slot < 0 ? -1.0f : 1.0f;
            const float a = (float)std::abs(slot);
            p.x = side * (a == 1.0f ? 185.0f : 315.0f);
            p.z = a == 1.0f ? -70.0f : -160.0f;
            p.yaw_deg = -side * (a == 1.0f ? 36.0f : 48.0f);
            p.pitch_deg = -3.0f;
            p.scale = a == 1.0f ? 0.80f : 0.64f;
            p.alpha = a == 1.0f ? 0.76f : 0.42f;
        }
        out.push_back(p);
    }
    return out;
}
