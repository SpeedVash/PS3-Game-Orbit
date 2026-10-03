#include "runtime_diag.h"
#include "project_identity.h"
#include <cstdio>
#include <string>
#include <ctime>

namespace {
FILE* g_file = nullptr;
const char* g_path = ProjectIdentity::LogPath;
}

namespace RuntimeDiag {
void init() {
    if (g_file) return;
    g_path = ProjectIdentity::LogPath;
    g_file = std::fopen(g_path, "a");
#if defined(__PSL1GHT__) && (defined(PS3_SP_LOADER_FIX20) || defined(PS3_SP_LOADER_FIX28))
    if (!g_file) {
        g_path = ProjectIdentity::FallbackLogPath;
        g_file = std::fopen(g_path, "a");
    }
#endif
    if (!g_file) return;
    std::time_t now = std::time(nullptr);
    std::fprintf(g_file, "\n=== %s start %lld ===\n", ProjectIdentity::DisplayTitle, (long long)now);
    std::fflush(g_file);
}

bool available() { return g_file != nullptr; }

void shutdown() {
    if (!g_file) return;
    std::fprintf(g_file, "=== %s shutdown ===\n", ProjectIdentity::DisplayTitle);
    std::fflush(g_file);
    std::fclose(g_file);
    g_file = nullptr;
}

void log(const char* fmt, ...) {
    va_list ap;
#if !defined(__PSL1GHT__) || (!defined(PS3_SP_LOADER_FIX20) && !defined(PS3_SP_LOADER_FIX28))
    va_start(ap, fmt);
    std::vfprintf(stdout, fmt, ap);
    std::fprintf(stdout, "\n");
    va_end(ap);
#endif

    if (!g_file) return;
    va_start(ap, fmt);
    std::vfprintf(g_file, fmt, ap);
    std::fprintf(g_file, "\n");
    va_end(ap);
    std::fflush(g_file);
}

std::string log_path() { return g_path; }
}
