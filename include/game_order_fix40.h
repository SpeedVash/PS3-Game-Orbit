#pragma once
#ifdef PS3_GAME_ORBIT_FIX40
#include "game_entry.h"
#include <string>
#include <vector>
enum class GameSortFix40 { Alphabetical, Reverse, Custom };
namespace GameOrderFix40 {
const char* name(GameSortFix40 mode);
void reconcile(std::vector<std::string>& order,const std::vector<GameEntry>& games);
void sort_visible(std::vector<int>& visible,const std::vector<GameEntry>& games,GameSortFix40 mode,const std::vector<std::string>& order);
bool move_visible(std::vector<std::string>& order,const std::vector<GameEntry>& games,const std::vector<int>& visible,int from,int to);
struct Saved {
    GameSortFix40 mode=GameSortFix40::Alphabetical;
    std::vector<std::string> paths;
    bool load(const std::string& path);
    bool save(const std::string& path) const;
};
}
#endif
