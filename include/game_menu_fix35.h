#pragma once
#include <string>

enum class GameMenuActionFix35 { None, Rename, ReloadCovers, ImportUsb };
struct GameMenuStateFix35 {
    bool open=false,busy=false;
#ifdef PS3_GAME_ORBIT_FIX36
    bool animated_background=true;
#endif
    int selected=0;
    std::string status;
    bool operator==(const GameMenuStateFix35& o) const {
        return open==o.open && busy==o.busy && selected==o.selected && status==o.status
#ifdef PS3_GAME_ORBIT_FIX36
            && animated_background==o.animated_background
#endif
        ;
    }
    bool operator!=(const GameMenuStateFix35& o) const {return !(*this==o);}
};
