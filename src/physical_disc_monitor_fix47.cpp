#include "physical_disc_monitor_fix47.h"
#ifdef PS3_GAME_ORBIT_FIX47
#include <algorithm>
#include <utility>
#ifdef __PSL1GHT__
#include <sys/systime.h>
#else
#include <chrono>
#include <system_error>
#endif
namespace {
void pause_disc_worker(unsigned ms){
#ifdef __PSL1GHT__
    sysUsleep(ms*1000);
#else
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
#endif
}
}
PhysicalDiscMonitorFix47::PhysicalDiscMonitorFix47(std::string root,DiscProbeFix47 probe,std::string covers,unsigned ms,Sample sample):
    root_(std::move(root)),covers_(std::move(covers)),probe_(probe),sample_(sample),interval_ms_(std::max(1u,ms)){}
bool PhysicalDiscMonitorFix47::start(const std::vector<GameEntry>& initial){
    if(running_)return true;
    if(!sample_ || !probe_)return false;
    latest_={};changed_=false;stopping_.store(false);
    for(const auto& game:initial)if(game.source==GameSource::Disc){latest_.present=true;latest_.game=game;break;}
#ifdef __PSL1GHT__
    char name[]="orbit_disc";
    if(sysThreadCreate(&thread_,entry,this,1600,256u<<10,THREAD_JOINABLE,name)!=0)return false;
#else
    try{thread_=std::thread(&PhysicalDiscMonitorFix47::run,this);}catch(const std::system_error&){return false;}
#endif
    running_=true;return true;
}
void PhysicalDiscMonitorFix47::stop(){
    if(!running_)return;
    stopping_.store(true);
#ifdef __PSL1GHT__
    u64 result=0;sysThreadJoin(thread_,&result);
#else
    thread_.join();
#endif
    running_=false;
}
bool PhysicalDiscMonitorFix47::take(DiscSampleFix47& result){
    if(!running_ || !try_lock())return false;
    const bool changed=changed_;
    if(changed){result=latest_;changed_=false;}
    unlock();return changed;
}
#ifdef __PSL1GHT__
void PhysicalDiscMonitorFix47::entry(void* self){static_cast<PhysicalDiscMonitorFix47*>(self)->run();sysThreadExit(0);}
#endif
void PhysicalDiscMonitorFix47::run(){
    while(!stopping_.load()){
        // Optical reads happen without holding the lock used by the UI.
        auto sample=sample_(root_,probe_,covers_);
        while(!try_lock()){
            if(stopping_.load())return;
            pause_disc_worker(1);
        }
        if(!stopping_.load() && !same_disc_sample_fix47(latest_,sample)){latest_=std::move(sample);changed_=true;}
        unlock();
        for(unsigned waited=0;waited<interval_ms_ && !stopping_.load();){
            const unsigned step=std::min(25u,interval_ms_-waited);pause_disc_worker(step);waited+=step;
        }
    }
}
#endif
