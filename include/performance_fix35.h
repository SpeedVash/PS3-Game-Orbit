#pragma once
#include <cstdint>
#ifdef PS3_GAME_ORBIT_FIX35
namespace OrbitPerformanceFix35 {
enum class Kind { Read, Decode, Upload, Interface, Count };
struct Metric {std::uint64_t count=0,total_us=0,max_us=0;};
std::uint64_t now_us();
void record(Kind kind,std::uint64_t elapsed_us);
Metric metric(Kind kind);
void report();
class Scope {
public:
    explicit Scope(Kind kind):kind_(kind),start_(now_us()){}
    ~Scope(){record(kind_,now_us()-start_);}
private: Kind kind_;std::uint64_t start_;
};
}
#endif
