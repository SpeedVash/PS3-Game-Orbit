#include "performance_fix35.h"
#ifdef PS3_GAME_ORBIT_FIX35
#include "runtime_diag.h"
#include <array>
#ifdef __PSL1GHT__
#include <lv2/systime.h>
#else
#include <chrono>
#endif
namespace OrbitPerformanceFix35 {
namespace {std::array<Metric,static_cast<unsigned>(Kind::Count)> metrics{};}
std::uint64_t now_us(){
#ifdef __PSL1GHT__
    return static_cast<std::uint64_t>(sysGetSystemTime());
#else
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
#endif
}
void record(Kind kind,std::uint64_t elapsed){auto& m=metrics[static_cast<unsigned>(kind)];++m.count;m.total_us+=elapsed;if(elapsed>m.max_us)m.max_us=elapsed;}
Metric metric(Kind kind){return metrics[static_cast<unsigned>(kind)];}
void report(){
    constexpr const char* names[]={"read","decode","upload","interface"};
    for(unsigned i=0;i<metrics.size();++i){const auto& m=metrics[i];
        RuntimeDiag::log("PERF 1.3.2: stage=%s count=%llu total_ms=%.3f average_ms=%.3f max_ms=%.3f",names[i],
            static_cast<unsigned long long>(m.count),double(m.total_us)/1000.0,m.count ? double(m.total_us)/double(m.count)/1000.0 : 0.0,double(m.max_us)/1000.0);
    }
}
}
#endif
