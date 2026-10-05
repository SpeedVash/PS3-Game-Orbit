#pragma once
#ifdef PS3_GAME_ORBIT_FIX35
#include "game_entry.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
namespace GameToolsFix35 {
bool valid_title(const std::string& title);
std::vector<std::uint16_t> utf16(const std::string& text);
std::string utf8(const std::uint16_t* text,std::size_t count);
class Names {
public:
    bool load(const std::string& path);
    bool save(const std::string& path) const;
    bool set(const std::string& game_path,const std::string& title);
    void apply(std::vector<GameEntry>& games) const;
private: std::unordered_map<std::string,std::string> names_;
};
struct ImportResult {bool ok=false;std::string message;
#ifdef PS3_GAME_ORBIT_FIX36
    unsigned updated_mask=0;
#endif
};
ImportResult import_usb(const GameEntry& game,const std::vector<std::string>& roots,const std::string& covers);
void resolve_art(GameEntry& game,const std::string& covers);
}
#endif
