#pragma once
#include <string>

enum class GameMenuActionFix35 { None, Rename, ReloadCovers, ImportUsb };
struct GameMenuStateFix35 {
    bool open=false,busy=false;
#ifdef PS3_GAME_ORBIT_FIX36
    bool animated_background=true;
#endif
    int selected=0;
#ifdef PS3_GAME_ORBIT_FIX37
    bool homebrew=false,remember_last_game=true,selected_favorite=false;
#endif
#ifdef PS3_GAME_ORBIT_FIX40
    int sort_mode=0,reorder_position=0,reorder_count=0;bool reorder=false;
#endif
#ifdef PS3_GAME_ORBIT_FIX41
    int case_style=0;
#endif
    std::string status;
    bool operator==(const GameMenuStateFix35& o) const {
        return open==o.open && busy==o.busy && selected==o.selected && status==o.status
#ifdef PS3_GAME_ORBIT_FIX36
            && animated_background==o.animated_background
#endif
#ifdef PS3_GAME_ORBIT_FIX37
            && homebrew==o.homebrew && remember_last_game==o.remember_last_game && selected_favorite==o.selected_favorite
#endif
#ifdef PS3_GAME_ORBIT_FIX40
            && sort_mode==o.sort_mode && reorder==o.reorder && reorder_position==o.reorder_position && reorder_count==o.reorder_count
#endif
#ifdef PS3_GAME_ORBIT_FIX41
            && case_style==o.case_style
#endif
        ;
    }
    bool operator!=(const GameMenuStateFix35& o) const {return !(*this==o);}
};
