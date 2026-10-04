
#define PS3_SP_LOADER_FIX28 1
#define PS3_SP_LOADER_FIX29 1
#define PS3_GAME_ORBIT_FIX30 1
#define PS3_GAME_ORBIT_FIX31 1
#define PS3_GAME_ORBIT_FIX32 1
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
using s64=long long;
static bool open_frame;
static unsigned polls,renders,acks,hud_calls,uploads,releases,rescans,decode_failures;
static bool timed;
static unsigned prefetch_calls;
static unsigned scenario,mount_requests,cancellations,disc_checks,preference_writes,layout_writes;
static s64 clock_value;
static std::string fixture;
static s64 sysGetSystemTime(){clock_value+=timed ? 60000000 : 16667;return clock_value;}
namespace RuntimeDiag { template<class... A> void log(const char*,A...) {} }
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
        if(scenario==3) {
            const auto frame=polls++;
            if(frame==60)input.l3.pressed=true;
            if(frame==90)input.cross.pressed=true;
            if(frame==140)input.circle.pressed=true;
            assert(polls<=141);return input;
        }
        switch(polls++){
            case 1:case 2:input.right.pressed=true;break;
            case 3:input.l1.pressed=true;break;
            case 4:input.r1.pressed=true;break;
            case 5:input.start.pressed=true;break;
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
    struct Stats {unsigned draw_calls=6,hud_draw_calls=1,background_draw_calls=1;} stats;
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
    void clear_cover_textures(){assert(!open_frame);covers=0;++releases;}
    unsigned gpu_cover_count() const {return covers;}
    bool set_library_hud(const LibraryHudLinesFix28& lines,const CoverflowState* state){
        assert(!open_frame && state);assert(!lines[0].empty() && !lines[1].empty());++hud_calls;return true;
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
    bool prefetch_nearby_cover(const CoverflowState&,CoverCache&){assert(!open_frame);++prefetch_calls;return false;}
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

namespace ProjectIdentity {constexpr const char* LayoutPath="layout-primary";constexpr const char* FallbackLayoutPath="layout-fallback";constexpr const char* PreferencesPath="primary";constexpr const char* LegacyPreferencesPath="legacy";constexpr const char* LegacyFallbackPreferencesPath="legacy-fallback";constexpr const char* FallbackPreferencesPath="fallback";}
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
 bool start(const GameEntry& g,std::uint64_t){assert(!open_frame);++mount_requests;path_value=g.path;value=scenario==0 ? State::Failed : State::WaitingResponse;return scenario!=0;}
 void update(std::uint64_t,const std::string& id){assert(!open_frame);if(scenario==1){if(value==State::WaitingResponse)value=State::WaitingDisc;else if(value==State::WaitingDisc && id=="BLES01234")value=State::Confirmed;}}
 bool needs_disc_check() const {return value==State::WaitingDisc;}
 bool busy() const {return value==State::WaitingResponse || value==State::WaitingDisc || value==State::Confirmed;}
 State state() const {return value;}
 const std::string& status() const {return text;}
 const std::string& path() const {return path_value;}
 int http_status() const {return value==State::Confirmed ? 200 : 0;}
 bool exit_ready(std::uint64_t) const {return scenario==1 && value==State::Confirmed && polls>=11;}
 void cancel(){assert(!open_frame);++cancellations;}
};
int main(int argc,char** argv){
 assert(argc==2);fixture=argv[1];
 for(scenario=0;scenario<4;++scenario){
  timed=false;open_frame=false;clock_value=0;
  polls=renders=acks=hud_calls=uploads=releases=rescans=decode_failures=prefetch_calls=0;
  mount_requests=cancellations=disc_checks=preference_writes=layout_writes=0;
  int result=0;bool user_exit=false,time_limit=false;
  Stream stream;Lifecycle lifecycle;Pad controller;Renderer renderer;CoverCache cache(3);
  LibraryBrowserFix28 browser(fixture);const auto mesh=build_full_cover_case_fix26();
