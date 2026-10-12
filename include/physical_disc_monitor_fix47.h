#pragma once
#ifdef PS3_GAME_ORBIT_FIX47
#include "physical_disc_fix47.h"
#include <atomic>
#ifdef __PSL1GHT__
#include <sys/thread.h>
#else
#include <thread>
#endif

class PhysicalDiscMonitorFix47 {
public:
    using Sample=DiscSampleFix47 (*)(const std::string&,DiscProbeFix47,const std::string&);
    explicit PhysicalDiscMonitorFix47(std::string root="/dev_bdvd",DiscProbeFix47 probe=probe_disc_access_fix47,
        std::string covers="/dev_hdd0/PS3COVERS",unsigned interval_ms=1000,Sample sample=read_disc_sample_fix47);
    ~PhysicalDiscMonitorFix47(){stop();}
    PhysicalDiscMonitorFix47(const PhysicalDiscMonitorFix47&)=delete;
    PhysicalDiscMonitorFix47& operator=(const PhysicalDiscMonitorFix47&)=delete;
    bool start(const std::vector<GameEntry>& initial);
    void stop();
    bool take(DiscSampleFix47& result);
private:
    std::string root_,covers_;
    DiscProbeFix47 probe_;
    Sample sample_;
    unsigned interval_ms_;
    std::atomic<bool> stopping_{false};
    std::atomic_flag lock_=ATOMIC_FLAG_INIT;
    bool running_=false,changed_=false;
    DiscSampleFix47 latest_;
#ifdef __PSL1GHT__
    sys_ppu_thread_t thread_=0;
    static void entry(void* self);
#else
    std::thread thread_;
#endif
    bool try_lock(){return !lock_.test_and_set(std::memory_order_acquire);}
    void unlock(){lock_.clear(std::memory_order_release);}
    void run();
};
#endif
