#include "ntfs_catalog_fix48.h"
#ifdef PS3_GAME_ORBIT_FIX48
#include "iso_metadata_fix48.h"
#include "metadata_utils.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <deque>
#include <unordered_set>
#ifdef __PSL1GHT__
#include "../plugin/native_calls.h"
#include "runtime_diag.h"
#endif
namespace OrbitNtfsCatalog48 {
namespace {
struct Item {std::string relative;std::uint64_t reference=0;};
bool items(OrbitNtfs48::Volume& v,std::vector<Item>& out){
    std::uint64_t root=0;if(!v.find("/PS3ISO",root))return false;
    struct Pending {std::string path;std::uint64_t ref;unsigned depth;};
    std::deque<Pending> queue{{"/PS3ISO",root,0}};std::unordered_set<std::uint64_t> seen;
    while(!queue.empty()){
        const auto current=queue.front();queue.pop_front();if(!seen.insert(current.ref).second)continue;
        if(seen.size()>512)return false;
        std::vector<OrbitNtfs48::Entry> entries;if(!v.list(current.ref,entries))return false;
        for(const auto& e:entries){
            if(e.directory){if(current.depth<4)queue.push_back({current.path+"/"+e.name,e.reference,current.depth+1});}
            else if(OrbitIso48::iso_name(e.name)){
                if(out.size()>=4096)return false;
                out.push_back({current.path+"/"+e.name,e.reference});
            }
        }
    }return true;
}
bool split_streams(OrbitNtfs48::Volume& v,const std::string& path,std::uint64_t ref,
    std::vector<OrbitNtfs48::Stream>& streams,std::string& error){
    OrbitNtfs48::Stream s;if(!v.file(ref,s)){error=v.error();return false;}
    if(!s.size || s.size%512 || !s.nonresident || s.initialized!=s.size){error="NTFS: ISO incompleta ou não suportada.";return false;}
    streams.push_back(std::move(s));
    if(path.size()<6 || !OrbitNtfs48::equal_ascii_ci(path.substr(path.size()-6),".iso.0"))return true;
    const auto prefix=path.substr(0,path.size()-1);
    for(unsigned i=1;i<ORBIT48_MAX_PARTS;++i){
        std::uint64_t next=0;if(!v.find(prefix+std::to_string(i),next))break;
        OrbitNtfs48::Stream part;if(!v.file(next,part) || !part.size || part.size%512 ||
           !part.nonresident || part.initialized!=part.size){error="NTFS: parte da ISO incompleta.";return false;}
        streams.push_back(std::move(part));
    }
    if(streams.size()==ORBIT48_MAX_PARTS){
        std::uint64_t next=0;if(v.find(prefix+std::to_string(ORBIT48_MAX_PARTS),next)){error="NTFS: ISO excede o limite de partes.";return false;}
    }
    const auto slash=path.find_last_of('/');
    const auto directory=slash==std::string::npos?"/":path.substr(0,slash);
    const auto name=prefix.substr(slash==std::string::npos?0:slash+1);
    std::uint64_t parent=0;std::vector<OrbitNtfs48::Entry> entries;
    if(!v.find(directory,parent) || !v.list(parent,entries)){error="NTFS: não foi possível conferir as partes.";return false;}
    for(const auto& item:entries){
        if(item.name.size()<=name.size() || !OrbitNtfs48::equal_ascii_ci(item.name.substr(0,name.size()),name))continue;
        const auto number=item.name.substr(name.size());
        if(number.size()>3 || number.find_first_not_of("0123456789")!=std::string::npos)continue;
        unsigned index=0;for(const auto c:number)index=index*10+unsigned(c-'0');
        if(index>=streams.size()){error="NTFS: falta uma parte da ISO.";return false;}
    }
    return true;
}
#ifdef __PSL1GHT__
struct Device {
    Orbit48DeviceInfo info{};std::uint32_t handle=0xffffffffu;std::uint64_t id=0;
    unsigned char scratch[65536] __attribute__((aligned(128)));
    explicit Device(std::uint64_t device):id(device){
        {lv2syscall1(8,0x8097);}
        if(orbit48_storage_info(id,&info) || info.sector_size<512 || info.sector_size>4096 ||
           (info.sector_size&(info.sector_size-1)) || !info.sectors)info.sectors=0;
        else if(orbit48_storage_open(id,&handle))info.sectors=0;
    }
    ~Device(){if(handle!=0xffffffffu)orbit48_storage_close(handle);}
    bool read(std::uint64_t lba,std::uint32_t count,void* out){
        if(!info.sectors || lba>=info.sectors || count>info.sectors-lba)return false;
        auto* p=static_cast<unsigned char*>(out);
        while(count){
            const auto n=std::min<std::uint32_t>(count,sizeof(scratch)/info.sector_size);
            if(!orbit48_storage_read(handle,lba,n,scratch))return false;
            std::memcpy(p,scratch,std::size_t(n)*info.sector_size);
            p+=std::size_t(n)*info.sector_size;lba+=n;count-=n;
        }return true;
    }
};
std::uint64_t device_id(unsigned i){return i<6?0x10300000000000aull+i:0x10300000000001full+(i-6);}
#endif
}
void scan_volume(OrbitNtfs48::Volume& v,unsigned device,unsigned partition,
    std::uint64_t id,std::vector<GameEntry>& games){
    std::vector<Item> entries;if(!items(v,entries))return;
    char prefix[64];std::snprintf(prefix,sizeof(prefix),"/dev_ntfs%03u_%02u",device,partition);
    for(const auto& entry:entries){
        std::vector<OrbitNtfs48::Stream> streams;std::string error;
        if(!split_streams(v,entry.relative,entry.reference,streams,error))continue;
        std::uint64_t size=0;bool valid=true;
        for(const auto& s:streams){if(s.size>UINT64_MAX-size){valid=false;break;}size+=s.size;}
        if(!valid || size%2048)continue;
        auto read=[&](std::uint64_t offset,void* out,std::size_t n){
            if(offset>size || n>size-offset)return false;
            auto* p=static_cast<unsigned char*>(out);
            for(const auto& stream:streams){
                if(offset>=stream.size){offset-=stream.size;continue;}
                const auto amount=std::size_t(std::min<std::uint64_t>(n,stream.size-offset));
                if(!v.read(stream,offset,p,amount))return false;
                p+=amount;n-=amount;offset=0;if(!n)return true;
            }return n==0;
        };
        const auto metadata=OrbitIso48::metadata(read,size);
        GameEntry game;game.path=prefix+entry.relative;game.title=metadata.title.empty()?OrbitIso48::stem(entry.relative):metadata.title;
        game.title_id=metadata.id.empty()?extract_title_id_from_text(entry.relative):metadata.id;
        game.source=GameSource::USB;game.format=GameFormat::ISO;game.storage=GameStorageFix48::NTFS;
        game.ntfs_device=id;game.ntfs_partition=v.partition_lba();game.ntfs_serial=v.serial();
        games.push_back(std::move(game));
    }
}
void scan(std::vector<GameEntry>& games){
#ifdef __PSL1GHT__
    if(!orbit48_cobra())return;
    for(unsigned i=0;i<8;++i){
        Device device(device_id(i));if(!device.info.sectors)continue;
        auto read=[&](std::uint64_t lba,std::uint32_t n,void* out){return device.read(lba,n,out);};
        const auto parts=OrbitNtfs48::partitions(read,device.info.sector_size,device.info.sectors);
        for(std::size_t p=0;p<parts.size() && p<128;++p){
            OrbitNtfs48::Volume volume(read,device.info.sector_size,device.info.sectors,parts[p]);
            if(volume.open())scan_volume(volume,i,unsigned(p),device.id,games);
        }
    }
    RuntimeDiag::log("NTFS 1.6: read-only scan completed; catalog=%u",unsigned(games.size()));
#else
    (void)games;
#endif
}
bool prepare(const GameEntry& game,Orbit48RawMap& map,std::string& error){
    map={};
#ifdef __PSL1GHT__
    if(game.storage!=GameStorageFix48::NTFS || !game.ntfs_device){error="Entrada NTFS inválida.";return false;}
    Device device(game.ntfs_device);if(!device.info.sectors){error="USB NTFS indisponível.";return false;}
    auto read=[&](std::uint64_t lba,std::uint32_t n,void* out){return device.read(lba,n,out);};
    const auto parts=OrbitNtfs48::partitions(read,device.info.sector_size,device.info.sectors);
    const auto part=std::find_if(parts.begin(),parts.end(),[&](const OrbitNtfs48::Partition& p){return p.start==game.ntfs_partition;});
    if(part==parts.end()){error="A partição NTFS foi trocada.";return false;}
    OrbitNtfs48::Volume volume(read,device.info.sector_size,device.info.sectors,*part);
    if(!volume.open() || volume.serial()!=game.ntfs_serial){error="O volume NTFS foi trocado.";return false;}
    const auto slash=game.path.find('/',1);if(slash==std::string::npos){error="Caminho NTFS inválido.";return false;}
    const auto relative=game.path.substr(slash);std::uint64_t ref=0;
    if(!volume.find(relative,ref)){error=volume.error();return false;}
    std::vector<OrbitNtfs48::Stream> streams;if(!split_streams(volume,relative,ref,streams,error))return false;
    map.device_id=device.id;
    for(const auto& s:streams)if(!volume.append_map(s,map.iso_size,map)){error=volume.error();return false;}
    return map.iso_size%2048==0;
#else
    (void)game;error="Montagem NTFS disponível no PS3.";return false;
#endif
}
}
#endif
