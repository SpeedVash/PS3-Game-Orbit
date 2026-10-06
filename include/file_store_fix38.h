#pragma once
#ifdef PS3_GAME_ORBIT_FIX38
#include <cstdio>
#include <string>
namespace FileStoreFix38 {
// LV2 rename refuses an existing destination. Keep the preceding complete file
// while installing a flushed temporary file, and restore it on commit failure.
bool commit(const std::string& temporary,const std::string& path);
FILE* open_read(const std::string& path);
}
#endif
