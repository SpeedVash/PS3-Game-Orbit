#pragma once
#include "input_state.h"

class Controller {
public:
    bool init();
    void shutdown();
    InputFrame poll();

#ifndef __PSL1GHT__
    // Host-test hook. Ignored on PS3 builds.
    void inject_host_frame(const InputFrame& frame);
#endif

private:
    InputFrame current_{};
#ifndef __PSL1GHT__
    InputFrame injected_{};
    bool has_injected_ = false;
#endif
};
