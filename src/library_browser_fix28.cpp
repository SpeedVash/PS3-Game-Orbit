#include "library_browser_fix28.h"
#include "safe_boot.h"
#include <algorithm>
#include <cmath>
#include <unordered_set>
#include <unordered_map>
#include "cover_orientation_fix28.h"
#include "project_identity.h"
#ifdef PS3_GAME_ORBIT_FIX31
#include "orbit_flow_fix31.h"
#endif

LibraryBrowserFix28::LibraryBrowserFix28(std::string cover,std::string second):
    diagnostic_cover_(std::move(cover)),second_diagnostic_cover_(std::move(second)) {
    front_pose();
}
void LibraryBrowserFix28::front_pose(){
    InputFrame reset;reset.connected=true;reset.square.pressed=true;
    inspect_.update(state_,reset,0);
#ifdef PS3_GAME_ORBIT_FIX31
    if(state_.layout==OrbitLayout::Spine) state_.center_yaw_deg=0;
    state_.case_scale=inspect_.scale();
#endif
}
std::string LibraryBrowserFix28::selected_path() const {
    const auto* game=current_game(state_);return game ? game->path : "";
}
void LibraryBrowserFix28::replace_catalog(std::vector<GameEntry> games){
    const auto previous=selected_path();
    std::unordered_set<std::string> favorites;
    std::unordered_map<std::string,unsigned> orientations;
    for(const auto& g:state_.games) if(g.favorite) favorites.insert(g.path);
    for(const auto& g:state_.games) orientations[g.path]=g.cover_orientation;
    for(auto& g:games){
        if(favorites.count(g.path)) g.favorite=true;
        const auto old=orientations.find(g.path);
        if(old!=orientations.end()) g.cover_orientation=old->second;
#ifdef PS3_GAME_ORBIT_FIX30
        g.cover_orientation=1; // Normal artwork is automatic; no saved manual flips.
#endif
    }
    state_.games=std::move(games);rebuild_visible(state_);
    for(std::size_t i=0;i<state_.visible.size();++i){
        if(state_.games[state_.visible[i]].path==previous){state_.selected=int(i);break;}
    }
    front_pose();
#ifdef PS3_GAME_ORBIT_FIX31
    pending_path_.clear();pending_layout_=-1;OrbitFlowFix31::reset(state_);
#endif
}
AppCommands LibraryBrowserFix28::update(const InputFrame& raw_input,float dt){
    auto input=raw_input;
    dt=std::clamp(dt,0.0f,0.1f);
#ifdef PS3_GAME_ORBIT_FIX35
    AppCommands menu_command;
#ifdef PS3_GAME_ORBIT_FIX37
    constexpr int MenuItems=4;
    const bool menu_was_open=state_.menu.open;
    bool toggled=false;
    if(input.connected && !mount_busy_ && !state_.menu.busy &&
       (input.start.pressed || (input.triangle.pressed && !state_.visible.empty()))){
        const bool home=input.start.pressed;
        state_.menu.open=!(state_.menu.open && state_.menu.homebrew==home);
        state_.menu.homebrew=home;help_open_=false;toggled=true;
        if(state_.menu.open){state_.menu.selected=0;state_.menu.status.clear();pending_path_.clear();pending_layout_=-1;}
    }
    input.start.pressed=input.triangle.pressed=false;
    if(menu_was_open || state_.menu.open){
        if(!toggled && !state_.menu.busy && menu_was_open && input.connected){
            if(input.circle.pressed)state_.menu.open=false;
            if(input.up.pressed)state_.menu.selected=(state_.menu.selected+MenuItems-1)%MenuItems;
            if(input.down.pressed)state_.menu.selected=(state_.menu.selected+1)%MenuItems;
            if(input.cross.pressed && state_.menu.open){
                if(state_.menu.homebrew){
                    switch(state_.menu.selected){
                    case 0:menu_command.rescan_library=true;state_.menu.open=false;break;
                    case 1:state_.menu.animated_background=!state_.menu.animated_background;preferences_changed_=true;break;
                    case 2:menu_command.import_all_usb=true;break;
                    case 3:state_.menu.remember_last_game=!state_.menu.remember_last_game;preferences_changed_=true;break;
                    }
                }else{
                    switch(state_.menu.selected){
                    case 0:menu_command.game_menu_action=GameMenuActionFix35::Rename;break;
                    case 1:menu_command.game_menu_action=GameMenuActionFix35::ImportUsb;break;
                    case 2:menu_command.game_menu_action=GameMenuActionFix35::ReloadCovers;break;
                    case 3:
                        if(auto* game=current_game(state_)){
                            game->favorite=!game->favorite;preferences_changed_=true;
                            if(state_.filter==FilterMode::Favorites){rebuild_visible(state_);OrbitFlowFix31::reset(state_);}
                            state_.menu.open=false;
                        }
                        break;
                    }
                }
            }
        }
        input=InputFrame{};
    }
#else
#ifdef PS3_GAME_ORBIT_FIX36
    constexpr int MenuItems=5;
#else
    constexpr int MenuItems=4;
#endif
    const bool menu_was_open=state_.menu.open;
    if(input.connected && input.start.pressed && !mount_busy_
#ifndef PS3_GAME_ORBIT_FIX36
        && !state_.visible.empty()
#endif
        && !state_.menu.busy){
        state_.menu.open=!state_.menu.open;help_open_=false;
        if(state_.menu.open){state_.menu.selected=0;state_.menu.status.clear();pending_path_.clear();pending_layout_=-1;}
        input.start.pressed=false;
    }
    if(menu_was_open || state_.menu.open){
        if(!state_.menu.busy && menu_was_open && input.connected){
            if(input.circle.pressed)state_.menu.open=false;
            if(input.up.pressed)state_.menu.selected=(state_.menu.selected+MenuItems-1)%MenuItems;
            if(input.down.pressed)state_.menu.selected=(state_.menu.selected+1)%MenuItems;
            if(input.cross.pressed && state_.menu.open){
                if(state_.menu.selected==0)menu_command.game_menu_action=GameMenuActionFix35::Rename;
                else if(state_.menu.selected==1)menu_command.game_menu_action=GameMenuActionFix35::ReloadCovers;
                else if(state_.menu.selected==2)menu_command.game_menu_action=GameMenuActionFix35::ImportUsb;
#ifdef PS3_GAME_ORBIT_FIX36
                else if(state_.menu.selected==3){state_.menu.animated_background=!state_.menu.animated_background;preferences_changed_=true;}
#endif
                else {menu_command.rescan_library=true;state_.menu.open=false;}
            }
        }
        input=InputFrame{};
    }
#endif
#endif
#ifdef PS3_GAME_ORBIT_FIX32
    const bool was_inspecting=state_.inspection_target || state_.inspection_phase>0;
    if(input.connected && input.l3.pressed && !mount_busy_ && !state_.visible.empty()) {
        if(!was_inspecting) {
            state_.browse_yaw=state_.center_yaw_deg;state_.browse_pitch=state_.center_pitch_deg;
            pending_path_.clear();pending_layout_=-1;
            inspect_.stop_auto();state_.center_yaw_deg=342;state_.center_pitch_deg=-5;
        }
        state_.inspection_target=!state_.inspection_target;
    }
    const bool inspecting=was_inspecting || state_.inspection_target;
    if(inspecting) {
#ifdef PS3_GAME_ORBIT_FIX33
        if(input.circle.pressed && !mount_busy_) {
            state_.inspection_target=false;input.circle.pressed=false;input.cross.pressed=false;
        }
#else
        if(input.circle.pressed || input.cross.pressed) state_.inspection_target=false;
        input.circle.pressed=input.cross.pressed=false;
#endif
        input.left.pressed=input.right.pressed=input.up.pressed=input.down.pressed=false;
        input.l1.pressed=input.r1.pressed=input.triangle.pressed=input.square.pressed=input.start.pressed=false;
        input.left_x=input.left_y=0;
        input.l2.held=input.r2.held=false;
    } else if(state_.layout==OrbitLayout::List) {
        input.left.pressed=input.left.pressed || input.up.pressed;
        input.right.pressed=input.right.pressed || input.down.pressed;
        if(std::fabs(input.left_y)>.42f) input.left_x=input.left_y;
        input.up.pressed=input.down.pressed=false;
    }
    state_.inspection_phase=std::clamp(state_.inspection_phase+
        (state_.inspection_target ? dt/1.4f : -dt/1.4f),0.0f,1.0f);
    if(inspecting && !state_.inspection_target && state_.inspection_phase==0) {
        state_.center_yaw_deg=state_.browse_yaw;state_.center_pitch_deg=state_.browse_pitch;
    }
#endif
    const auto previous=selected_path();
    auto navigation=input;
    const auto old_filter=state_.filter;
#ifdef PS3_GAME_ORBIT_FIX31
    const auto old_selected=state_.selected;
    const auto old_yaw=state_.center_yaw_deg,old_pitch=state_.center_pitch_deg;
    const auto old_layout=state_.layout;
    const bool explicit_navigation=input.left.pressed || input.right.pressed || std::fabs(input.left_x)>.42f;
    if(explicit_navigation) pending_path_.clear();
    if(!mount_busy_ && !explicit_navigation && !pending_path_.empty()) {
        for(std::size_t i=0;i<state_.visible.size();++i) {
            if(state_.games[state_.visible[i]].path==pending_path_) {state_.selected=int(i);break;}
        }
        pending_path_.clear();
    }
    if(input.connected && !mount_busy_ && (input.square.pressed || pending_layout_>=0)) {
        state_.layout=pending_layout_>=0 ? static_cast<OrbitLayout>(pending_layout_) :
#ifdef PS3_GAME_ORBIT_FIX32
            state_.layout==OrbitLayout::Classic ? OrbitLayout::Spine :
            state_.layout==OrbitLayout::Spine ? OrbitLayout::List : OrbitLayout::Classic;
#else
            state_.layout==OrbitLayout::Classic ? OrbitLayout::Spine : OrbitLayout::Classic;
#endif
        pending_layout_=-1;
        if(!OrbitFlowFix31::can_admit(state_)) {pending_layout_=int(state_.layout);state_.layout=old_layout;}
        else front_pose();
    }
#endif
    const auto* old_game=current_game(state_);
    const bool favorite=old_game && old_game->favorite;
    const unsigned orientation=old_game ? old_game->cover_orientation : 1;
#ifndef PS3_SP_LOADER_FIX29
    navigation.cross.pressed=false;navigation.square.pressed=false;navigation.r3.pressed=false;
#else
    navigation.square.pressed=false;navigation.r3.pressed=false;
    if(mount_busy_){
        navigation.left.pressed=navigation.right.pressed=false;navigation.left_x=0;
        navigation.l1.pressed=navigation.r1.pressed=navigation.triangle.pressed=false;
        navigation.cross.pressed=navigation.start.pressed=false;
    }
#endif
    navigation.right_x=navigation.right_y=0;
    auto commands=navigation_.update(state_,navigation,dt);
#ifdef PS3_GAME_ORBIT_FIX35
    commands.game_menu_action=menu_command.game_menu_action;
    commands.rescan_library=commands.rescan_library || menu_command.rescan_library;
#ifdef PS3_GAME_ORBIT_FIX37
    commands.import_all_usb=menu_command.import_all_usb;
#endif
#endif
#ifdef PS3_GAME_ORBIT_FIX31
    if(!OrbitFlowFix31::can_admit(state_)) {
        if(state_.filter==old_filter) {
            pending_path_=selected_path();state_.selected=old_selected;
            state_.center_yaw_deg=old_yaw;state_.center_pitch_deg=old_pitch;
        } else OrbitFlowFix31::reset(state_);
    }
#endif
#ifndef PS3_GAME_ORBIT_FIX30
    if(input.connected && input.l3.pressed && !mount_busy_){
        auto* game=current_game(state_);
        if(game){
            constexpr unsigned modes[]={1,3,6,8,2,4};
            unsigned i=0;while(i<6 && modes[i]!=game->cover_orientation) ++i;
            game->cover_orientation=modes[(i+1)%6];
        }
    }
#else
    if(input.connected && input.select.pressed) help_open_=!help_open_;
#endif
    if(previous!=selected_path()){
        front_pose();
#ifdef PS3_GAME_ORBIT_FIX31
        if(explicit_navigation) state_.navigation_direction=input.left.pressed || input.left_x<-.42f ? -1 : 1;
#endif
#if defined(PS3_SP_LOADER_FIX29) && !defined(PS3_GAME_ORBIT_FIX31)
        if(state_.filter==old_filter && !previous.empty() && !selected_path().empty()){
            state_.transition=0;
            state_.navigation_direction=input.left.pressed || input.left_x < -0.42f ? -1 : 1;
        }else state_.transition=1;
#endif
    }else state_.transition=std::min(1.0f,state_.transition+dt/0.22f);
    auto rotation=input;
    rotation.circle.pressed=false;
    rotation.left.held=rotation.right.held=rotation.up.held=rotation.down.held=false;
    // Triangle belongs to session favorites. SELECT uses the validated back pose.
    rotation.triangle.pressed=input.select.pressed;
#ifdef PS3_GAME_ORBIT_FIX30
    rotation.triangle.pressed=false;
    rotation.square.pressed=false;
#endif
#ifdef PS3_SP_LOADER_FIX29
    rotation.cross.pressed=input.up.pressed;
#endif
    inspect_.update(state_,rotation,dt);
#ifdef PS3_GAME_ORBIT_FIX37
    if(input.connected && (input.l2.held || input.r2.held || input.r3.pressed || input.up.pressed ||
       std::fabs(input.right_x)>.18f || std::fabs(input.right_y)>.18f))preferences_changed_=true;
    state_.menu.selected_favorite=current_game(state_) && current_game(state_)->favorite;
    if(state_.visible.empty()){
        state_.inspection_target=false;state_.inspection_phase=0;
        if(!state_.menu.homebrew)state_.menu.open=false;
        OrbitFlowFix31::reset(state_);
    }
#endif
#ifdef PS3_GAME_ORBIT_FIX31
    if(input.connected && input.r3.pressed && state_.layout==OrbitLayout::Spine) state_.center_yaw_deg=0;
    state_.case_scale=inspect_.scale();
    OrbitFlowFix31::advance(state_,dt);
    if(state_.layout!=old_layout) preferences_changed_=true;
#endif
    const auto* changed_game=current_game(state_);
    if(previous!=selected_path() || state_.filter!=old_filter ||
       (changed_game && (changed_game->favorite!=favorite || changed_game->cover_orientation!=orientation))) preferences_changed_=true;
    return commands;
}
void LibraryBrowserFix28::restore_selection(FilterMode filter,const std::string& path){
    state_.filter=filter;rebuild_visible(state_);
    for(std::size_t i=0;i<state_.visible.size();++i) if(state_.games[state_.visible[i]].path==path){state_.selected=int(i);break;}
    state_.transition=1;front_pose();preferences_changed_=false;
#ifdef PS3_GAME_ORBIT_FIX31
    pending_path_.clear();OrbitFlowFix31::reset(state_);
#endif
}
#ifdef PS3_GAME_ORBIT_FIX31
void LibraryBrowserFix28::restore_layout(OrbitLayout layout) {
    state_.layout=layout;front_pose();OrbitFlowFix31::reset(state_);
    pending_layout_=-1;preferences_changed_=false;
}
#endif
CoverflowState LibraryBrowserFix28::render_state() const {
#ifdef PS3_GAME_ORBIT_FIX37
    return state_;
#else
    if(!state_.visible.empty()) return state_;
#ifdef PS3_GAME_ORBIT_FIX30
    auto empty=make_safe_boot_state("");
#else
    auto empty=make_safe_boot_state(state_.games.empty() ? diagnostic_cover_ : "");
    if(state_.games.empty() && !second_diagnostic_cover_.empty()){
        auto game=empty.games.front();game.path="DEMO_FIX28_2";game.title="Segunda capa de teste";
        game.cover_path=second_diagnostic_cover_;empty.games.push_back(game);empty.visible.push_back(1);
    }
#endif
#ifdef PS3_GAME_ORBIT_FIX36
    empty.menu=state_.menu;
#endif
    empty.center_yaw_deg=state_.center_yaw_deg;empty.center_pitch_deg=state_.center_pitch_deg;
#ifdef PS3_GAME_ORBIT_FIX31
    empty.layout=state_.layout;empty.case_scale=inspect_.scale();OrbitFlowFix31::reset(empty);
#endif
    return empty;
#endif
}
LibraryHudLinesFix28 LibraryBrowserFix28::hud_lines(const std::string& cover_status) const {
    const char* filter=state_.filter==FilterMode::HDD ? "HDD" : state_.filter==FilterMode::USB ? "USB" :
                       state_.filter==FilterMode::Favorites ? "FAVORITOS" : "TODOS";
    LibraryHudLinesFix28 lines;
#ifdef PS3_GAME_ORBIT_FIX30
    (void)cover_status;
    (void)filter;
    lines[0]=ProjectIdentity::Name;
    const auto* game=current_game(state_);
    lines[1]=game ? game->title : state_.games.empty() ? "Sua biblioteca está vazia" : "Nenhum jogo neste filtro";
    lines[2]=game ? std::to_string(state_.selected+1)+" / "+std::to_string(state_.visible.size())+"    "+
        (game->source==GameSource::HDD ? "HDD" : "USB")+"    "+(game->format==GameFormat::ISO ? "ISO" : "Pasta")+
        (game->title_id.empty() ? "" : "    "+game->title_id)+(game->favorite ? "    Favorito" : "") :
        #ifdef PS3_GAME_ORBIT_FIX36
        "0 / 0    START para abrir opções";
#else
        "0 / 0    START para atualizar a biblioteca";
#endif
    lines[3]=operation_status_;
    lines[4]=help_open_ ? "HELP" : "";
#ifdef PS3_GAME_ORBIT_FIX31
#ifdef PS3_GAME_ORBIT_FIX32
    lines[4]+=(state_.layout==OrbitLayout::List ? "|LIST" : state_.layout==OrbitLayout::Spine ? "|SPINE" : "|CLASSIC");
    if(state_.inspection_target || state_.inspection_phase>0) lines[4]+="|OPEN";
#ifdef PS3_GAME_ORBIT_FIX33
    if(mount_busy_) lines[4]+="|MOUNT";
#endif
#else
    lines[4]+=(state_.layout==OrbitLayout::Spine ? "|SPINE" : "|CLASSIC");
#endif
#endif
    lines[5]=state_.filter==FilterMode::HDD ? "HDD" : state_.filter==FilterMode::USB ? "USB" :
        state_.filter==FilterMode::Favorites ? "Favoritos" : "Todos";
    return lines;
#else
    lines[0]=std::string("PS3_SP_LOADER ")+ProjectIdentity::Version+" | DUAS CAPAS | "+filter;
    const auto* g=current_game(state_);
    if(g){
        lines[1]=g->title;
        lines[2]=std::to_string(state_.selected+1)+"/"+std::to_string(state_.visible.size())+"  "+
                 (g->source==GameSource::HDD ? "HDD" : "USB")+" / "+
                 (g->format==GameFormat::ISO ? "ISO" : "PASTA")+"  "+
                 (g->title_id.empty() ? "SEM ID" : g->title_id)+"  "+cover_status+
                 (g->favorite ? "  FAVORITO" : "")+"  "+cover_orientation_name_fix28(g->cover_orientation);
    }else{
        lines[1]=state_.games.empty() ? "Nenhum jogo encontrado - capa de demonstração" : "Nenhum jogo neste filtro";
        lines[2]="0/0  TOTAL: "+std::to_string(state_.games.size())+"  "+cover_status;
    }
    lines[3]="ESQ/DIR: jogo  L1/R1: filtro  TRIANGULO: favorito";
    lines[4]="DIR: girar  L2/R2: zoom  X: auto  L3: orientar capa";
    lines[5]="QUADRADO: frente  SELECT: verso  START: reler  O: sair";
#ifdef PS3_SP_LOADER_FIX29
    lines[3]=operation_status_.empty() ? "ESQ/DIR: jogo  L1/R1: filtro  TRIANGULO: favorito" : operation_status_;
    lines[4]="X: montar  CIMA: auto  DIR: giro  L2/R2: zoom  L3: capa";
#endif
    return lines;
#endif
}
