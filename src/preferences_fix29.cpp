#include "file_store_fix38.h"
#include "preferences_fix29.h"
#include <algorithm>
#include <cstdio>
#include <cstdint>
#include <sstream>
#include <utility>
#include <vector>
#include <unistd.h>

namespace {
constexpr std::size_t MaxBytes=2*1024*1024,MaxRecords=4096;
std::uint32_t checksum(const std::string& s){std::uint32_t h=2166136261u;for(unsigned char c:s) h=(h^c)*16777619u;return h;}
std::string hex(const std::string& s){
    constexpr char digits[]="0123456789abcdef";std::string out;
    for(unsigned char c:s){out+=digits[c>>4];out+=digits[c&15];}return out;
}
bool unhex(const std::string& in,std::string& out){
    if(in.size()>2048 || in.size()%2) return false;
    out.clear();
    for(std::size_t i=0;i<in.size();i+=2){
        const auto digit=[](char c){return c>='0' && c<='9' ? c-'0' : c>='a' && c<='f' ? c-'a'+10 : -1;};
        const int a=digit(in[i]),b=digit(in[i+1]);if(a<0 || b<0) return false;
        const char c=char(a*16+b);if(static_cast<unsigned char>(c)<32) return false;out+=c;
    }
    return true;
}
bool valid_path(const std::string& s){return !s.empty() && s.size()<=1024 && s.rfind("/dev_",0)==0;}
}
bool PreferencesFix29::load(const std::string& path){
    FILE* file=
#ifdef PS3_GAME_ORBIT_FIX38
FileStoreFix38::open_read(path)
#else
std::fopen(path.c_str(),"rb")
#endif
;if(!file) return false;
    std::string data;char block[4096];
    while(const auto n=std::fread(block,1,sizeof(block),file)){
        data.append(block,n);if(data.size()>MaxBytes){std::fclose(file);return false;}
    }
    const bool read_ok=!std::ferror(file);std::fclose(file);if(!read_ok) return false;
    const auto newline=data.find('\n');if(newline==std::string::npos) return false;
    std::istringstream header(data.substr(0,newline));std::string magic,extra;std::uint32_t digest;
    if(!(header>>magic>>digest) || magic!="PS3_SP_PREFS29_V1" || (header>>extra)) return false;
    const auto payload=data.substr(newline+1);if(checksum(payload)!=digest) return false;
    std::istringstream lines(payload);std::string line;
    std::unordered_map<std::string,Record> records;FilterMode filter=FilterMode::All;std::string selected;
    bool filter_seen=false,selection_seen=false;
    while(std::getline(lines,line)){
        std::istringstream row(line);std::string tag,p,tail;unsigned a=0,b=0;
        if(!(row>>tag)) return false;
        if(tag=="filter"){
            if(filter_seen || !(row>>a) || a>3 || (row>>tail)) return false;
            filter_seen=true;filter=static_cast<FilterMode>(a);
        }else if(tag=="selected"){
            if(selection_seen || !(row>>p) || (row>>tail)) return false;
            selection_seen=true;
            if(p!="-" && (!unhex(p,selected) || !valid_path(selected))) return false;
        }else if(tag=="game"){
            std::string decoded;
            if(!(row>>a>>b>>p) || a>1 || b<1 || b>8 || (row>>tail) || !unhex(p,decoded) || !valid_path(decoded) || records.size()>=MaxRecords) return false;
            if(!records.emplace(decoded,Record{a!=0,b}).second) return false;
        }else return false;
    }
    if(!filter_seen || !selection_seen) return false;
    records_=std::move(records);filter_=filter;selected_=std::move(selected);return true;
}
bool PreferencesFix29::save(const std::string& path) const {
    std::vector<std::string> keys;for(const auto& entry:records_) keys.push_back(entry.first);std::sort(keys.begin(),keys.end());
    std::string payload="filter "+std::to_string(static_cast<unsigned>(filter_))+"\nselected "+(selected_.empty() ? "-" : hex(selected_))+"\n";
    for(const auto& key:keys){
        const auto& r=records_.at(key);payload+="game "+std::to_string(unsigned(r.favorite))+" "+std::to_string(r.orientation)+" "+hex(key)+"\n";
    }
    const std::string data="PS3_SP_PREFS29_V1 "+std::to_string(checksum(payload))+"\n"+payload;
    if(data.size()>MaxBytes || records_.size()>MaxRecords) return false;
    const auto temporary=path+".tmp";FILE* file=std::fopen(temporary.c_str(),"wb");if(!file) return false;
    bool ok=std::fwrite(data.data(),1,data.size(),file)==data.size() && std::fflush(file)==0;
    if(ok) ok=fsync(fileno(file))==0;
    if(std::fclose(file)!=0) ok=false;
    if(ok) ok=
#ifdef PS3_GAME_ORBIT_FIX38
FileStoreFix38::commit(temporary,path)
#else
std::rename(temporary.c_str(),path.c_str())==0
#endif
;
    if(!ok) std::remove(temporary.c_str());
    return ok;
}
void PreferencesFix29::apply(std::vector<GameEntry>& games) const {
    for(auto& g:games){const auto it=records_.find(g.path);if(it!=records_.end()){g.favorite=it->second.favorite;g.cover_orientation=it->second.orientation;}}
}
void PreferencesFix29::capture(const CoverflowState& state){
    filter_=state.filter;if(const auto* game=current_game(state)) selected_=game->path;
    for(const auto& g:state.games) if(valid_path(g.path)){
        if(!records_.count(g.path) && records_.size()>=MaxRecords) continue;
        records_[g.path]={g.favorite,g.cover_orientation};
    }
}
