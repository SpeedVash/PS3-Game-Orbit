#include "library_pair_fix28.h"
#include <algorithm>
namespace LibraryPairFix28 {
std::vector<CasePose> poses(const CoverflowState& state,int radius){
#ifdef PS3_GAME_ORBIT_FIX31
    return OrbitFlowFix31::poses(state,radius);
#else
    auto selected=build_coverflow_render_plan(state,0);
    if(radius<=0 || selected.empty() || state.visible.size()<2) return selected;
    const auto neighborhood=build_coverflow_render_plan(state,1);
    for(const auto& pose:neighborhood){
        if(pose.relative_slot==1 && pose.game_index!=selected.front().game_index){
            selected.push_back(pose);break;
        }
    }
#ifdef PS3_SP_LOADER_FIX29
    const float t=std::clamp(state.transition,0.0f,1.0f);
    const float ease=t*t*(3-2*t),remaining=1-ease;
    auto& center=selected.front();
    center.x=state.navigation_direction*185*remaining;
    center.z=55-125*remaining;
    center.yaw_deg+=(-state.navigation_direction*36-center.yaw_deg)*remaining;
    center.pitch_deg+=(-3-center.pitch_deg)*remaining;
    if(selected.size()>1) selected[1].x+=65*remaining;
#endif
    return selected;
#endif
}
}
