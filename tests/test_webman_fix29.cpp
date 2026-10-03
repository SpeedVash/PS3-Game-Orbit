#include <cassert>
#include <chrono>
#include <cstdio>
#include <thread>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "webman_mount_fix29.h"
#include "mount_operation_fix29.h"

using Clock=std::chrono::steady_clock;
static std::uint64_t milliseconds(){return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now().time_since_epoch()).count();}
class Server {
public:
    int fd=-1,port=0;std::string request;std::thread thread;
    explicit Server(std::string response,bool segmented=true,bool silent=false){
        fd=socket(AF_INET,SOCK_STREAM,0);assert(fd>=0);
        sockaddr_in addr{};addr.sin_family=AF_INET;addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
        assert(bind(fd,reinterpret_cast<sockaddr*>(&addr),sizeof(addr))==0);assert(listen(fd,1)==0);
        socklen_t n=sizeof(addr);assert(getsockname(fd,reinterpret_cast<sockaddr*>(&addr),&n)==0);port=ntohs(addr.sin_port);
        thread=std::thread([this,response,segmented,silent]{
            const int client=accept(fd,nullptr,nullptr);assert(client>=0);
            char b[256];
            while(request.find("\r\n\r\n")==std::string::npos){const auto count=recv(client,b,sizeof(b),0);if(count<=0) break;request.append(b,count);}
            if(silent){std::this_thread::sleep_for(std::chrono::milliseconds(70));}
            else if(segmented){
                for(char c:response){if(send(client,&c,1,MSG_NOSIGNAL)<=0) break;std::this_thread::sleep_for(std::chrono::milliseconds(1));}
            }else send(client,response.data(),response.size(),MSG_NOSIGNAL);
            close(client);
        });
    }
    ~Server(){if(thread.joinable()) thread.join();close(fd);}
};
static void finish(WebmanMountFix29& client,unsigned& ticks){
    const auto end=milliseconds()+2500;
    while(client.busy() && milliseconds()<end){
        const auto before=Clock::now();client.update(milliseconds());
        assert(Clock::now()-before<std::chrono::milliseconds(40));
        ++ticks;std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    assert(!client.busy());
}
int main(){
    unsigned ticks=0;
    {
        Server server("HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n");WebmanMountFix29 client("127.0.0.1",server.port);
        assert(client.start("/dev_usb007/GAMES/Jogo ç [BLES01234]",milliseconds()));
        assert(!client.start("/dev_hdd0/PS3ISO/Other.iso",milliseconds()));finish(client,ticks);
        assert(client.state()==WebmanMountFix29::State::Accepted && client.http_status()==200 && ticks>20);
        server.thread.join();assert(server.request.find("GET /mount_ps3/dev_usb007/GAMES/Jogo%20%C3%A7%20%5BBLES01234%5D HTTP/1.0\r\n")==0);
    }
    puts("PASS: real loopback HTTP, USB007, UTF-8/space/bracket encoding, fragmented status/headers, duplicate-request rejection and nonblocking updates");
    for(const auto& response:{std::string("HTTP/1.0 500 Internal Server Error\r\nX-Number: 200\r\n\r\n"),std::string("HTTP/1.1 302 Redirect\r\nLocation: /setup.ps3\r\n\r\n"),std::string("HTTP/1.1 2000 Nope\r\n\r\n"),std::string("garbage 200\r\n\r\n"),std::string("HTTP/1.1 200 OK\r\n")}){
        Server server(response,false);WebmanMountFix29 client("127.0.0.1",server.port);assert(client.start("/dev_hdd0/PS3ISO/Test.iso",milliseconds()));finish(client,ticks);assert(client.state()==WebmanMountFix29::State::Failed);
    }
    {
        Server server("",false,true);WebmanMountFix29 client("127.0.0.1",server.port);assert(client.start("/dev_usb000/PS3ISO/Test.iso",milliseconds(),25));finish(client,ticks);
        assert(client.error().find("tempo")!=std::string::npos);
    }
    {
        Server server("",false,true);WebmanMountFix29 client("127.0.0.1",server.port);assert(client.start("/dev_hdd0/GAMES/Test",milliseconds()));
        while(client.state()!=WebmanMountFix29::State::Reading){client.update(milliseconds());std::this_thread::sleep_for(std::chrono::milliseconds(1));}
        client.cancel();assert(!client.busy() && client.state()==WebmanMountFix29::State::Cancelled);
    }
    for(const std::string& path:std::vector<std::string>{"DEMO_FIX28_2","/dev_hdd0/tmp/a.iso","/dev_usb008/PS3ISO/Game.iso","/dev_hdd0/PS3ISO/../x","/dev_hdd0/GAMES/x/..","/dev_hdd0/GAMES/x;/shutdown.ps3",std::string("/dev_hdd0/GAMES/x\r\nInjected"),std::string("/dev_hdd0/GAMES/x")+std::string(1,'\0')+"other"}){
        WebmanMountFix29 client;assert(!client.start(path,milliseconds()));assert(client.state()==WebmanMountFix29::State::Failed);
    }
    puts("PASS: HTTP errors/redirects/malformed or partial replies, timeout, cancellation and invalid/demo/command paths fail without success");
    {
        Server server("HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n",false);MountOperationFix29 operation("127.0.0.1",server.port);
        GameEntry g;g.path="/dev_hdd0/GAMES/Jogo [BLES01234]";g.title_id="BLES01234";assert(operation.start(g,milliseconds()));
        while(operation.state()==MountOperationFix29::State::WaitingResponse){operation.update(milliseconds(),"BLES99999");std::this_thread::sleep_for(std::chrono::milliseconds(1));}
        assert(operation.needs_disc_check() && !operation.exit_ready(milliseconds()));
        operation.update(milliseconds(),"BLES99999");assert(operation.needs_disc_check());
        const auto now=milliseconds();operation.update(now,"BLES01234");assert(operation.state()==MountOperationFix29::State::Confirmed && !operation.exit_ready(now));
        assert(operation.exit_ready(now+1200));
    }
    {
        Server server("HTTP/1.1 200 OK\r\n\r\n",false);MountOperationFix29 operation("127.0.0.1",server.port);
        GameEntry g;g.path="/dev_usb000/PS3ISO/Jogo.iso";assert(operation.start(g,milliseconds()));
        while(operation.state()==MountOperationFix29::State::WaitingResponse){operation.update(milliseconds(),"BLES01234");std::this_thread::sleep_for(std::chrono::milliseconds(1));}
        assert(operation.state()==MountOperationFix29::State::Unverified && !operation.exit_ready(milliseconds()+60000));
    }
    puts("PASS: HTTP acceptance alone cannot exit the app; expected disc ID and visible delay required; ISO without known ID stays explicitly unverified");
}
