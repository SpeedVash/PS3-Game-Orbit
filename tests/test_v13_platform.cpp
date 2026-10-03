#include <cassert>
#include <cstdio>
#include "ps3_lifecycle.h"
#include "runtime_diag.h"
#include "project_identity.h"
#include <string>
int main(){
    RuntimeDiag::init();
    RuntimeDiag::log("V13 platform host test");
    Ps3Lifecycle life;
    assert(life.init());
    life.pump();
    assert(!life.exit_requested());
    life.shutdown();
    assert(std::string(ProjectIdentity::Name) == "PS3_SP_LOADER");
    assert(std::string(ProjectIdentity::Version) == "V13");
    assert(std::string(ProjectIdentity::AppId) == "PSSP00001");
    assert(RuntimeDiag::log_path().find("PS3_SP_LOADER_V13.log") != std::string::npos);
    RuntimeDiag::shutdown();
    std::puts("V13 lifecycle + diagnostics tests: OK");
    return 0;
}
