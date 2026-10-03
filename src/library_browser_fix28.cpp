#include "library_browser_fix28.h"
#include "safe_boot.h"
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include "cover_orientation_fix28.h"
#include "project_identity.h"

LibraryBrowserFix28::LibraryBrowserFix28(std::string cover,std::string second):
    diagnostic_cover_(std::move(cover)),second_diagnostic_cover_(std::move(second)) {
    front_pose();
}
void LibraryBrowserFix28::front_pose(){
    InputFrame reset;reset.connected=true;reset.square.pressed=true;
    inspect_.update(state_,reset,0);
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
}
AppCommands LibraryBrowserFix28::update(const InputFrame& input,float dt){
    dt=std::clamp(dt,0.0f,0.1f);
    const auto previous=selected_path();
    auto navigation=input;
    const auto old_filter=state_.filter;
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
    const auto commands=navigation_.update(state_,navigation,dt);
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
#ifdef PS3_SP_LOADER_FIX29
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
    const auto* changed_game=current_game(state_);
    if(previous!=selected_path() || state_.filter!=old_filter ||
       (changed_game && (changed_game->favorite!=favorite || changed_game->cover_orientation!=orientation))) preferences_changed_=true;
    return commands;
}
void LibraryBrowserFix28::restore_selection(FilterMode filter,const std::string& path){
    state_.filter=filter;rebuild_visible(state_);
    for(std::size_t i=0;i<state_.visible.size();++i) if(state_.games[state_.visible[i]].path==path){state_.selected=int(i);break;}
    state_.transition=1;front_pose();preferences_changed_=false;
}
CoverflowState LibraryBrowserFix28::render_state() const {
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
    empty.center_yaw_deg=state_.center_yaw_deg;empty.center_pitch_deg=state_.center_pitch_deg;
    return empty;
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
        "0 / 0    START para atualizar a biblioteca";
    lines[3]=operation_status_;
    lines[4]=help_open_ ? "HELP" : "";
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
