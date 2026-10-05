#pragma once
#ifdef PS3_GAME_ORBIT_FIX36
#include <string>
class BackgroundSettingsFix36 {
public:
    bool load(const std::string& path);
    bool save(const std::string& path)const;
    bool animated()const{return animated_;}
    void capture(bool value){animated_=value;}
private:bool animated_=true;
};
#endif
