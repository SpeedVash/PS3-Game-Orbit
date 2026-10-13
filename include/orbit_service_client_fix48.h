#pragma once
#include "game_entry.h"
#include "orbit_service_protocol_fix48.h"
#include <memory>
#include <functional>
#include <cstdint>
#include <string>
#include <vector>
namespace OrbitService48 {
Orbit48FanConfig defaults();
std::string fan_mode_name(unsigned mode);
std::string error_message(std::uint32_t error);
bool load_settings(const std::string& path,Orbit48FanConfig& config);
bool save_settings(const std::string& path,const Orbit48FanConfig& config);
std::vector<unsigned char> native_payload(const GameEntry& game,std::string& error);
std::vector<unsigned char> raw_payload(const Orbit48RawMap& map);
bool bootstrap(std::string& error);
bool exchange(bool mount,unsigned command,const std::vector<unsigned char>& payload,
    std::uint64_t sequence,std::uint32_t& native_error,std::vector<unsigned char>& reply,
    const std::function<bool()>& cancelled={},std::uint64_t timeout_ms=0);
struct Snapshot {
    bool ready=false;std::uint64_t revision=0;Orbit48FanStatus status{};std::string message;
};
}
class OrbitServiceClientFix48 {
public:
    OrbitServiceClientFix48();~OrbitServiceClientFix48();
    OrbitServiceClientFix48(const OrbitServiceClientFix48&)=delete;
    OrbitServiceClientFix48& operator=(const OrbitServiceClientFix48&)=delete;
    bool start();void stop();
    std::uint64_t configure(const Orbit48FanConfig& config);
    bool take(OrbitService48::Snapshot& snapshot);
private:struct Impl;std::unique_ptr<Impl> impl_;
};
class NativeMountFix48 {
public:
    enum class State { Idle, Waiting, Accepted, Failed };
    NativeMountFix48();~NativeMountFix48();
    bool start(const GameEntry& game,std::uint64_t now);
    void update(std::uint64_t now);void cancel();
    State state() const{return state_;}
    const std::string& error() const{return error_;}
private:struct Impl;std::unique_ptr<Impl> impl_;State state_=State::Idle;std::string error_;
};
