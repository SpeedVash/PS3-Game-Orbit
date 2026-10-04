#pragma once
#include <cstdarg>
#include <string>
#include <cstdint>

namespace RuntimeDiag {
void init();
bool available();
void shutdown();
void log(const char* fmt, ...);
std::string log_path();
#ifdef PS3_GAME_ORBIT_FIX35
void flush_if_idle(bool idle,std::uint64_t now_us);
std::size_t pending_bytes();
#endif
}
