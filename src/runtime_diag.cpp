#include "runtime_diag.h"
#include "project_identity.h"
#include <cstdio>
#include <string>
#include <ctime>
#include <array>
#include <algorithm>
#ifdef PS3_GAME_ORBIT_FIX45
#include <atomic>
#endif

namespace {
FILE* g_file = nullptr;
const char* g_path = ProjectIdentity::LogPath;
#ifdef PS3_GAME_ORBIT_FIX35
std::string pending;std::uint64_t last_flush=0;
#ifdef PS3_GAME_ORBIT_FIX45
std::atomic<std::uint64_t> dropped{0};
std::atomic_flag pending_lock=ATOMIC_FLAG_INIT;
#else
std::uint64_t dropped=0;
#endif
constexpr std::size_t MaxPending=64u<<10;
void flush_pending(){
    if(!g_file)return;
#ifdef PS3_GAME_ORBIT_FIX45
    std::string output;output.reserve(MaxPending);
    if(pending_lock.test_and_set(std::memory_order_acquire))return;
    output.swap(pending);const auto suppressed=dropped.exchange(0);
    pending_lock.clear(std::memory_order_release);
    if(!output.empty())std::fwrite(output.data(),1,output.size(),g_file);
    if(suppressed)std::fprintf(g_file,"LOG: suppressed=%llu buffer_limit=65536\n",static_cast<unsigned long long>(suppressed));
#else
    if(!pending.empty()){std::fwrite(pending.data(),1,pending.size(),g_file);pending.clear();}
    if(dropped){std::fprintf(g_file,"LOG: suppressed=%llu buffer_limit=65536\n",static_cast<unsigned long long>(dropped));dropped=0;}
#endif
    std::fflush(g_file);
}
#endif
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
#ifdef PS3_GAME_ORBIT_FIX35
    pending.clear();last_flush=0;dropped=0;
#ifdef PS3_GAME_ORBIT_FIX45
    pending.reserve(MaxPending);
#endif
#endif
    std::time_t now = std::time(nullptr);
    std::fprintf(g_file, "\n=== %s start %lld ===\n", ProjectIdentity::DisplayTitle, (long long)now);
    std::fflush(g_file);
}

bool available() { return g_file != nullptr; }

void shutdown() {
    if (!g_file) return;
    #ifdef PS3_GAME_ORBIT_FIX35
    flush_pending();
#endif
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
#ifdef PS3_GAME_ORBIT_FIX35
    std::array<char,2048> line{};va_start(ap,fmt);
    const int n=std::vsnprintf(line.data(),line.size(),fmt,ap);va_end(ap);
    if(n<0)return;
    const auto length=std::min(std::size_t(n),line.size()-1);
#ifdef PS3_GAME_ORBIT_FIX45
    if(pending_lock.test_and_set(std::memory_order_acquire)){++dropped;return;}
#endif
    if(pending.size()+length+1<=MaxPending){pending.append(line.data(),length);pending.push_back('\n');}
    else ++dropped;
#ifdef PS3_GAME_ORBIT_FIX45
    pending_lock.clear(std::memory_order_release);
#endif
#else
    va_start(ap, fmt);
    std::vfprintf(g_file, fmt, ap);
    std::fprintf(g_file, "\n");
    va_end(ap);
    std::fflush(g_file);
#endif
}

#ifdef PS3_GAME_ORBIT_FIX35
void flush_if_idle(bool idle,std::uint64_t now){
    if(idle && now-last_flush>=3000000){flush_pending();last_flush=now;}
}
std::size_t pending_bytes(){
#ifdef PS3_GAME_ORBIT_FIX45
    if(pending_lock.test_and_set(std::memory_order_acquire))return 0;
    const auto size=pending.size();pending_lock.clear(std::memory_order_release);return size;
#else
    return pending.size();
#endif
}
#endif
std::string log_path() { return g_path; }
}
