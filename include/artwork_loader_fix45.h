#pragma once
#ifdef PS3_GAME_ORBIT_FIX45
#include "prepared_artwork_fix45.h"
#include <atomic>
#include <memory>
#ifdef __PSL1GHT__
#include <sys/thread.h>
#else
#include <thread>
#endif

class ArtworkLoaderFix45 {
public:
    static constexpr std::size_t MaxRequests=64,MaxReady=2;
    using Prepare=PreparedArtworkFix45 (*)(const ArtworkRequestFix45&);
    explicit ArtworkLoaderFix45(Prepare prepare=prepare_artwork_fix45):prepare_(prepare){}
    ~ArtworkLoaderFix45(){stop();}
    ArtworkLoaderFix45(const ArtworkLoaderFix45&)=delete;
    ArtworkLoaderFix45& operator=(const ArtworkLoaderFix45&)=delete;
    bool start();
    void stop();
    bool running() const{return running_;}
    // Main never waits for the worker lock. Both operations can be retried
    // next frame; even a slow disk read cannot hold this lock.
    bool request(std::vector<ArtworkRequestFix45> priority);
    std::unique_ptr<PreparedArtworkFix45> take();
    std::uint64_t generation() const{return generation_.load();}
    void cancel(){generation_.fetch_add(1);}
    std::uint64_t decoded() const{return decoded_.load();}
    std::uint64_t discarded() const{return discarded_.load();}
    std::size_t peak_ready() const{return peak_ready_.load();}
private:
    Prepare prepare_;
    std::atomic_flag lock_=ATOMIC_FLAG_INIT;
    std::atomic<bool> stopping_{false};
    std::atomic<std::uint64_t> generation_{1},decoded_{0},discarded_{0};
    std::atomic<std::size_t> peak_ready_{0};
    bool running_=false,active_=false;
    ArtworkRequestFix45 active_request_;
    std::vector<ArtworkRequestFix45> desired_,pending_;
    std::vector<std::unique_ptr<PreparedArtworkFix45>> ready_;
#ifdef __PSL1GHT__
    sys_ppu_thread_t thread_=0;
    static void entry(void* self);
#else
    std::thread thread_;
#endif
    bool try_lock(){return !lock_.test_and_set(std::memory_order_acquire);}
    void unlock(){lock_.clear(std::memory_order_release);}
    void worker_lock();
    void run();
};
#endif
