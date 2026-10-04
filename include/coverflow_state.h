#pragma once
#include <vector>
#include "game_entry.h"
#include "case_pose.h"
#ifdef PS3_GAME_ORBIT_FIX35
#include "game_menu_fix35.h"
#endif

enum class FilterMode { All, HDD, USB, Favorites };
enum class OrbitLayout { Classic, Spine
#ifdef PS3_GAME_ORBIT_FIX32
, List
#endif
};

struct CoverflowState {
    std::vector<GameEntry> games;
    std::vector<int> visible;
    int selected = 0;
    FilterMode filter = FilterMode::All;
#ifdef PS3_GAME_ORBIT_FIX35
    GameMenuStateFix35 menu;
#endif
    float center_yaw_deg = 0.0f;
    float center_pitch_deg = -5.0f;
    bool inspect_mode = false;
#ifdef PS3_GAME_ORBIT_FIX32
    bool inspection_target = false;
    float inspection_phase = 0;
    float browse_yaw = 28, browse_pitch = -5;
#endif
    float transition = 1.0f;
    int navigation_direction = 1;
#ifdef PS3_GAME_ORBIT_FIX31
    OrbitLayout layout = OrbitLayout::Classic;
    float case_scale = 0.55f;
    bool flow_initialized = false;
    std::vector<CasePose> flow;
#endif
};

void rebuild_visible(CoverflowState& s);
void move_selection(CoverflowState& s, int delta);
const GameEntry* current_game(const CoverflowState& s);
GameEntry* current_game(CoverflowState& s);
const char* filter_name(FilterMode f);
