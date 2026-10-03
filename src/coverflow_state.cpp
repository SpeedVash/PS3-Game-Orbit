#include "coverflow_state.h"
#include <algorithm>

void rebuild_visible(CoverflowState& s) {
    s.visible.clear();
    for (int i = 0; i < (int)s.games.size(); ++i) {
        const auto& g = s.games[i];
        bool ok = false;
        switch (s.filter) {
            case FilterMode::All: ok = true; break;
            case FilterMode::HDD: ok = g.source == GameSource::HDD; break;
            case FilterMode::USB: ok = g.source == GameSource::USB; break;
            case FilterMode::Favorites: ok = g.favorite; break;
        }
        if (ok) s.visible.push_back(i);
    }
    if (s.visible.empty()) s.selected = 0;
    else s.selected = std::clamp(s.selected, 0, (int)s.visible.size()-1);
    s.center_yaw_deg = 0.0f;
    s.center_pitch_deg = -5.0f;
    s.inspect_mode = false;
}

void move_selection(CoverflowState& s, int delta) {
    if (s.visible.empty()) return;
    const int n = (int)s.visible.size();
    s.selected = (s.selected + delta) % n;
    if (s.selected < 0) s.selected += n;
    s.center_yaw_deg = 0.0f;
    s.center_pitch_deg = -5.0f;
}

const GameEntry* current_game(const CoverflowState& s) {
    if (s.visible.empty() || s.selected < 0 || s.selected >= (int)s.visible.size()) return nullptr;
    int i = s.visible[s.selected];
    return (i >= 0 && i < (int)s.games.size()) ? &s.games[i] : nullptr;
}
GameEntry* current_game(CoverflowState& s) {
    if (s.visible.empty() || s.selected < 0 || s.selected >= (int)s.visible.size()) return nullptr;
    int i = s.visible[s.selected];
    return (i >= 0 && i < (int)s.games.size()) ? &s.games[i] : nullptr;
}
const char* filter_name(FilterMode f) {
    switch (f) {
        case FilterMode::All: return "TODOS";
        case FilterMode::HDD: return "HDD";
        case FilterMode::USB: return "USB";
        case FilterMode::Favorites: return "FAVORITOS";
    }
    return "TODOS";
}
