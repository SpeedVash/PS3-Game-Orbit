#pragma once
#include <vector>
#include "game_entry.h"

enum class FilterMode { All, HDD, USB, Favorites };

struct CoverflowState {
    std::vector<GameEntry> games;
    std::vector<int> visible;
    int selected = 0;
    FilterMode filter = FilterMode::All;
    float center_yaw_deg = 0.0f;
    float center_pitch_deg = -5.0f;
    bool inspect_mode = false;
    float transition = 1.0f;
    int navigation_direction = 1;
};

void rebuild_visible(CoverflowState& s);
void move_selection(CoverflowState& s, int delta);
const GameEntry* current_game(const CoverflowState& s);
GameEntry* current_game(CoverflowState& s);
const char* filter_name(FilterMode f);
