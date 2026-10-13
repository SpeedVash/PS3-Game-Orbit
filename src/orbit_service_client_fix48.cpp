#include "orbit_service_client_fix48.h"
#ifdef PS3_GAME_ORBIT_FIX48
#include "iso_metadata_fix48.h"
#include "ntfs_catalog_fix48.h"
#include "metadata_utils.h"
#include "file_store_fix38.h"
#include "raw_reader_fix48.h"
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <functional>
#include <sys/stat.h>
#include <unistd.h>
#ifdef __PSL1GHT__
#include <sys/thread.h>
#include <lv2/thread.h>
#include <lv2/systime.h>
#include <ppu-lv2.h>
#else
#include <thread>
#endif
namespace {
using Bytes=std::vector<unsigned char>;
std::atomic_flag bootstrap_lock=ATOMIC_FLAG_INIT;
std::atomic<bool> service_ready{false};
std::uint64_t clock_us(){
#ifdef __PSL1GHT__
    return std::uint64_t(sysGetSystemTime());
#else
    return std::uint64_t(std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
#endif
}
void pause_ms(unsigned n){::usleep(n*1000);}
void lock(std::atomic_flag& flag){while(flag.test_and_set(std::memory_order_acquire))pause_ms(1);}
void unlock(std::atomic_flag& flag){flag.clear(std::memory_order_release);}
bool read_bytes(const std::string& path,Bytes& b,std::size_t cap=ORBIT48_HEADER_BYTES+ORBIT48_MAX_PAYLOAD){
    struct stat info{};if(stat(path.c_str(),&info) || !S_ISREG(info.st_mode) ||
        info.st_size<0 || std::uint64_t(info.st_size)>cap)return false;
    FILE* file=std::fopen(path.c_str(),"rb");if(!file)return false;
    b.resize(std::size_t(info.st_size));const bool ok=std::fread(b.data(),1,b.size(),file)==b.size() && !std::ferror(file);
    std::fclose(file);return ok;
}
bool write_bytes(const std::string& path,const Bytes& b){
    const auto temp=path+".tmp";FILE* f=std::fopen(temp.c_str(),"wb");if(!f)return false;
    bool ok=std::fwrite(b.data(),1,b.size(),f)==b.size() && std::fflush(f)==0 && fsync(fileno(f))==0;
    if(std::fclose(f))ok=false;
#ifdef PS3_GAME_ORBIT_FIX38
    if(ok)ok=FileStoreFix38::commit(temp,path);
#else
    if(ok)ok=!std::rename(temp.c_str(),path.c_str());
#endif
    if(!ok)std::remove(temp.c_str());
    return ok;
}
std::uint32_t hash(const unsigned char* p,std::size_t n){
    std::uint32_t v=2166136261u;for(std::size_t i=0;i<n;++i)v=(v^p[i])*16777619u;return v;
}
Bytes config_bytes(const Orbit48FanConfig& c){
    Bytes b(20);orbit48_put32(b.data(),c.mode);orbit48_put32(b.data()+4,c.target_c);
    orbit48_put32(b.data()+8,c.minimum_percent);orbit48_put32(b.data()+12,c.maximum_percent);
    orbit48_put32(b.data()+16,c.manual_percent);return b;
}
Orbit48FanConfig decode_config(const unsigned char* p){
    return {orbit48_be32(p),orbit48_be32(p+4),orbit48_be32(p+8),orbit48_be32(p+12),orbit48_be32(p+16)};
}
bool status(const Bytes& b,Orbit48FanStatus& out){
    if(b.size()<40)return false;
    out.config=decode_config(b.data());if(!orbit48_valid_fan_config(&out.config))return false;
    out.state=orbit48_be32(b.data()+20);out.cpu_c=orbit48_be32(b.data()+24);
    out.rsx_c=orbit48_be32(b.data()+28);out.percent=orbit48_be32(b.data()+32);
    out.native_error=orbit48_be32(b.data()+36);return out.state<=ORBIT48_FAN_EMERGENCY && out.percent<=100;
}
class Thread {
public:
    ~Thread(){join();}
    bool start(std::function<void()> fn,const char* name,unsigned stack=256*1024){
        function_=std::move(fn);
#ifdef __PSL1GHT__
        if(sysThreadCreate(&id_,entry,this,1800,stack,THREAD_JOINABLE,const_cast<char*>(name)))return false;
        started_=true;
#else
        (void)name;(void)stack;
        try{thread_=std::thread([this]{function_();});started_=true;}catch(...){return false;}
#endif
        return true;
    }
    void join(){
        if(!started_)return;
#ifdef __PSL1GHT__
        u64 value=0;sysThreadJoin(id_,&value);
#else
        thread_.join();
#endif
        started_=false;
    }
private:
    std::function<void()> function_;bool started_=false;
#ifdef __PSL1GHT__
    sys_ppu_thread_t id_=0;
    static void entry(void* context){static_cast<Thread*>(context)->function_();sysThreadExit(0);}
#else
    std::thread thread_;
#endif
};
}
namespace OrbitService48 {
Orbit48FanConfig defaults(){return {ORBIT48_FAN_AUTO,68,35,85,40};}
std::string fan_mode_name(unsigned mode){return mode==ORBIT48_FAN_AUTO?"Automático":mode==ORBIT48_FAN_MANUAL?"Manual":"Padrão do PS3";}
std::string error_message(std::uint32_t error){
    switch(error){
    case 0:return {};
    case ORBIT48_BAD_REQUEST:return "O serviço recebeu um pedido inválido.";
    case ORBIT48_NO_COBRA:return "Ative o HEN ou o Cobra para montar os jogos.";
    case ORBIT48_BAD_PATH:return "O jogo ou uma parte da ISO está indisponível.";
    case ORBIT48_BAD_IMAGE:return "Os dados do jogo não puderam ser verificados.";
    case ORBIT48_USB_CHANGED:return "O USB NTFS foi retirado ou trocado.";
    case ORBIT48_BUSY:return "O serviço ainda está ocupado.";
    case ORBIT48_CANCELLED:return "Montagem cancelada.";
    case ORBIT48_FAN_CONFLICT:return "Outro controlador está ativo; ventoinha do Orbit suspensa.";
    case ORBIT48_FAN_UNAVAILABLE:return "Controle da ventoinha indisponível neste firmware.";
    default:{char text[96];std::snprintf(text,sizeof(text),"O console recusou a operação (0x%08X).",unsigned(error));return text;}
    }
}
bool load_settings(const std::string& path,Orbit48FanConfig& config){
    for(const auto& candidate:{path,path+".bak"}){
        Bytes b;if(!read_bytes(candidate,b,64) || b.size()!=32 || std::memcmp(b.data(),"FAN48CFG",8) ||
            orbit48_be32(b.data()+28)!=hash(b.data(),28))continue;
        const auto c=decode_config(b.data()+8);if(!orbit48_valid_fan_config(&c))continue;
        config=c;return true;
    }
    return false;
}
bool save_settings(const std::string& path,const Orbit48FanConfig& config){
    if(!orbit48_valid_fan_config(&config))return false;
    Bytes b(32);std::memcpy(b.data(),"FAN48CFG",8);const auto c=config_bytes(config);
    std::memcpy(b.data()+8,c.data(),20);orbit48_put32(b.data()+28,hash(b.data(),28));return write_bytes(path,b);
}
Bytes native_payload(const GameEntry& game,std::string& error){
    std::vector<std::string> parts;
    if(game.format==GameFormat::Folder){
        struct stat info{};if(stat((game.path+"/PS3_GAME/PARAM.SFO").c_str(),&info) || !S_ISREG(info.st_mode)){
            error="A pasta do jogo ficou indisponível.";return {};
        }
        const auto actual=read_param_sfo_string(game.path+"/PS3_GAME/PARAM.SFO","TITLE_ID");
        if(actual.empty() || actual!=game.title_id){error="O jogo da pasta foi trocado. Atualize a biblioteca.";return {};}
        parts.push_back(game.path);
    }else if(game.format==GameFormat::ISO){
        if(!OrbitIso48::native_parts(game.path,parts,error))return {};
        const auto metadata=OrbitIso48::native_metadata(parts);
        if(!metadata.id.empty() && !game.title_id.empty() && metadata.id!=game.title_id){
            error="A ISO foi trocada. Atualize a biblioteca.";return {};
        }
    }else{error="Formato de jogo não suportado.";return {};}
    if(game.title_id.size()!=0 && game.title_id.size()!=9){error="ID do jogo inválida.";return {};}
    Bytes b(18,0);orbit48_put32(b.data(),game.format==GameFormat::Folder?1:0);orbit48_put32(b.data()+4,unsigned(parts.size()));
    if(!game.title_id.empty())std::memcpy(b.data()+8,game.title_id.data(),9);
    for(const auto& part:parts){
        if(part.empty() || part.size()>=ORBIT48_PATH_BYTES){error="Caminho do jogo excede o limite.";return {};}
        const auto at=b.size();b.resize(at+4+part.size());orbit48_put32(b.data()+at,unsigned(part.size()));
        std::memcpy(b.data()+at+4,part.data(),part.size());
    }return b;
}
Bytes raw_payload(const Orbit48RawMap& map){
    if(!orbit48_validate_raw_map(&map))return {};
    Bytes b(48+map.count*24);orbit48_put64(b.data(),map.device_id);orbit48_put64(b.data()+8,map.partition_lba);
    orbit48_put64(b.data()+16,map.volume_serial);orbit48_put64(b.data()+24,map.device_sectors);
    orbit48_put64(b.data()+32,map.iso_size);orbit48_put32(b.data()+40,map.sector_size);orbit48_put32(b.data()+44,map.count);
    for(unsigned i=0;i<map.count;++i){auto* p=b.data()+48+i*24;orbit48_put64(p,map.extents[i].iso_offset);
        orbit48_put64(p+8,map.extents[i].device_lba);orbit48_put64(p+16,map.extents[i].sector_count);}
    return b;
}
bool exchange(bool mount,unsigned command,const Bytes& payload,std::uint64_t sequence,
    std::uint32_t& error,Bytes& response,const std::function<bool()>& cancelled,std::uint64_t timeout_ms){
    if(payload.size()>ORBIT48_MAX_PAYLOAD){error=ORBIT48_BAD_REQUEST;return false;}
    Bytes b(ORBIT48_HEADER_BYTES+payload.size(),0);std::memcpy(b.data(),"PGOR",4);b[4]=1;b[5]=6;
    b[6]=static_cast<unsigned char>(command>>8);b[7]=static_cast<unsigned char>(command);
    orbit48_put64(b.data()+8,sequence);orbit48_put32(b.data()+16,unsigned(payload.size()));
    std::copy(payload.begin(),payload.end(),b.begin()+ORBIT48_HEADER_BYTES);
    const std::string input=mount?ORBIT48_MOUNT_REQUEST:ORBIT48_CONTROL_REQUEST,output=mount?ORBIT48_MOUNT_REPLY:ORBIT48_CONTROL_REPLY;
    if(!write_bytes(input,b)){error=ORBIT48_BAD_REQUEST;return false;}
    const auto deadline=clock_us()+(timeout_ms?timeout_ms*1000:(mount?20000000ull:3000000ull));bool cancel_sent=false;
    do{
        if(cancelled && cancelled() && mount && !cancel_sent){
            Bytes token(8);orbit48_put64(token.data(),sequence);write_bytes(ORBIT48_CANCEL_PATH,token);cancel_sent=true;
        }
        Bytes received;
        if(read_bytes(output,received) && received.size()>=ORBIT48_HEADER_BYTES &&
           !std::memcmp(received.data(),"PGOR",4) && received[4]==1 && received[5]==6 &&
           ((unsigned(received[6])<<8)|received[7])==(command|0x8000u) &&
           orbit48_be64(received.data()+8)==sequence &&
           orbit48_be32(received.data()+16)==received.size()-ORBIT48_HEADER_BYTES){
            error=orbit48_be32(received.data()+20);response.assign(received.begin()+ORBIT48_HEADER_BYTES,received.end());return true;
        }
        if(cancelled && cancelled() && !mount){error=ORBIT48_CANCELLED;return false;}
        pause_ms(25);
    }while(clock_us()<deadline);
    error=ORBIT48_BUSY;return false;
}
bool bootstrap(std::string& error){
    if(service_ready.load(std::memory_order_acquire))return true;
    lock(bootstrap_lock);
    if(service_ready.load(std::memory_order_acquire)){unlock(bootstrap_lock);return true;}
    bool loaded=false;
#ifdef __PSL1GHT__
    std::uint32_t version=0;int code=0;
    {lv2syscall2(8,0x7000,reinterpret_cast<std::uintptr_t>(&version));code=static_cast<int>(p1);}
    if(code || !version){error=error_message(ORBIT48_NO_COBRA);unlock(bootstrap_lock);return false;}
    std::vector<unsigned> slots;
    for(unsigned i=0;i<7;++i){
        char name[64]{},path[1024]{};
        {lv2syscall5(8,0x7777,0x0047,i,reinterpret_cast<std::uintptr_t>(name),reinterpret_cast<std::uintptr_t>(path));code=static_cast<int>(p1);}
        name[63]=0;path[1023]=0;
        if(!code && std::string(path)==ORBIT48_PLUGIN_PATH){loaded=true;break;}
        if(!path[0] && i>0)slots.push_back(i);
    }
    if(!loaded){
        std::uint32_t probe_error=0;Bytes probe_response;
        if(exchange(false,ORBIT48_STATUS,{},clock_us(),probe_error,probe_response,{},500) &&
           !probe_error && probe_response.size()==44){
            service_ready.store(true,std::memory_order_release);unlock(bootstrap_lock);return true;
        }
        struct stat info{};
        if(stat(ORBIT48_PLUGIN_PATH,&info) || !S_ISREG(info.st_mode)){error="O plugin do Orbit não foi encontrado. Reinstale o PKG.";unlock(bootstrap_lock);return false;}
        for(const unsigned slot:slots){
            {lv2syscall5(8,0x1ee7,slot,reinterpret_cast<std::uintptr_t>(ORBIT48_PLUGIN_PATH),0,0);code=static_cast<int>(p1);}
            if(!code){loaded=true;break;}
        }
        if(!loaded){error="Não há slot livre para o serviço do Orbit.";unlock(bootstrap_lock);return false;}
    }
#else
    loaded=true;
#endif
    std::uint32_t native_error=0;Bytes response;
    const bool ready=loaded && exchange(false,ORBIT48_STATUS,{},clock_us(),native_error,response) &&
        !native_error && response.size()==44;
    service_ready.store(ready,std::memory_order_release);unlock(bootstrap_lock);
    if(!ready)error="O serviço do Orbit não respondeu. Reinicie o console e tente novamente.";
    return ready;
}
}
struct OrbitServiceClientFix48::Impl {
    Thread worker;std::atomic<bool> stop{false};std::atomic_flag mutex=ATOMIC_FLAG_INIT;
    bool dirty=false,pending=false;std::uint64_t revision=0;OrbitService48::Snapshot snapshot;
    Orbit48FanConfig requested=OrbitService48::defaults();
    void publish(OrbitService48::Snapshot s){lock(mutex);snapshot=std::move(s);dirty=true;unlock(mutex);}
    void run(){
        OrbitService48::Snapshot s;std::string message;
        if(!OrbitService48::bootstrap(message)){s.message=message;publish(std::move(s));return;}
        s.ready=true;auto configured=OrbitService48::defaults();OrbitService48::load_settings(ORBIT48_SETTINGS_PATH,configured);
        {lock(mutex);if(!pending && revision==0){requested=configured;pending=true;}unlock(mutex);}
        while(true){
            Orbit48FanConfig c{};bool apply=false;std::uint64_t request_revision=0;
            lock(mutex);apply=pending;if(apply){c=requested;pending=false;request_revision=revision;}unlock(mutex);
            // Finish the last requested setting before the interface leaves.
            // Status polling can stop immediately; configuration needs its ACK.
            if(!apply && stop.load(std::memory_order_acquire))break;
            std::uint32_t code=0;Bytes response;
            const bool ok=OrbitService48::exchange(false,apply?ORBIT48_CONFIGURE:ORBIT48_STATUS,
                apply?config_bytes(c):Bytes{},clock_us(),code,response,[&]{return !apply && stop.load(std::memory_order_acquire);});
            if(ok && status(response,s.status)){
                s.message=OrbitService48::error_message(code);
                if(!code && apply && !OrbitService48::save_settings(ORBIT48_SETTINGS_PATH,c))s.message="Ajuste aplicado; configuração não pôde ser salva.";
                else if(!code){
                    if(s.status.state==ORBIT48_FAN_EXTERNAL)s.message="Outro controlador ativo: Orbit suspenso.";
                    else if(s.status.state==ORBIT48_FAN_UNSUPPORTED)s.message="Ventoinha indisponível neste firmware.";
                    else if(s.status.state==ORBIT48_FAN_SENSOR_FAILURE)s.message="Falha ao ler temperatura; proteção SYSCON.";
                    else if(s.status.state==ORBIT48_FAN_STARTING_GAME)s.message="Aguardando a inicialização do jogo.";
                    else if(s.status.state==ORBIT48_FAN_EMERGENCY)s.message="Temperatura alta: velocidade elevada.";
                    else s.message="Serviço do Orbit ativo.";
                }
            }else s.message=OrbitService48::error_message(code?code:std::uint32_t(ORBIT48_BUSY));
            if(apply)s.revision=request_revision;
            publish(s);
            for(unsigned i=0;i<20 && !stop.load(std::memory_order_acquire);++i){
                pause_ms(50);if(mutex.test_and_set(std::memory_order_acquire))continue;
                const bool next=pending;unlock(mutex);if(next)break;
            }
        }
    }
};
OrbitServiceClientFix48::OrbitServiceClientFix48():impl_(new Impl){}
OrbitServiceClientFix48::~OrbitServiceClientFix48(){stop();}
bool OrbitServiceClientFix48::start(){return impl_->worker.start([this]{impl_->run();},"Orbit service client");}
void OrbitServiceClientFix48::stop(){impl_->stop.store(true,std::memory_order_release);impl_->worker.join();}
std::uint64_t OrbitServiceClientFix48::configure(const Orbit48FanConfig& c){
    if(!orbit48_valid_fan_config(&c))return 0;
    lock(impl_->mutex);impl_->requested=c;impl_->pending=true;const auto revision=++impl_->revision;unlock(impl_->mutex);
    return revision;
}
bool OrbitServiceClientFix48::take(OrbitService48::Snapshot& out){
    if(impl_->mutex.test_and_set(std::memory_order_acquire))return false;
    const bool changed=impl_->dirty;if(changed){out=impl_->snapshot;impl_->dirty=false;}unlock(impl_->mutex);return changed;
}
struct NativeMountFix48::Impl {
    Thread worker;std::atomic<bool> cancelled{false},done{false},accepted{false};
    std::string error;GameEntry game;
    void run(){
        std::string why;
        if(!OrbitService48::bootstrap(why)){error=std::move(why);done.store(true,std::memory_order_release);return;}
        Bytes payload;unsigned command=ORBIT48_MOUNT_NATIVE;
        if(game.storage==GameStorageFix48::NTFS){
            auto map=std::make_unique<Orbit48RawMap>();
            if(!OrbitNtfsCatalog48::prepare(game,*map,why)){error=std::move(why);done.store(true,std::memory_order_release);return;}
            payload=OrbitService48::raw_payload(*map);command=ORBIT48_MOUNT_NTFS;
        }else payload=OrbitService48::native_payload(game,why);
        if(payload.empty()){error=why.empty()?"Não foi possível preparar a montagem.":why;done.store(true,std::memory_order_release);return;}
        if(cancelled.load(std::memory_order_acquire)){error="Montagem cancelada.";done.store(true,std::memory_order_release);return;}
        std::uint32_t code=0;Bytes response;
        const bool ok=OrbitService48::exchange(true,command,payload,clock_us(),code,response,
            [&]{return cancelled.load(std::memory_order_acquire);});
        if(!ok || code)error=OrbitService48::error_message(code?code:std::uint32_t(ORBIT48_BUSY));
        else accepted.store(true,std::memory_order_release);
        done.store(true,std::memory_order_release);
    }
};
NativeMountFix48::NativeMountFix48():impl_(new Impl){}
NativeMountFix48::~NativeMountFix48(){cancel();impl_->worker.join();}
bool NativeMountFix48::start(const GameEntry& game,std::uint64_t now){
    (void)now;if(state_==State::Waiting)return false;impl_->worker.join();
    impl_->game=game;impl_->cancelled.store(false);impl_->done.store(false);impl_->accepted.store(false);impl_->error.clear();error_.clear();
    if(!impl_->worker.start([this]{impl_->run();},"Orbit mount",512*1024)){error_="Não foi possível iniciar a montagem.";state_=State::Failed;return false;}
    state_=State::Waiting;return true;
}
void NativeMountFix48::update(std::uint64_t now){
    (void)now;if(state_!=State::Waiting || !impl_->done.load(std::memory_order_acquire))return;
    error_=impl_->error;state_=impl_->accepted.load(std::memory_order_acquire)?State::Accepted:State::Failed;
}
void NativeMountFix48::cancel(){impl_->cancelled.store(true,std::memory_order_release);}
#endif
