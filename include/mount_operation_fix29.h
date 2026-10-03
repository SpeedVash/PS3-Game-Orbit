#pragma once
#include "game_entry.h"
#include "webman_mount_fix29.h"

class MountOperationFix29 {
public:
    enum class State { Idle, WaitingResponse, WaitingDisc, Confirmed, Unverified, Failed };
    explicit MountOperationFix29(std::string host="127.0.0.1",int port=80):client_(std::move(host),port){}
    bool start(const GameEntry& game,std::uint64_t now_ms);
    void update(std::uint64_t now_ms,const std::string& mounted_title_id="");
    void cancel(){client_.cancel();if(busy()){state_=State::Failed;status_="Pedido encerrado";}}
    bool busy() const {return state_==State::WaitingResponse || state_==State::WaitingDisc || state_==State::Confirmed;}
    bool needs_disc_check() const {return state_==State::WaitingDisc;}
    bool exit_ready(std::uint64_t now_ms) const {return state_==State::Confirmed && now_ms>=exit_at_;}
    const std::string& status() const {return status_;}
    const std::string& path() const {return path_;}
    State state() const {return state_;}
    int http_status() const {return client_.http_status();}
private:
    WebmanMountFix29 client_;
    State state_=State::Idle;
    std::string expected_id_,path_,status_;
    std::uint64_t deadline_=0,exit_at_=0;
};
