#pragma once
#ifdef PS3_GAME_ORBIT_FIX35
#include "image_decode.h"
#include "game_menu_fix35.h"
#include <array>
#include <string>
#include <vector>
struct HudRectFix35 {int x,y,width,height;};
class HudCacheFix35 {
public:
    std::vector<HudRectFix35> update(const std::array<std::string,6>& lines,const std::vector<std::string>& rows,int active,const GameMenuStateFix35& menu);
    const DecodedImageRGBA& image()const{return image_;}
    std::size_t last_changed_bytes()const{return last_changed_bytes_;}
    void clear(){image_={};lines_={};rows_.clear();active_=-1;menu_={};last_changed_bytes_=0;}
private:
    DecodedImageRGBA image_;
    std::array<std::string,6> lines_{};std::vector<std::string> rows_;int active_=-1;
    GameMenuStateFix35 menu_;std::size_t last_changed_bytes_=0;
};
#endif
