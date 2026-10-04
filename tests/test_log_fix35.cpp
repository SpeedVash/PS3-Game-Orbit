#include "runtime_diag.h"
#include "project_identity.h"
#include <cassert>
#include <filesystem>
#include <cstdio>
int main(){
 std::remove(ProjectIdentity::LogPath);RuntimeDiag::init();assert(RuntimeDiag::available());
 const auto path=RuntimeDiag::log_path();const auto before=std::filesystem::file_size(path);
 RuntimeDiag::log("First batch %d",35);assert(RuntimeDiag::pending_bytes()>0 && std::filesystem::file_size(path)==before);
 RuntimeDiag::flush_if_idle(false,4000000);assert(RuntimeDiag::pending_bytes()>0 && std::filesystem::file_size(path)==before);
 RuntimeDiag::flush_if_idle(true,4000000);assert(RuntimeDiag::pending_bytes()==0 && std::filesystem::file_size(path)>before);
 const auto after=std::filesystem::file_size(path);RuntimeDiag::log("Deferred batch");RuntimeDiag::flush_if_idle(true,4500000);
 assert(RuntimeDiag::pending_bytes()>0 && std::filesystem::file_size(path)==after);
 for(int i=0;i<5000;++i)RuntimeDiag::log("Buffer bound %d",i);
 assert(RuntimeDiag::pending_bytes()<=65536);RuntimeDiag::shutdown();assert(!RuntimeDiag::available() && std::filesystem::file_size(path)>after);
 std::remove(path.c_str());std::puts("PASS: actual log defers writes during navigation, batches idle flushes, caps memory at 64 KiB and flushes at shutdown");
}
