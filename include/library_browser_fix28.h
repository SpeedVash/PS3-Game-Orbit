#pragma once
#include <array>
#include <string>
#include <vector>
#include "app_controller.h"
#include "inspect_case_fix25.h"

using LibraryHudLinesFix28=std::array<std::string,6>;

class LibraryBrowserFix28 {
public:
    explicit LibraryBrowserFix28(std::string diagnostic_cover,std::string second_diagnostic_cover="");
    void replace_catalog(std::vector<GameEntry> games);
    AppCommands update(const InputFrame& input,float dt);
    const CoverflowState& state() const { return state_; }
    CoverflowState render_state() const;
    LibraryHudLinesFix28 hud_lines(const std::string& cover_status) const;
    float scale() const { return inspect_.scale(); }
    bool automatic() const { return inspect_.automatic(); }
    bool help_open() const { return help_open_; }
    void restore_selection(FilterMode filter,const std::string& path);
#ifdef PS3_GAME_ORBIT_FIX31
    void restore_layout(OrbitLayout layout);
#endif
    void set_operation_status(std::string status,bool busy){operation_status_=std::move(status);mount_busy_=busy;}
    bool take_preferences_changed(){const bool changed=preferences_changed_;preferences_changed_=false;return changed;}
private:
    CoverflowState state_;
    AppController navigation_;
    InspectCaseFix25 inspect_;
    std::string diagnostic_cover_;
    std::string second_diagnostic_cover_;
    std::string operation_status_;
    bool mount_busy_=false;
    bool preferences_changed_=false;
    bool help_open_=false;
#ifdef PS3_GAME_ORBIT_FIX31
    std::string pending_path_;
    int pending_layout_=-1;
#endif
    void front_pose();
    std::string selected_path() const;
};
