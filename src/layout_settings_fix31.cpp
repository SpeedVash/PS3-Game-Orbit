#include "file_store_fix38.h"
#include "layout_settings_fix31.h"
#include <cstdio>
#include <cstring>
#include <unistd.h>

bool LayoutSettingsFix31::load(const std::string& path) {
    auto* f=
#ifdef PS3_GAME_ORBIT_FIX38
FileStoreFix38::open_read(path)
#else
std::fopen(path.c_str(),"rb")
#endif
;if(!f) return false;
    char bytes[64]{};const auto count=std::fread(bytes,1,sizeof(bytes),f);
    const bool valid_read=!std::ferror(f);std::fclose(f);
    if(!valid_read || count==sizeof(bytes)) return false;
    const std::string data(bytes,count);
    if(data=="PS3_GAME_ORBIT_LAYOUT_V1\nclassic\n") layout_=OrbitLayout::Classic;
    else if(data=="PS3_GAME_ORBIT_LAYOUT_V1\nspine\n") layout_=OrbitLayout::Spine;
#ifdef PS3_GAME_ORBIT_FIX32
    else if(data=="PS3_GAME_ORBIT_LAYOUT_V1\nlist\n") layout_=OrbitLayout::List;
#endif
    else return false;
    return true;
}
bool LayoutSettingsFix31::save(const std::string& path) const {
    const char* value=layout_==OrbitLayout::Spine ? "PS3_GAME_ORBIT_LAYOUT_V1\nspine\n" : "PS3_GAME_ORBIT_LAYOUT_V1\nclassic\n";
#ifdef PS3_GAME_ORBIT_FIX32
    if(layout_==OrbitLayout::List) value="PS3_GAME_ORBIT_LAYOUT_V1\nlist\n";
#endif
    const auto temp=path+".tmp";
    auto* f=std::fopen(temp.c_str(),"wb");if(!f) return false;
    bool ok=std::fwrite(value,1,std::strlen(value),f)==std::strlen(value);
    if(std::fflush(f)!=0 || fsync(fileno(f))!=0) ok=false;
    if(std::fclose(f)!=0) ok=false;
    if(ok) ok=
#ifdef PS3_GAME_ORBIT_FIX38
FileStoreFix38::commit(temp,path)
#else
std::rename(temp.c_str(),path.c_str())==0
#endif
;
    if(!ok) std::remove(temp.c_str());
    return ok;
}
