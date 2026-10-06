#include "file_store_fix38.h"
#include "orbit_settings_fix37.h"
#ifdef PS3_GAME_ORBIT_FIX37
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <unistd.h>

namespace {
std::uint32_t checksum(const std::string& s){std::uint32_t h=2166136261u;for(unsigned char c:s)h=(h^c)*16777619u;return h;}
}
bool OrbitSettingsFix37::load(const std::string& path){
    FILE* f=
#ifdef PS3_GAME_ORBIT_FIX38
FileStoreFix38::open_read(path)
#else
std::fopen(path.c_str(),"rb")
#endif
;if(!f)return false;
    char buffer[4097];const auto size=std::fread(buffer,1,sizeof(buffer),f);
    const bool ok=!std::ferror(f) && size<sizeof(buffer);std::fclose(f);if(!ok)return false;
    const std::string data(buffer,size);const auto nl=data.find('\n');if(nl==std::string::npos)return false;
    std::istringstream header(data.substr(0,nl));std::string magic,extra;std::uint32_t hash=0;
    if(!(header>>magic>>hash) || magic!="PS3_GAME_ORBIT_SETTINGS_14" || (header>>extra) || checksum(data.substr(nl+1))!=hash)return false;
    std::istringstream row(data.substr(nl+1));unsigned layout_value=0,animated=0,remember=0,automatic=0;
    OrbitSettingsFix37 candidate;
    if(!(row>>layout_value>>animated>>remember>>automatic>>candidate.zoom>>candidate.yaw>>candidate.pitch) || (row>>extra))return false;
    if(layout_value>2 || animated>1 || remember>1 || automatic>1 || !std::isfinite(candidate.zoom) ||
       !std::isfinite(candidate.yaw) || !std::isfinite(candidate.pitch) || candidate.zoom<.55f || candidate.zoom>.80f ||
       candidate.yaw<0 || candidate.yaw>=360 || candidate.pitch<-28 || candidate.pitch>22)return false;
    candidate.layout=static_cast<OrbitLayout>(layout_value);candidate.animated_background=animated!=0;
    candidate.remember_last_game=remember!=0;candidate.automatic_rotation=automatic!=0;*this=candidate;return true;
}
bool OrbitSettingsFix37::save(const std::string& path) const {
    std::ostringstream row;row<<std::setprecision(9)<<unsigned(layout)<<' '<<unsigned(animated_background)<<' '
        <<unsigned(remember_last_game)<<' '<<unsigned(automatic_rotation)<<' '<<zoom<<' '<<yaw<<' '<<pitch<<'\n';
    const auto payload=row.str(),data="PS3_GAME_ORBIT_SETTINGS_14 "+std::to_string(checksum(payload))+"\n"+payload;
    const auto temp=path+".tmp";FILE* f=std::fopen(temp.c_str(),"wb");if(!f)return false;
    bool ok=std::fwrite(data.data(),1,data.size(),f)==data.size() && std::fflush(f)==0;
    if(ok)ok=fsync(fileno(f))==0;
    if(std::fclose(f)!=0)ok=false;
    if(ok)ok=
#ifdef PS3_GAME_ORBIT_FIX38
FileStoreFix38::commit(temp,path)
#else
std::rename(temp.c_str(),path.c_str())==0
#endif
;
    if(!ok)std::remove(temp.c_str());
    return ok;
}
void OrbitSettingsFix37::capture(const CoverflowState& state,float scale,bool automatic){
    layout=state.layout;animated_background=state.menu.animated_background;remember_last_game=state.menu.remember_last_game;
    zoom=std::clamp(scale,.55f,.80f);automatic_rotation=automatic;
    const bool inspecting=state.inspection_target || state.inspection_phase>0;
    yaw=std::fmod(inspecting?state.browse_yaw:state.center_yaw_deg,360.f);if(yaw<0)yaw+=360;
    pitch=std::clamp(inspecting?state.browse_pitch:state.center_pitch_deg,-28.f,22.f);
}
#endif
