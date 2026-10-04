#include "layout_settings_fix31.h"
#include <cstdio>
#include <cstring>
#include <unistd.h>

bool LayoutSettingsFix31::load(const std::string& path) {
    auto* f=std::fopen(path.c_str(),"rb");if(!f) return false;
    char bytes[64]{};const auto count=std::fread(bytes,1,sizeof(bytes),f);
    const bool valid_read=!std::ferror(f);std::fclose(f);
    if(!valid_read || count==sizeof(bytes)) return false;
    const std::string data(bytes,count);
    if(data=="PS3_GAME_ORBIT_LAYOUT_V1\nclassic\n") layout_=OrbitLayout::Classic;
    else if(data=="PS3_GAME_ORBIT_LAYOUT_V1\nspine\n") layout_=OrbitLayout::Spine;
    else return false;
    return true;
}
bool LayoutSettingsFix31::save(const std::string& path) const {
    const char* value=layout_==OrbitLayout::Spine ? "PS3_GAME_ORBIT_LAYOUT_V1\nspine\n" : "PS3_GAME_ORBIT_LAYOUT_V1\nclassic\n";
    const auto temp=path+".tmp";
    auto* f=std::fopen(temp.c_str(),"wb");if(!f) return false;
    bool ok=std::fwrite(value,1,std::strlen(value),f)==std::strlen(value);
    if(std::fflush(f)!=0 || fsync(fileno(f))!=0) ok=false;
    if(std::fclose(f)!=0) ok=false;
    if(ok) ok=std::rename(temp.c_str(),path.c_str())==0;
    if(!ok) std::remove(temp.c_str());
    return ok;
}
