#pragma once
#include <array>
#include <string>
#include <vector>
#include "app_controller.h"
#include "inspect_case_fix25.h"
#ifdef PS3_GAME_ORBIT_FIX37
#include "orbit_flow_fix31.h"
#endif

using LibraryHudLinesFix28=std::array<std::string,6>;

class LibraryBrowserFix28 {
public:
    explicit LibraryBrowserFix28(std::string diagnostic_cover,std::string second_diagnostic_cover="");
    void replace_catalog(std::vector<GameEntry> games,bool preserve_pose=false);
    AppCommands update(const InputFrame& input,float dt);
    const CoverflowState& state() const { return state_; }
    CoverflowState render_state() const;
    LibraryHudLinesFix28 hud_lines(const std::string& cover_status) const;
    float scale() const { return inspect_.scale(); }
    bool automatic() const { return inspect_.automatic(); }
    bool help_open() const { return help_open_; }
#ifdef PS3_GAME_ORBIT_FIX48
    void restore_fan_config(const Orbit48FanConfig& config){state_.menu.fan_config=config;}
    void fan_snapshot(const Orbit48FanStatus& status,const std::string& message){
        if(!state_.menu.open || !state_.menu.fan_menu)return;
        state_.menu.fan_config=status.config;
        state_.menu.fan_telemetry="CPU: "+std::to_string(status.cpu_c)+"°C · RSX: "+
            std::to_string(status.rsx_c)+"°C · Fan: "+
            ((status.state==ORBIT48_FAN_EXTERNAL || status.state==ORBIT48_FAN_UNSUPPORTED)?
                std::string("--"):std::to_string(status.percent))+"%";
        state_.menu.status=message;
    }
#endif
#ifdef PS3_GAME_ORBIT_FIX37
    GameEntry* entry(int index){return index>=0 && index<int(state_.games.size())?&state_.games[std::size_t(index)]:nullptr;}
    void restore_view(float zoom,float yaw,float pitch,bool automatic,bool remember){
        inspect_.restore(zoom,automatic);state_.case_scale=zoom;state_.center_yaw_deg=yaw;state_.center_pitch_deg=pitch;
        state_.menu.remember_last_game=remember;OrbitFlowFix31::reset(state_);preferences_changed_=false;
    }
#endif
#ifdef PS3_GAME_ORBIT_FIX35
#ifdef PS3_GAME_ORBIT_FIX36
    void restore_background(bool animated){state_.menu.animated_background=animated;preferences_changed_=false;}
#endif
    GameEntry* selected_entry(){return current_game(state_);}
    void menu_status(std::string status,bool busy=false){state_.menu.status=std::move(status);state_.menu.busy=busy;}
#endif
#ifdef PS3_GAME_ORBIT_FIX40
    void restore_order(const GameOrderFix40::Saved& order);
    void refresh_order();
#endif
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
