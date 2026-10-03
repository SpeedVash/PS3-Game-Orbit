#pragma once
#include <cstdint>

class Ps3Lifecycle {
public:
    bool init();
    void pump();
    void shutdown();
    bool exit_requested() const { return exit_requested_; }
    static bool global_exit_requested();
private:
    bool registered_ = false;
    bool exit_requested_ = false;
#ifdef __PSL1GHT__
    static Ps3Lifecycle* instance_;
    static void callback(std::uint64_t status, std::uint64_t param, void* userdata);
#endif
};
