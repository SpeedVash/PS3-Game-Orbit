
#define PS3_SP_LOADER_FIX28 1
#define PS3_SP_LOADER_FIX29 1
#define PS3_GAME_ORBIT_FIX30 1
#define PS3_GAME_ORBIT_FIX31 1
#define PS3_GAME_ORBIT_FIX32 1
#define PS3_GAME_ORBIT_FIX33 1
#define PS3_GAME_ORBIT_FIX34 1
#define PS3_GAME_ORBIT_FIX35 1
#define PS3_GAME_ORBIT_FIX36 1
#include "prefetch_policy_fix36.h"
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>
#include <unordered_set>
#include "library_pair_fix28.h"
#include "cover_orientation_fix28.h"
#include "library_browser_fix28.h"
#include "cover_cache.h"
#include "full_cover_case_fix26.h"
#include "performance_fix35.h"
using s64=long long;
static bool open_frame;
static unsigned polls,renders,acks,hud_calls,uploads,releases,rescans,decode_failures;
static bool timed;
static unsigned prefetch_calls,art_calls,open_mount_requests;
static unsigned scenario,mount_requests,cancellations,disc_checks,preference_writes,layout_writes;
static s64 clock_value;
static unsigned name_writes,imports,invalidations,menu_frames;
static unsigned background_writes,background_updates,inspection_prefetches,last_invalidated_mask;
static std::string fixture;
static s64 sysGetSystemTime(){clock_value+=timed ? 60000000 : 16667;return clock_value;}
namespace RuntimeDiag { template<class... A> void log(const char*,A...) {} void flush_if_idle(bool,std::uint64_t){assert(!open_frame);} }
namespace OrbitPerformanceFix35 {std::uint64_t now_us(){return clock_value;} void record(Kind,std::uint64_t){} void report(){assert(!open_frame);}}
struct Stream {
    bool begin_frame(){assert(!open_frame);open_frame=true;return true;}
    bool complete_frame(){assert(open_frame);open_frame=false;++acks;return true;}
    unsigned completed_frames() const {return acks;}
    unsigned switches() const {return acks;}
};
struct Lifecycle {void pump(){} bool exit_requested() const {return false;}};
struct Pad {
    InputFrame poll(){
        InputFrame input;input.connected=true;
        if(timed){++polls;return input;}
        if(scenario>=6){
            const auto frame=polls++;
            if(frame==2)input.start.pressed=true;
            if(frame>=3 && frame<3+(scenario==9 ? 4u : scenario==10 ? 3u : scenario-6))input.down.pressed=true;
            if(frame==6)input.cross.pressed=true;
            if(frame==10 || frame==11)input.circle.pressed=true;
            assert(polls<=12);return input;
        }
        if(scenario>=3) {
            const auto frame=polls++;
            if(frame==60)input.l3.pressed=true;
            if(frame==90)input.cross.pressed=true;
            if(frame==140 || (scenario==5 && frame==230))input.circle.pressed=true;
            assert(polls<=231);return input;
        }
        switch(polls++){
            case 1:case 2:input.right.pressed=true;break;
            case 3:input.l1.pressed=true;break;
            case 4:input.r1.pressed=true;break;
            case 5:break;
            case 6:input.select.pressed=input.square.pressed=true;break;
            case 7:input.cross.pressed=true;break;
            case 8:input.right_x=1;break;
            case 9:input.circle.pressed=true;break;
        }
        if(scenario==1) input.circle.pressed=false;
        assert(polls<14);return input;
    }
};
struct Renderer {
    std::string error;
    unsigned covers=1;std::unordered_set<int> uploaded;
    struct Stats {unsigned draw_calls=6,hud_draw_calls=1,background_draw_calls=1,wave_draw_calls=3;} stats;
    bool sync_visible_covers(const CoverflowState& state,CoverCache&,int radius){
        assert(!open_frame && radius==1);++uploads;uploaded.clear();
        for(const auto& pose:LibraryPairFix28::poses(state,radius)){
            const auto& game=state.games[pose.game_index];
            if(game.path=="GAME2" && !game.cover_path.empty()) ++decode_failures;
            else if(!game.cover_path.empty()) uploaded.insert(pose.game_index);
        }
        covers=uploaded.size();return true;
    }
    bool has_cover_texture(int index) const {return uploaded.count(index);}
    bool has_full_cover_texture(int index) const {return uploaded.count(index);}
    void invalidate_game_art(int,unsigned mask=7){assert(!open_frame);++invalidations;last_invalidated_mask=mask;}
    void clear_cover_textures(){assert(!open_frame);covers=0;++releases;}
    unsigned gpu_cover_count() const {return covers;}
    bool sync_inspection_art(const CoverflowState&){assert(!open_frame);++art_calls;return true;}
    bool set_library_hud(const LibraryHudLinesFix28& lines,const CoverflowState* state){
        assert(!open_frame && state);assert(!lines[0].empty() && !lines[1].empty());if(state->menu.open)++menu_frames; ++hud_calls;return true;
    }
    void set_diagnostic_view(const char*,V14Surface){}
    bool render(const CoverflowState& state,const V14CaseMesh&,int radius,float scale){
        assert(open_frame && radius==1 && scale>=0.55f && scale<=0.80f);
        assert(current_game(state));stats.draw_calls=LibraryPairFix28::poses(state,1).size()*6;plan.packets.resize(stats.draw_calls);++renders;return true;
    }
    struct Plan {std::vector<int> packets;} plan;
    const Plan& last_frame_plan() const {return plan;}
    unsigned gpu_cache_bytes() const {return covers*4096;}
    unsigned long long gpu_cache_hits() const {return 0;}
    unsigned long long gpu_cache_misses() const {return uploads;}
    unsigned long long gpu_cache_evictions() const {return releases;}
    bool prefetch_nearby_cover(const CoverflowState& s,CoverCache&){assert(!open_frame && !s.menu.open && !s.inspection_target && s.inspection_phase==0);++prefetch_calls;return false;}
    bool prefetch_inspection_art(const CoverflowState& s){assert(!open_frame && !s.menu.open && !s.inspection_target && s.inspection_phase==0);++inspection_prefetches;return false;}
    bool update_orbit_background(bool enabled,float dt){assert(!open_frame && dt>=0 && dt<=.1f);++background_updates;stats.wave_draw_calls=enabled?3:0;return true;}
    unsigned inside_cache_count() const {assert(!open_frame);return 0;}
    unsigned disc_cache_count() const {assert(!open_frame);return 0;}
    unsigned inside_cache_bytes() const {assert(!open_frame);return 0;}
    unsigned disc_cache_bytes() const {assert(!open_frame);return 0;}
    unsigned long long inside_cache_hits() const {assert(!open_frame);return 0;}
    unsigned long long disc_cache_hits() const {assert(!open_frame);return 0;}
    unsigned long long inside_cache_misses() const {assert(!open_frame);return 0;}
    unsigned long long disc_cache_misses() const {assert(!open_frame);return 0;}
    const Stats& last_stats() const {return stats;}
    const std::string& last_error() const {return error;}
};
static std::vector<GameEntry> scan_catalog(){
    assert(!open_frame);++rescans;
    std::vector<GameEntry> games(3);
    for(unsigned i=0;i<games.size();++i){
        games[i].title="Test "+std::to_string(i);games[i].path="GAME"+std::to_string(i);
        if(i!=1){games[i].cover_path=fixture;games[i].cover_kind=GameCoverKind::FullCover;}
    }
    return games;
}

