#include "game_tools_fix35.h"
#ifdef PS3_GAME_ORBIT_FIX35
#include "cover_resolver.h"
#include "cover_image.h"
#include "image_decode.h"
#include <algorithm>
#include <array>
#include <cstdio>
#include <cerrno>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
namespace GameToolsFix35 {
namespace {
std::string hex(const std::string& text){const char* digits="0123456789abcdef";std::string out;for(unsigned char c:text){out+=digits[c>>4];out+=digits[c&15];}return out;}
bool unhex(const std::string& text,std::string& out){
    if(text.size()>2048 || text.size()%2)return false;
    out.clear();
    const auto digit=[](char c){return c>='0' && c<='9' ? c-'0' : c>='a' && c<='f' ? c-'a'+10 : -1;};
    for(std::size_t i=0;i<text.size();i+=2){const int a=digit(text[i]),b=digit(text[i+1]);if(a<0 || b<0)return false;out+=char(a*16+b);}return true;
}
#ifndef PS3_GAME_ORBIT_FIX36
bool exists(const std::string& p){struct stat st{};return stat(p.c_str(),&st)==0;}
#endif
bool game_path(const std::string& p){return p.rfind("/dev_",0)==0 && p.size()<=1024 && p.find('\n')==std::string::npos && p.find('\0')==std::string::npos;}
#ifndef PS3_GAME_ORBIT_FIX36
bool title_id(const std::string& id){if(id.size()!=9)return false;for(unsigned i=0;i<9;++i)if(i<4 ? id[i]<'A' || id[i]>'Z' : id[i]<'0' || id[i]>'9')return false;return true;}
#endif
std::uint32_t digest(const std::string& s){std::uint32_t h=2166136261u;for(unsigned char c:s)h=(h^c)*16777619u;return h;}
bool write_file(const std::string& path,const unsigned char* bytes,std::size_t count){
    FILE* f=std::fopen(path.c_str(),"wb");if(!f)return false;
    bool ok=std::fwrite(bytes,1,count,f)==count && std::fflush(f)==0;
    if(ok)ok=fsync(fileno(f))==0;
    if(std::fclose(f)!=0)ok=false;
    return ok;
}
}
std::vector<std::uint16_t> utf16(const std::string& text){
    std::vector<std::uint16_t> out;
    for(std::size_t i=0;i<text.size();){
        const auto a=static_cast<unsigned char>(text[i++]);unsigned cp=a,n=0,min=0;
        if(a>=0xc2 && a<=0xdf){cp=a&31;n=1;min=128;}else if(a>=0xe0 && a<=0xef){cp=a&15;n=2;min=2048;}else if(a>=0xf0 && a<=0xf4){cp=a&7;n=3;min=65536;}else if(a>=128)return {};
        if(i+n>text.size())return {};
        for(unsigned j=0;j<n;++j){const auto b=static_cast<unsigned char>(text[i++]);if((b&0xc0)!=0x80)return {};cp=(cp<<6)|(b&63);}
        if(cp<min || cp>0x10ffff || (cp>=0xd800 && cp<=0xdfff) || cp<32 || cp==127)return {};
        if(cp<65536)out.push_back(std::uint16_t(cp));else{cp-=65536;out.push_back(std::uint16_t(0xd800+(cp>>10)));out.push_back(std::uint16_t(0xdc00+(cp&1023)));}
    }
    return out;
}
std::string utf8(const std::uint16_t* text,std::size_t count){
    std::string out;
    for(std::size_t i=0;i<count && text[i];++i){unsigned cp=text[i];
        if(cp>=0xd800 && cp<=0xdbff){if(i+1>=count || text[i+1]<0xdc00 || text[i+1]>0xdfff)return {};cp=0x10000+((cp-0xd800)<<10)+(text[++i]-0xdc00);}else if(cp>=0xdc00 && cp<=0xdfff)return {};
        if(cp<128)out+=char(cp);else if(cp<2048){out+=char(0xc0|(cp>>6));out+=char(0x80|(cp&63));}else if(cp<65536){out+=char(0xe0|(cp>>12));out+=char(0x80|((cp>>6)&63));out+=char(0x80|(cp&63));}else{out+=char(0xf0|(cp>>18));out+=char(0x80|((cp>>12)&63));out+=char(0x80|((cp>>6)&63));out+=char(0x80|(cp&63));}
    }return out;
}
bool valid_title(const std::string& title){const auto chars=utf16(title);return !chars.empty() && chars.size()<=96 && title.size()<=384 && title.find_first_not_of(' ')!=std::string::npos;}
bool Names::set(const std::string& path,const std::string& title){if(!game_path(path) || !valid_title(title) || (!names_.count(path) && names_.size()>=4096))return false;names_[path]=title;return true;}
void Names::apply(std::vector<GameEntry>& games)const{for(auto& game:games){const auto it=names_.find(game.path);if(it!=names_.end())game.title=it->second;}}
bool Names::save(const std::string& path)const{
    std::vector<std::string> keys;for(const auto& n:names_)keys.push_back(n.first);std::sort(keys.begin(),keys.end());
    std::string body;for(const auto& key:keys)body+=hex(key)+" "+hex(names_.at(key))+"\n";
    const auto data="ORBIT_NAMES35_V1 "+std::to_string(digest(body))+"\n"+body;
    if(data.size()>2u*1024*1024)return false;
    const auto temp=path+".tmp";
    bool ok=write_file(temp,reinterpret_cast<const unsigned char*>(data.data()),data.size());
    if(ok)ok=std::rename(temp.c_str(),path.c_str())==0;
    if(!ok)std::remove(temp.c_str());
    return ok;
}
bool Names::load(const std::string& path){
    FILE* f=std::fopen(path.c_str(),"rb");if(!f)return false;std::string data;std::array<char,4096> block{};
    while(const auto n=std::fread(block.data(),1,block.size(),f)){data.append(block.data(),n);if(data.size()>2u*1024*1024){std::fclose(f);return false;}}
    const bool ok=!std::ferror(f);std::fclose(f);if(!ok)return false;
    const auto newline=data.find('\n');if(newline==std::string::npos)return false;
    std::istringstream header(data.substr(0,newline));std::string magic,extra;std::uint32_t sum;
    if(!(header>>magic>>sum) || magic!="ORBIT_NAMES35_V1" || (header>>extra))return false;
    const auto body=data.substr(newline+1);if(digest(body)!=sum)return false;
    Names fresh;std::istringstream rows(body);std::string row;
    while(std::getline(rows,row)){std::istringstream fields(row);std::string key,value,path_out,title;
        if(!(fields>>key>>value) || (fields>>extra) || !unhex(key,path_out) || !unhex(value,title) || fresh.names_.count(path_out) || !fresh.set(path_out,title))return false;
    }names_=std::move(fresh.names_);return true;
}
void resolve_art(GameEntry& game,const std::string& covers){CoverResolver resolver(covers);const auto result=resolver.resolve(game);game.cover_path=result.path;game.cover_kind=result.path.empty() ? GameCoverKind::None : result.is_full_cover ? GameCoverKind::FullCover : GameCoverKind::FrontOnly;resolver.resolve_inspection_art(game);}
#ifndef PS3_GAME_ORBIT_FIX36
ImportResult import_usb(const GameEntry& game,const std::vector<std::string>& roots,const std::string& covers){
    if(!title_id(game.title_id))return {false,"ID do jogo não disponível para importar."};
    const std::array<std::string,3> names{{game.title_id+".jpg",game.title_id+"_INSIDE.jpg",game.title_id+"_DISC.png"}};
    std::string source;
    for(const auto& root:roots)if(exists(root+"/"+names[0]) && exists(root+"/"+names[1]) && exists(root+"/"+names[2])){source=root;break;}
    if(source.empty())return {false,"As três imagens não foram encontradas no pendrive."};
    if(mkdir(covers.c_str(),0777)!=0 && errno!=EEXIST)return {false,"Não foi possível criar a pasta de capas."};
    std::array<std::string,3> staged;
    const auto remove_staged=[&](){for(const auto& path:staged)if(!path.empty())std::remove(path.c_str());};
    for(unsigned i=0;i<names.size();++i){
        auto image=load_cover_file(source+"/"+names[i],i==2 ? GameCoverKind::FrontOnly : GameCoverKind::FullCover);
        if(!image.valid() || image.width>8192 || image.height>8192 || std::size_t(image.width)*image.height>16000000){remove_staged();return {false,"Imagem ausente, inválida ou grande demais: "+names[i]};}
        {DecodedImageRGBA decoded;std::string error;if(!decode_cover_rgba(image,decoded,error)){remove_staged();return {false,"Imagem corrompida: "+names[i]};}}
        staged[i]=covers+"/"+names[i]+".orbit-new";
        if(exists(staged[i]) || !write_file(staged[i],image.encoded.data(),image.encoded.size())){remove_staged();return {false,"Falha ao copiar as imagens."};}
    }
    std::vector<std::pair<std::string,std::string>> backups;std::vector<std::string> committed;
    const auto rollback=[&](){for(const auto& path:committed)std::remove(path.c_str());for(auto it=backups.rbegin();it!=backups.rend();++it)std::rename(it->second.c_str(),it->first.c_str());remove_staged();};
    for(const auto& suffix:{"","_INSIDE","_DISC"})for(const auto& ext:{".jpg",".jpeg",".png",".JPG",".JPEG",".PNG"}){
        const auto path=covers+"/"+game.title_id+suffix+ext;if(!exists(path))continue;
        const auto backup=path+".orbit-old";
        if(exists(backup) || std::rename(path.c_str(),backup.c_str())!=0){rollback();return {false,"Não foi possível substituir as capas anteriores."};}
        backups.emplace_back(path,backup);
    }
    for(unsigned i=0;i<names.size();++i){const auto path=covers+"/"+names[i];if(std::rename(staged[i].c_str(),path.c_str())!=0){rollback();return {false,"Falha ao concluir a atualização das capas."};}committed.push_back(path);}
    for(const auto& backup:backups)std::remove(backup.second.c_str());
    return {true,"Três imagens copiadas. Capas atualizadas."};
}
#endif
}
#endif
