#pragma once
#include <cstdarg>
#include <string>

namespace RuntimeDiag {
void init();
bool available();
void shutdown();
void log(const char* fmt, ...);
std::string log_path();
}
