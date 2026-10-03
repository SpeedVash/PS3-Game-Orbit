#include "ps3_lifecycle.h"
#include "runtime_diag.h"

#ifdef __PSL1GHT__
#include <sysutil/sysutil.h>
Ps3Lifecycle* Ps3Lifecycle::instance_ = nullptr;

void Ps3Lifecycle::callback(std::uint64_t status, std::uint64_t, void*) {
    if (!instance_) return;
    if (status == SYSUTIL_EXIT_GAME) {
        RuntimeDiag::log("SYSUTIL_EXIT_GAME received");
        instance_->exit_requested_ = true;
    }
}
#endif

bool Ps3Lifecycle::init() {
#ifdef __PSL1GHT__
    instance_ = this;
    if (sysUtilRegisterCallback(SYSUTIL_EVENT_SLOT0, callback, nullptr) != 0) {
        RuntimeDiag::log("sysUtilRegisterCallback failed");
        instance_ = nullptr;
        return false;
    }
    registered_ = true;
#endif
    return true;
}

bool Ps3Lifecycle::global_exit_requested() {
#ifdef __PSL1GHT__
    return instance_ && instance_->exit_requested_;
#else
    return false;
#endif
}

void Ps3Lifecycle::pump() {
#ifdef __PSL1GHT__
    if (registered_) sysUtilCheckCallback();
#endif
}

void Ps3Lifecycle::shutdown() {
#ifdef __PSL1GHT__
    if (registered_) sysUtilUnregisterCallback(SYSUTIL_EVENT_SLOT0);
    registered_ = false;
    instance_ = nullptr;
#endif
}
