#include "webman_mount_fix29.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <utility>
#ifdef __PSL1GHT__
#include <net/net.h>
#else
#include <cerrno>
#include <fcntl.h>
#include <poll.h>
#endif

namespace {
bool pending_error(){
#ifdef __PSL1GHT__
    const int e=net_errno;
    return e==NET_EINPROGRESS || e==NET_EALREADY || e==NET_EAGAIN || e==NET_EINTR;
#else
    return errno==EINPROGRESS || errno==EALREADY || errno==EAGAIN || errno==EWOULDBLOCK || errno==EINTR;
#endif
}
bool valid_path(const std::string& p){
    if(p.empty() || p.size()>1024 || p.find('\0')!=std::string::npos) return false;
    if(p.rfind("/dev_hdd0/",0)!=0){
        if(p.size()<13 || p.rfind("/dev_usb00",0)!=0 || p[10]<'0' || p[10]>'7' || p[11]!='/') return false;
    }
    if(p.find("/../")!=std::string::npos || p.find("/./")!=std::string::npos || p.size()<4) return false;
    if(p.compare(p.size()-3,3,"/..")==0 || p.compare(p.size()-2,2,"/.")==0 || p.find(';')!=std::string::npos) return false;
    for(unsigned char c:p) if(c<32 || c==127) return false;
    const auto root=p.find('/',1);if(root==std::string::npos) return false;
    const auto rest=p.substr(root);
    return rest.rfind("/GAMES/",0)==0 || rest.rfind("/GAMEZ/",0)==0 || rest.rfind("/PS3ISO/",0)==0;
}
std::string encode(const std::string& p){
    constexpr char hex[]="0123456789ABCDEF";std::string out;
    for(unsigned char c:p){
        if((c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9') || c=='/' || c=='_' || c=='-' || c=='.' || c=='~') out+=char(c);
        else {out+='%';out+=hex[c>>4];out+=hex[c&15];}
    }
    return out;
}
}
WebmanMountFix29::WebmanMountFix29(std::string host,int port):host_(std::move(host)),port_(port){}
WebmanMountFix29::~WebmanMountFix29(){
    close_socket();
#ifdef __PSL1GHT__
    if(network_initialized_) netDeinitialize();
#endif
}
void WebmanMountFix29::close_socket(){
    if(fd_>=0){
#ifdef __PSL1GHT__
        netClose(SOCKET_FD(fd_));
#else
        close(fd_);
#endif
        fd_=-1;
    }
}
bool WebmanMountFix29::fail(const std::string& message){close_socket();error_=message;state_=State::Failed;return false;}
bool WebmanMountFix29::busy() const {return state_==State::Connecting || state_==State::Sending || state_==State::Reading;}
void WebmanMountFix29::cancel(){if(busy()){close_socket();state_=State::Cancelled;error_="Pedido cancelado";}}
bool WebmanMountFix29::start(const std::string& path,std::uint64_t now_ms,unsigned timeout_ms){
    if(busy()) return false;
    close_socket();error_.clear();response_.clear();http_status_=0;sent_=0;target_.clear();
    if(!valid_path(path)) return fail("Caminho de jogo invalido");
    // mount_ps3 is the documented homebrew/in-game variant. mount.ps3 may
    // reject mounting while this application is running.
    target_="/mount_ps3"+encode(path);
    request_="GET "+target_+" HTTP/1.0\r\nHost: "+host_+"\r\nConnection: close\r\n\r\n";
    if(request_.size()>=2048) return fail("Caminho longo demais para webMAN");
    if(port_<=0 || port_>65535 || host_.find_first_of("\r\n")!=std::string::npos) return fail("Endereco webMAN invalido");
#ifdef __PSL1GHT__
    if(!network_initialized_){if(netInitialize()!=0) return fail("Rede indisponivel");network_initialized_=true;}
#endif
    fd_=socket(AF_INET,SOCK_STREAM,0);
    if(fd_<0) return fail("Nao foi possivel abrir a conexao");
#ifdef __PSL1GHT__
    const int nonblocking=1;
    if(setsockopt(fd_,SOL_SOCKET,SO_NBIO,&nonblocking,sizeof(nonblocking))!=0) return fail("Rede sem modo assincrono");
#else
    const int flags=fcntl(fd_,F_GETFL,0);
    if(flags<0 || fcntl(fd_,F_SETFL,flags|O_NONBLOCK)!=0) return fail("Rede sem modo assincrono");
#endif
    sockaddr_in address{};
    address.sin_family=AF_INET;address.sin_port=htons(static_cast<std::uint16_t>(port_));
    if(inet_pton(AF_INET,host_.c_str(),&address.sin_addr)!=1) return fail("Endereco webMAN invalido");
    deadline_=now_ms+timeout_ms;state_=State::Connecting;
    const int rc=connect(fd_,reinterpret_cast<sockaddr*>(&address),sizeof(address));
    if(rc==0) state_=State::Sending;
    else if(!pending_error()) return fail("webMAN indisponivel: conexao recusada");
    return true;
}
void WebmanMountFix29::update(std::uint64_t now_ms){
    if(!busy()) return;
    if(now_ms>=deadline_){fail("webMAN nao respondeu a tempo");return;}
    pollfd descriptor{};descriptor.fd=fd_;
    descriptor.events=state_==State::Reading ? POLLIN : POLLOUT;
    const int ready=poll(&descriptor,1,0);
    if(ready<0){if(!pending_error()) fail("Falha ao consultar webMAN");return;}
    if(!ready) return;
    if(state_==State::Connecting){
        int error=0;socklen_t length=sizeof(error);
        if(getsockopt(fd_,SOL_SOCKET,SO_ERROR,&error,&length)!=0 || error!=0){fail("webMAN indisponivel: conexao recusada");return;}
        state_=State::Sending;
    }
    if(state_==State::Sending){
#ifdef MSG_NOSIGNAL
        constexpr int flags=MSG_NOSIGNAL;
#else
        constexpr int flags=0;
#endif
        const auto sent=send(fd_,request_.data()+sent_,request_.size()-sent_,flags);
        if(sent<0){if(!pending_error()) fail("Falha no envio ao webMAN");return;}
        if(sent==0){fail("Conexao webMAN encerrada");return;}
        sent_+=static_cast<std::size_t>(sent);
        if(sent_==request_.size()) state_=State::Reading;
        return;
    }
    if(state_!=State::Reading) return;
    char bytes[1024];const auto received=recv(fd_,bytes,sizeof(bytes),0);
    if(received<0){if(!pending_error()) fail("Falha na resposta webMAN");return;}
    if(received==0){fail("Resposta webMAN incompleta");return;}
    response_.append(bytes,static_cast<std::size_t>(received));
    if(response_.size()>8192){fail("Resposta webMAN longa demais");return;}
    const auto line=response_.find("\r\n");
    if(line==std::string::npos) return;
    if(response_.rfind("HTTP/1.0 ",0)!=0 && response_.rfind("HTTP/1.1 ",0)!=0){fail("Resposta HTTP invalida");return;}
    if(line<12 || response_[9]<'0' || response_[9]>'9' || response_[10]<'0' || response_[10]>'9' || response_[11]<'0' || response_[11]>'9' || (line>12 && response_[12]!=' ')){
        fail("Status HTTP invalido");return;
    }
    http_status_=(response_[9]-'0')*100+(response_[10]-'0')*10+(response_[11]-'0');
    if(http_status_<200 || http_status_>=300){fail("webMAN respondeu HTTP "+std::to_string(http_status_));return;}
    if(response_.find("\r\n\r\n")==std::string::npos) return;
    close_socket();state_=State::Accepted; // HTTP acceptance is not mount proof.
}
