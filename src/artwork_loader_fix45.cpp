#include "artwork_loader_fix45.h"
#ifdef PS3_GAME_ORBIT_FIX45
#include <algorithm>
#ifdef __PSL1GHT__
#include <sys/systime.h>
#else
#include <chrono>
#include <system_error>
#endif
namespace {
void pause_worker(){
#ifdef __PSL1GHT__
    sysUsleep(2000);
#else
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
#endif
}
bool contains(const std::vector<ArtworkRequestFix45>& jobs,const ArtworkRequestFix45& job){
    return std::find(jobs.begin(),jobs.end(),job)!=jobs.end();
}
}
void ArtworkLoaderFix45::worker_lock(){while(!try_lock())pause_worker();}
bool ArtworkLoaderFix45::start(){
    if(running_)return true;
    stopping_.store(false);desired_.reserve(MaxRequests);pending_.reserve(MaxRequests);ready_.reserve(MaxReady);
#ifdef __PSL1GHT__
    char name[]="orbit_artwork";
    if(sysThreadCreate(&thread_,entry,this,1500,1u<<20,THREAD_JOINABLE,name)!=0)return false;
#else
    try{thread_=std::thread(&ArtworkLoaderFix45::run,this);}catch(const std::system_error&){return false;}
#endif
    running_=true;return true;
}
void ArtworkLoaderFix45::stop(){
    if(!running_)return;
    stopping_.store(true);
#ifdef __PSL1GHT__
    u64 result=0;sysThreadJoin(thread_,&result);
#else
    thread_.join();
#endif
    running_=false;desired_.clear();pending_.clear();ready_.clear();active_=false;
}
#ifdef __PSL1GHT__
void ArtworkLoaderFix45::entry(void* self){static_cast<ArtworkLoaderFix45*>(self)->run();sysThreadExit(0);}
#endif
bool ArtworkLoaderFix45::request(std::vector<ArtworkRequestFix45> priority){
    if(priority.size()>MaxRequests)priority.resize(MaxRequests);
    std::vector<ArtworkRequestFix45> next;next.reserve(MaxRequests);
    std::vector<std::unique_ptr<PreparedArtworkFix45>> obsolete;obsolete.reserve(MaxReady);
    if(!running_ || !try_lock())return false;
    const auto epoch=generation();
    for(auto it=ready_.begin();it!=ready_.end();){
        if((*it)->request.generation!=epoch || !contains(priority,(*it)->request)){
            obsolete.push_back(std::move(*it));it=ready_.erase(it);++discarded_;
        }else ++it;
    }
    for(const auto& job:priority){
        if(job.generation!=epoch || (active_ && job==active_request_))continue;
        bool prepared=false;for(const auto& item:ready_)if(item->request==job){prepared=true;break;}
        if(!prepared && !contains(next,job))next.push_back(job);
    }
    // Move/swap only; freeing obsolete pixels happens after releasing the lock.
    desired_.swap(priority);pending_.swap(next);
    std::stable_sort(ready_.begin(),ready_.end(),[&](const std::unique_ptr<PreparedArtworkFix45>& a,const std::unique_ptr<PreparedArtworkFix45>& b){
        return std::find(desired_.begin(),desired_.end(),a->request)<std::find(desired_.begin(),desired_.end(),b->request);
    });
    unlock();return true;
}
std::unique_ptr<PreparedArtworkFix45> ArtworkLoaderFix45::take(){
    if(!running_ || !try_lock())return {};
    std::unique_ptr<PreparedArtworkFix45> result;
    if(!ready_.empty()){result=std::move(ready_.front());ready_.erase(ready_.begin());}
    unlock();return result;
}
void ArtworkLoaderFix45::run(){
    while(!stopping_.load()){
        ArtworkRequestFix45 job;bool found=false;
        worker_lock();
        if(ready_.size()<MaxReady && !pending_.empty()){
            job=std::move(pending_.front());pending_.erase(pending_.begin());
            if(job.generation==generation()){active_request_=job;active_=true;found=true;}
        }
        unlock();
        if(!found){pause_worker();continue;}
        auto prepared=std::unique_ptr<PreparedArtworkFix45>(new PreparedArtworkFix45(prepare_(job)));
        ++decoded_;
        worker_lock();active_=false;
        if(!stopping_.load() && job.generation==generation() && contains(desired_,job)){
            ready_.push_back(std::move(prepared));
            if(ready_.size()>peak_ready_.load())peak_ready_.store(ready_.size());
        }else ++discarded_;
        unlock();
    }
}
#endif
