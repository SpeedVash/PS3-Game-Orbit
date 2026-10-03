#include <cassert>
#include <cmath>
#include <iostream>
#include "safe_boot.h"
#include "render_plan.h"

int main() {
    CoverflowState s = make_safe_boot_state("/dev_hdd0/game/PSSP00001/USRDIR/FULL_COVER_TEST_275x147.png");
    assert(s.games.size() == 1);
    assert(s.visible.size() == 1 && s.visible[0] == 0);
    assert(s.games[0].cover_kind == GameCoverKind::FullCover);
    assert(s.games[0].cover_path.find("FULL_COVER_TEST_275x147.png") != std::string::npos);
    assert(s.inspect_mode);
    const auto poses = build_coverflow_render_plan(s, 0);
    assert(poses.size() == 1 && poses[0].selected);

    InputFrame f{}; f.connected = true; f.right_x = 1.0f;
    const float y0 = s.center_yaw_deg;
    assert(update_safe_boot(s, f, 0.1f) == SafeBootAction::Stay);
    assert(s.center_yaw_deg > y0);

    f = {}; f.connected = true; f.r3.pressed = true;
    update_safe_boot(s, f, 0.016f);
    assert(std::fabs(s.center_yaw_deg - 28.0f) < 0.01f);
    assert(std::fabs(s.center_pitch_deg + 5.0f) < 0.01f);

    f = {}; f.connected = true; f.cross.pressed = true;
    assert(update_safe_boot(s, f, 0.016f) == SafeBootAction::ContinueNormal);
    f = {}; f.connected = true; f.circle.pressed = true;
    assert(update_safe_boot(s, f, 0.016f) == SafeBootAction::Exit);

    std::cout << "V13 safe-boot state/input tests: OK\n";
    return 0;
}
