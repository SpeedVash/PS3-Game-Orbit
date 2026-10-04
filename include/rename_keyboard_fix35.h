#pragma once
#ifdef PS3_GAME_ORBIT_FIX35
#include <array>
#include <string>
#include <cstdint>
#ifdef __PSL1GHT__
#include <sysutil/osk.h>
#endif
class RenameKeyboardFix35 {
public:
    bool begin(const std::string& title);
    void poll();
    bool busy()const{return busy_;}
    bool take_result(std::string& title,bool& accepted);
    void shutdown();
private:
    bool busy_=false,done_=false,unloading_=false,unloaded_=false,ready_=false,accepted_=false;
    std::string title_;
#ifdef __PSL1GHT__
    static RenameKeyboardFix35* instance_;
    static void callback(std::uint64_t status,std::uint64_t param,void* userdata);
    sys_mem_container_t container_=0;
    bool registered_=false,container_created_=false;
    std::array<std::uint16_t,97> initial_{},output_{};
    std::array<std::uint16_t,48> prompt_{};
    oskCallbackReturnParam result_{};
#endif
};
#endif
