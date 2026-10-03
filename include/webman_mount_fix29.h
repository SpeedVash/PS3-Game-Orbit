#pragma once
#include <cstdint>
#include <string>

// Cooperative, nonblocking HTTP client. No worker changes RSX resources.
class WebmanMountFix29 {
public:
    enum class State { Idle, Connecting, Sending, Reading, Accepted, Failed, Cancelled };
    explicit WebmanMountFix29(std::string host="127.0.0.1",int port=80);
    ~WebmanMountFix29();
    WebmanMountFix29(const WebmanMountFix29&)=delete;
    WebmanMountFix29& operator=(const WebmanMountFix29&)=delete;
    bool start(const std::string& path,std::uint64_t now_ms,unsigned timeout_ms=10000);
    void update(std::uint64_t now_ms);
    void cancel();
    bool busy() const;
    State state() const {return state_;}
    int http_status() const {return http_status_;}
    const std::string& error() const {return error_;}
    const std::string& request_target() const {return target_;}
private:
    std::string host_,request_,response_,error_,target_;
    int port_,fd_=-1,http_status_=0;
    bool network_initialized_=false;
    std::size_t sent_=0;
    std::uint64_t deadline_=0;
    State state_=State::Idle;
    void close_socket();
    bool fail(const std::string& message);
};
