#include "background_settings_fix36.h"
#ifdef PS3_GAME_ORBIT_FIX36
#include <cstdio>
#include <unistd.h>
bool BackgroundSettingsFix36::load(const std::string& path){
    auto* f=std::fopen(path.c_str(),"rb");if(!f)return false;
    char bytes[64]{};const auto n=std::fread(bytes,1,sizeof(bytes),f);const bool ok=!std::ferror(f);std::fclose(f);
    if(!ok || n==sizeof(bytes))return false;
    const std::string data(bytes,n);
    if(data=="ORBIT_BACKGROUND36_V1\non\n")animated_=true;
    else if(data=="ORBIT_BACKGROUND36_V1\noff\n")animated_=false;
    else return false;
    return true;
}
bool BackgroundSettingsFix36::save(const std::string& path)const{
    const std::string data=animated_?"ORBIT_BACKGROUND36_V1\non\n":"ORBIT_BACKGROUND36_V1\noff\n";
    const auto temp=path+".tmp";auto* f=std::fopen(temp.c_str(),"wb");if(!f)return false;
    bool ok=std::fwrite(data.data(),1,data.size(),f)==data.size() && std::fflush(f)==0;
    if(ok)ok=fsync(fileno(f))==0;
    if(std::fclose(f)!=0)ok=false;
    if(ok)ok=std::rename(temp.c_str(),path.c_str())==0;
    if(!ok)std::remove(temp.c_str());
    return ok;
}
#endif
