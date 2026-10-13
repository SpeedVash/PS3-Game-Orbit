#pragma once
#include <string>
#ifdef PS3_GAME_ORBIT_FIX48
#include "orbit_service_protocol_fix48.h"
#endif

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
#ifdef PS3_GAME_ORBIT_FIX48
    bool fan_menu=false;
    Orbit48FanConfig fan_config{ORBIT48_FAN_AUTO,68,35,85,40};
    std::string fan_telemetry;
#endif
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
#ifdef PS3_GAME_ORBIT_FIX48
            && fan_menu==o.fan_menu && fan_telemetry==o.fan_telemetry &&
            fan_config.mode==o.fan_config.mode && fan_config.target_c==o.fan_config.target_c &&
            fan_config.minimum_percent==o.fan_config.minimum_percent &&
            fan_config.maximum_percent==o.fan_config.maximum_percent &&
            fan_config.manual_percent==o.fan_config.manual_percent
#endif
        ;
    }
    bool operator!=(const GameMenuStateFix35& o) const {return !(*this==o);}
};