namespace ProjectIdentity {constexpr const char* BackgroundPath="background";constexpr const char* FallbackBackgroundPath="background-fallback";constexpr const char* NamesPath="names";constexpr const char* FallbackNamesPath="names-fallback";constexpr const char* CoverDirectory="covers";constexpr const char* LayoutPath="layout-primary";constexpr const char* FallbackLayoutPath="layout-fallback";constexpr const char* PreferencesPath="primary";constexpr const char* LegacyPreferencesPath="legacy";constexpr const char* LegacyFallbackPreferencesPath="legacy-fallback";constexpr const char* FallbackPreferencesPath="fallback";}
namespace GameToolsFix35 {
struct Names {bool load(const std::string&){assert(!open_frame);return false;} void apply(std::vector<GameEntry>&)const{assert(!open_frame);} bool set(const std::string&,const std::string&){assert(!open_frame);return true;} bool save(const std::string&)const{assert(!open_frame);++name_writes;return true;}};
struct ImportResult {bool ok;std::string message;unsigned updated_mask;};
static ImportResult import_usb(const GameEntry&,const std::vector<std::string>& roots,const std::string&){assert(!open_frame && roots.size()==8 && roots[0]=="/dev_usb000" && roots[7]=="/dev_usb007");++imports;return {true,"Imported",2};}
static void resolve_art(GameEntry&,const std::string&){assert(!open_frame);}
}
struct RenameKeyboardFix35 {
 bool active=false,ready=false;
 bool begin(const std::string&){assert(!open_frame);active=true;return true;}
 void poll(){assert(!open_frame);if(active){active=false;ready=true;}}
 bool busy()const{return active;}
 bool take_result(std::string& title,bool& accepted){assert(!open_frame);if(!ready)return false;ready=false;accepted=true;title="Renamed Game";return true;}
 void shutdown(){assert(!open_frame);active=false;}
};
struct PreferencesFix29 {
 bool load(const std::string&){assert(!open_frame);return false;}
 bool save(const std::string& p) const {assert(!open_frame);++preference_writes;return scenario!=2 || p=="fallback";}
 void apply(std::vector<GameEntry>&) const {assert(!open_frame);}
 void capture(const CoverflowState&) {assert(!open_frame);}
 FilterMode filter() const {return FilterMode::All;}
 std::string selected_path() const {return "";}
};
struct LayoutSettingsFix31 {
 bool load(const std::string&){assert(!open_frame);return false;}
 bool save(const std::string& p) const {assert(!open_frame);++layout_writes;return scenario!=2 || p=="layout-fallback";}
 void capture(OrbitLayout){assert(!open_frame);}
 OrbitLayout layout() const {return OrbitLayout::Classic;}
};
static std::string read_param_sfo_string(const std::string&,const std::string&){assert(!open_frame);++disc_checks;return "BLES01234";}
struct MountOperationFix29 {
 enum class State {Idle,WaitingResponse,WaitingDisc,Confirmed,Unverified,Failed};
 State value=State::Idle;std::string text,path_value;
 bool start(const GameEntry& g,std::uint64_t){assert(!open_frame);++mount_requests;path_value=g.path;if(scenario>=3)++open_mount_requests;value=(scenario==0 || scenario==5) ? State::Failed : State::WaitingResponse;return scenario!=0 && scenario!=5;}
 void update(std::uint64_t,const std::string& id){assert(!open_frame);if(scenario==1 || scenario==4){if(value==State::WaitingResponse)value=State::WaitingDisc;else if(value==State::WaitingDisc && id=="BLES01234")value=State::Confirmed;}}
 bool needs_disc_check() const {return value==State::WaitingDisc;}
 bool busy() const {return value==State::WaitingResponse || value==State::WaitingDisc || value==State::Confirmed;}
 State state() const {return value;}
 const std::string& status() const {return text;}
 const std::string& path() const {return path_value;}
 int http_status() const {return value==State::Confirmed ? 200 : 0;}
 bool exit_ready(std::uint64_t) const {return (scenario==1 || scenario==4) && value==State::Confirmed && polls>=11;}
 void cancel(){assert(!open_frame);++cancellations;}
};
struct BackgroundSettingsFix36 {
 bool value=true;
 bool load(const std::string&){assert(!open_frame);return false;}
 bool save(const std::string&)const{assert(!open_frame);++background_writes;return true;}
 bool animated()const{return value;}
 void capture(bool enabled){assert(!open_frame);value=enabled;}
};
struct TexturePoolStats {
 unsigned texture_buffer_allocations()const{return 0;}
 unsigned texture_buffer_reuses()const{return 0;}
 unsigned pooled_texture_bytes()const{return 0;}
};
int main(int argc,char** argv){
 assert(argc==2);fixture=argv[1];
 for(scenario=0;scenario<11;++scenario){
  timed=false;open_frame=false;clock_value=0;
  polls=renders=acks=hud_calls=uploads=releases=rescans=decode_failures=prefetch_calls=art_calls=open_mount_requests=0;
  mount_requests=cancellations=disc_checks=preference_writes=layout_writes=0;name_writes=imports=invalidations=menu_frames=0;
  background_writes=background_updates=inspection_prefetches=last_invalidated_mask=0;
  int result=0;bool user_exit=false,time_limit=false;
  TexturePoolStats rsx;Stream stream;Lifecycle lifecycle;Pad controller;Renderer renderer;CoverCache cache(15,16u<<20);
  LibraryBrowserFix28 browser(fixture);const auto mesh=build_full_cover_case_fix26();
