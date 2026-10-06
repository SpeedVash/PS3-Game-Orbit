#pragma once
#ifdef PS3_GAME_ORBIT_FIX37
#include "coverflow_state.h"
#include <string>

class OrbitSettingsFix37 {
public:
    OrbitLayout layout=OrbitLayout::Classic;
    bool animated_background=true,remember_last_game=true,automatic_rotation=false;
    float zoom=.55f,yaw=28,pitch=-5;
    bool load(const std::string& path);
    bool save(const std::string& path) const;
    void capture(const CoverflowState& state,float scale,bool automatic);
};
#endif
