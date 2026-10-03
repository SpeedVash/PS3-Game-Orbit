#include "webman_client.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <cctype>
#ifdef __PSL1GHT__
#include <net/net.h>
#endif

WebmanClient::WebmanClient(std::string host, int port) : host_(std::move(host)), port_(port) {}

WebmanClient::~WebmanClient() {
#ifdef __PSL1GHT__
    if (network_initialized_) {
        netDeinitialize();
        network_initialized_ = false;
    }
#endif
}

#ifdef __PSL1GHT__
bool WebmanClient::ensure_network() {
    if (network_initialized_) return true;
    const int rc = netInitialize();
    if (rc != 0) {
        char buf[96];
        ::snprintf(buf, sizeof(buf), "netInitialize failed: %d", rc);
        last_error_ = buf;
        return false;
    }
    network_initialized_ = true;
    return true;
}
#endif

std::string WebmanClient::url_encode_path(const std::string& value) {
    static const char hex[] = "0123456789ABCDEF";
    std::string out;
    for (unsigned char c : value) {
        // Preserve PS3 path syntax; encode characters that are unsafe in HTTP request targets.
        if (std::isalnum(c) || c=='/' || c=='_' || c=='-' || c=='.' || c=='~') {
            out.push_back((char)c);
        } else {
            out.push_back('%');
            out.push_back(hex[(c >> 4) & 0xF]);
            out.push_back(hex[c & 0xF]);
        }
    }
    return out;
}

bool WebmanClient::send_get(const std::string& path) {
    last_error_.clear();
#ifdef __PSL1GHT__
    if (!ensure_network()) return false;
#endif

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { last_error_ = "socket() failed"; return false; }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port_);
    addr.sin_addr.s_addr = inet_addr(host_.c_str());
    if (connect(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        last_error_ = "connect() to webMAN failed";
        close(fd);
        return false;
    }

    char request[2048];
    const int n = ::snprintf(request, sizeof(request),
        "GET %s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n",
        path.c_str(), host_.c_str());
    if (n <= 0 || n >= (int)sizeof(request)) {
        last_error_ = "HTTP request too large";
        close(fd);
        return false;
    }

    const char* p = request;
    int left = n;
    while (left > 0) {
        int sent = (int)send(fd, p, left, 0);
        if (sent <= 0) {
            last_error_ = "send() failed";
            close(fd);
            return false;
        }
        p += sent;
        left -= sent;
    }

    char head[96]{};
    int got = (int)recv(fd, head, sizeof(head)-1, 0);
    close(fd);
    if (got <= 0) { last_error_ = "No HTTP response from webMAN"; return false; }

    const std::string response(head, (std::size_t)got);
    const bool http = response.rfind("HTTP/1.", 0) == 0;
    const bool accepted = response.find(" 2") != std::string::npos || response.find(" 3") != std::string::npos;
    if (!http || !accepted) {
        last_error_ = "webMAN returned a non-2xx/3xx HTTP response";
        return false;
    }
    return true;
}

bool WebmanClient::mount_game(const std::string& ps3_path) {
    return send_get("/mount.ps3" + url_encode_path(ps3_path));
}

bool WebmanClient::unmount_game() {
    return send_get("/mount.ps3/unmount");
}
