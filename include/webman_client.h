#pragma once
#include <string>

class WebmanClient {
public:
    explicit WebmanClient(std::string host = "127.0.0.1", int port = 80);
    ~WebmanClient();

    bool mount_game(const std::string& ps3_path);
    bool unmount_game();
    const std::string& last_error() const { return last_error_; }

private:
    std::string host_;
    int port_;
    std::string last_error_;
#ifdef __PSL1GHT__
    bool network_initialized_ = false;
    bool ensure_network();
#endif
    bool send_get(const std::string& path);
    static std::string url_encode_path(const std::string& value);
};
