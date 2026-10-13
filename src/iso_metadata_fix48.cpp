#include "iso_metadata_fix48.h"
#ifdef PS3_GAME_ORBIT_FIX48
#include "ntfs_reader_fix48.h"
#include "orbit_service_protocol_fix48.h"
#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <limits>
#include <dirent.h>
namespace OrbitIso48 {
namespace {
std::uint16_t le16(const unsigned char* p){return std::uint16_t(p[0]|(unsigned(p[1])<<8));}
std::uint32_t le32(const unsigned char* p){return std::uint32_t(le16(p))|(std::uint32_t(le16(p+2))<<16);}
bool suffix(const std::string& name,const std::string& value){
    return name.size()>=value.size() && OrbitNtfs48::equal_ascii_ci(name.substr(name.size()-value.size()),value);
}
struct File {std::uint64_t offset=0,size=0;bool directory=false;};
bool entry(Reader read,std::uint64_t size,const File& directory,const std::string& name,File& out){
    if(!directory.directory || directory.size>4*1024*1024 || directory.offset>size || directory.size>size-directory.offset)return false;
    std::vector<unsigned char> bytes(std::size_t(directory.size));
    if(!read(directory.offset,bytes.data(),bytes.size()))return false;
    for(std::size_t at=0;at<bytes.size();){
        const auto n=bytes[at];if(!n){at=(at/2048+1)*2048;continue;}
        if(n<34 || n>bytes.size()-at)return false;
        const auto* p=bytes.data()+at;const auto nl=p[32];if(nl>n-33)return false;
        std::string candidate(reinterpret_cast<const char*>(p+33),nl);
        const auto version=candidate.find(';');if(version!=std::string::npos)candidate.resize(version);
        if(!candidate.empty() && candidate.back()=='.')candidate.pop_back();
        if(OrbitNtfs48::equal_ascii_ci(candidate,name)){
            if(p[25]&0x80)return false;
            out={std::uint64_t(le32(p+2))*2048,le32(p+10),(p[25]&2)!=0};
            return out.offset<=size && out.size<=size-out.offset;
        }at+=n;
    }return false;
}
}
bool iso_name(const std::string& name){return suffix(name,".iso") || suffix(name,".iso.0");}
std::string stem(std::string name){
    const auto slash=name.find_last_of('/');if(slash!=std::string::npos)name.erase(0,slash+1);
    if(suffix(name,".iso.0"))name.resize(name.size()-6);
    else if(suffix(name,".iso"))name.resize(name.size()-4);
    return name;
}
Metadata sfo(const std::vector<unsigned char>& b){
    Metadata out;
    if(b.size()<20 || b.size()>128*1024 || std::memcmp(b.data(),"\0PSF",4))return out;
    const auto key=le32(b.data()+8),data=le32(b.data()+12),count=le32(b.data()+16);
    if(count>4096 || key<20 || key>b.size() || data<key || data>b.size() || count>(key-20)/16)return out;
    for(unsigned i=0;i<count;++i){
        const auto* p=b.data()+20+i*16;const auto off=le16(p),format=le16(p+2);
        const auto length=le32(p+4),maximum=le32(p+8),value=le32(p+12);
        if(off>=data-key || length>maximum || value>b.size()-data || length>b.size()-data-value)return {};
        const auto* first=b.data()+key+off;const auto* end=static_cast<const unsigned char*>(std::memchr(first,0,data-key-off));
        if(!end)return {};
        const std::string name(reinterpret_cast<const char*>(first),std::size_t(end-first));
        if(format!=0x0204 || !length)continue;
        const auto* raw=b.data()+data+value;const auto* finish=static_cast<const unsigned char*>(std::memchr(raw,0,length));
        if(!finish)return {};
        std::string text(reinterpret_cast<const char*>(raw),std::size_t(finish-raw));
        if(name=="TITLE")out.title=text.size()<=512?text:std::string{};
        else if(name=="TITLE_ID"){
            if(text.size()!=9)continue;
            bool valid=true;
            for(std::size_t k=0;k<9;++k){auto& c=text[k];if(c>='a' && c<='z')c=char(c-32);
                if(k<4 ? c<'A'||c>'Z' : c<'0'||c>'9')valid=false;}
            if(valid)out.id=std::move(text);
        }else if(name=="CATEGORY" && text!="DG")return {};
    }return out;
}
Metadata metadata(Reader read,std::uint64_t size){
    std::array<unsigned char,2048> pvd{};
    if(size<18*2048 || !read(16*2048,pvd.data(),pvd.size()) || pvd[0]!=1 ||
       std::memcmp(pvd.data()+1,"CD001",5) || pvd[6]!=1 || le16(pvd.data()+128)!=2048)return {};
    const auto* r=pvd.data()+156;if(r[0]<34 || !(r[25]&2))return {};
    const File root{std::uint64_t(le32(r+2))*2048,le32(r+10),true};
    File game,param;if(!entry(read,size,root,"PS3_GAME",game) || !entry(read,size,game,"PARAM.SFO",param) ||
       param.directory || param.size<20 || param.size>128*1024)return {};
    std::vector<unsigned char> bytes(std::size_t(param.size));if(!read(param.offset,bytes.data(),bytes.size()))return {};
    return sfo(bytes);
}
bool native_parts(const std::string& first,std::vector<std::string>& files,std::string& error){
    files.clear();struct stat info{};
    if(!iso_name(first) || stat(first.c_str(),&info) || !S_ISREG(info.st_mode) || info.st_size<=0 || info.st_size%512){
        error="ISO indisponível ou tamanho inválido.";return false;
    }
    files.push_back(first);
    if(suffix(first,".iso.0")){
        const auto prefix=first.substr(0,first.size()-1);
        for(unsigned i=1;i<ORBIT48_MAX_PARTS;++i){
            const auto path=prefix+std::to_string(i);
            if(stat(path.c_str(),&info))break;
            if(!S_ISREG(info.st_mode) || info.st_size<=0 || info.st_size%512){error="Uma parte da ISO está incompleta.";files.clear();return false;}
            files.push_back(path);
        }
        if(files.size()==ORBIT48_MAX_PARTS && !stat((prefix+std::to_string(ORBIT48_MAX_PARTS)).c_str(),&info)){
            error="ISO excede o limite de partes.";files.clear();return false;
        }
        const auto slash=prefix.find_last_of('/');
        const auto directory=slash==std::string::npos?".":prefix.substr(0,slash);
        const auto name=prefix.substr(slash==std::string::npos?0:slash+1);
        DIR* dir=opendir(directory.c_str());
        if(!dir){error="Não foi possível conferir as partes da ISO.";files.clear();return false;}
        bool gap=false;
        while(const auto* item=readdir(dir)){
            const std::string candidate=item->d_name;
            if(candidate.size()<=name.size() || candidate.compare(0,name.size(),name))continue;
            const auto number=candidate.substr(name.size());
            if(number.size()>3 || number.find_first_not_of("0123456789")!=std::string::npos)continue;
            unsigned index=0;for(const auto c:number)index=index*10+unsigned(c-'0');
            if(index>=files.size())gap=true;
        }
        closedir(dir);
        if(gap){error="Falta uma parte da ISO. Confira a sequência .0, .1, .2...";files.clear();return false;}
    }
    std::uint64_t total=0;
    for(const auto& file:files){
        if(stat(file.c_str(),&info) || info.st_size<=0 || std::uint64_t(info.st_size)>UINT64_MAX-total){
            error="Uma parte da ISO mudou durante a verificação.";files.clear();return false;
        }
        total+=std::uint64_t(info.st_size);
    }
    if(total%2048){error="O conjunto da ISO está incompleto.";files.clear();return false;}
    return true;
}
Metadata native_metadata(const std::vector<std::string>& files){
    std::vector<std::uint64_t> sizes;std::uint64_t total=0;
    for(const auto& path:files){
        struct stat info{};if(stat(path.c_str(),&info) || info.st_size<0 || std::uint64_t(info.st_size)>UINT64_MAX-total)return {};
        sizes.push_back(std::uint64_t(info.st_size));total+=sizes.back();
    }
    auto read=[&](std::uint64_t offset,void* output,std::size_t n){
        if(offset>total || n>total-offset)return false;
        auto* p=static_cast<unsigned char*>(output);
        for(std::size_t i=0;i<files.size() && n;++i){
            if(offset>=sizes[i]){offset-=sizes[i];continue;}
            FILE* f=std::fopen(files[i].c_str(),"rb");if(!f)return false;
            const auto amount=std::size_t(std::min<std::uint64_t>(n,sizes[i]-offset));
            const bool ok=offset<=std::uint64_t(std::numeric_limits<off_t>::max()) &&
                ::fseeko(f,static_cast<off_t>(offset),SEEK_SET)==0 && std::fread(p,1,amount,f)==amount;
            std::fclose(f);if(!ok)return false;p+=amount;n-=amount;offset=0;
        }return n==0;
    };return metadata(read,total);
}
}
#endif
