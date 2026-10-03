#include "controller.h"
#include <algorithm>

#ifdef __PSL1GHT__
static float normalize_axis(unsigned int v) {
    // DualShock 3 analog axis is 0x00..0xFF, center ~0x80.
    float f = ((float)v - 128.0f) / 127.0f;
    return std::max(-1.0f, std::min(1.0f, f));
}

static void update_edge(ButtonEdge& dst, bool next) {
    const bool old = dst.held;
    dst.held = next;
    dst.pressed = next && !old;
    dst.released = !next && old;
}

#endif

static void clear_edges(InputFrame& f) {
#define CLEAR(x) f.x.pressed = false; f.x.released = false
    CLEAR(left); CLEAR(right); CLEAR(up); CLEAR(down);
    CLEAR(cross); CLEAR(circle); CLEAR(square); CLEAR(triangle);
    CLEAR(l1); CLEAR(r1); CLEAR(l2); CLEAR(r2);
    CLEAR(start); CLEAR(select); CLEAR(l3); CLEAR(r3);
#undef CLEAR
}

#ifdef __PSL1GHT__
#include <io/pad.h>
#include <cstring>

bool Controller::init() {
    std::memset(&current_, 0, sizeof(current_));
    return ioPadInit(1) == 0;
}

void Controller::shutdown() {
    ioPadEnd();
}

InputFrame Controller::poll() {
    clear_edges(current_);

    padInfo2 info{};
    if (ioPadGetInfo2(&info) != 0 || (info.port_status[0] & 1u) == 0) {
        // Force release edges when a pad disconnects.
        update_edge(current_.left, false); update_edge(current_.right, false);
        update_edge(current_.up, false); update_edge(current_.down, false);
        update_edge(current_.cross, false); update_edge(current_.circle, false);
        update_edge(current_.square, false); update_edge(current_.triangle, false);
        update_edge(current_.l1, false); update_edge(current_.r1, false);
        update_edge(current_.l2, false); update_edge(current_.r2, false);
        update_edge(current_.start, false); update_edge(current_.select, false);
        update_edge(current_.l3, false); update_edge(current_.r3, false);
        current_.left_x = current_.left_y = current_.right_x = current_.right_y = 0.0f;
        current_.connected = false;
        return current_;
    }

    current_.connected = true;
    padData data{};
    if (ioPadGetData(0, &data) != 0 || data.len == 0) {
        // PSL1GHT zero-fills padData when nothing changed. Preserve held state/axes.
        return current_;
    }

    update_edge(current_.left, data.BTN_LEFT != 0);
    update_edge(current_.right, data.BTN_RIGHT != 0);
    update_edge(current_.up, data.BTN_UP != 0);
    update_edge(current_.down, data.BTN_DOWN != 0);
    update_edge(current_.cross, data.BTN_CROSS != 0);
    update_edge(current_.circle, data.BTN_CIRCLE != 0);
    update_edge(current_.square, data.BTN_SQUARE != 0);
    update_edge(current_.triangle, data.BTN_TRIANGLE != 0);
    update_edge(current_.l1, data.BTN_L1 != 0);
    update_edge(current_.r1, data.BTN_R1 != 0);
    update_edge(current_.l2, data.BTN_L2 != 0);
    update_edge(current_.r2, data.BTN_R2 != 0);
    update_edge(current_.start, data.BTN_START != 0);
    update_edge(current_.select, data.BTN_SELECT != 0);
    update_edge(current_.l3, data.BTN_L3 != 0);
    update_edge(current_.r3, data.BTN_R3 != 0);

    current_.right_x = normalize_axis(data.ANA_R_H);
    current_.right_y = normalize_axis(data.ANA_R_V);
    current_.left_x  = normalize_axis(data.ANA_L_H);
    current_.left_y  = normalize_axis(data.ANA_L_V);
    return current_;
}

#else
bool Controller::init() { return true; }
void Controller::shutdown() {}

void Controller::inject_host_frame(const InputFrame& frame) {
    injected_ = frame;
    has_injected_ = true;
}

InputFrame Controller::poll() {
    clear_edges(current_);
    if (!has_injected_) return current_;
    current_ = injected_;
    has_injected_ = false;
    return current_;
}
#endif
