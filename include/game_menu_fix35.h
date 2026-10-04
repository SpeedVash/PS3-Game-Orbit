#pragma once
#include <string>

enum class GameMenuActionFix35 { None, Rename, ReloadCovers, ImportUsb };
struct GameMenuStateFix35 {
    bool open=false,busy=false;
    int selected=0;
    std::string status;
    bool operator==(const GameMenuStateFix35& o) const {
        return open==o.open && busy==o.busy && selected==o.selected && status==o.status;
    }
    bool operator!=(const GameMenuStateFix35& o) const {return !(*this==o);}
};
