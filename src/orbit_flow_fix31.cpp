#include "orbit_flow_fix31.h"
#ifdef PS3_GAME_ORBIT_FIX31
#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace OrbitFlowFix31 {
namespace {
float angle_delta(float target, float current) {
    float d=std::fmod(target-current,360.0f);
    if(d>180) d-=360;
    if(d<-180) d+=360;
    return d;
}
void approach(float& v,float target,float amount,float epsilon=.001f) {
    if(amount==0) return;
    v+=(target-v)*amount;
    if(std::fabs(target-v)<epsilon) v=target;
}
void approach_angle(float& v,float target,float amount) {
    if(amount==0) return;
    const auto d=angle_delta(target,v);
    v+=d*amount;
    if(std::fabs(d)<.01f) v=target;
}
}
const char* layout_name(OrbitLayout layout) {
#ifdef PS3_GAME_ORBIT_FIX32
    if(layout==OrbitLayout::List) return "Lista";
#endif
    return layout==OrbitLayout::Spine ? "Spine" : "Clássico";
}
std::vector<CasePose> targets(const CoverflowState& state,int radius) {
    std::vector<CasePose> out;
    const int n=int(state.visible.size());
    if(n==0 || state.selected<0 || state.selected>=n) return out;
    auto add=[&](int slot) {
        int index=(state.selected+slot)%n;if(index<0) index+=n;
        const int game=state.visible[std::size_t(index)];
        if(game<0 || game>=int(state.games.size())) return;
        for(const auto& p:out) if(p.game_index==game) return;
        CasePose p;p.game_index=game;p.relative_slot=slot;p.selected=slot==0;
        p.scale=MinimumScale;
        if(slot==0) {
            p.z=55;p.scale=state.case_scale;
            p.yaw_deg=state.center_yaw_deg;p.pitch_deg=state.center_pitch_deg;
#ifdef PS3_GAME_ORBIT_FIX32
            if(state.layout==OrbitLayout::List) p.x=80;
            p.inspection_phase=state.inspection_phase;
            const float u=std::clamp(state.inspection_phase/.20f,0.0f,1.0f);
            const float ease=u*u*(3-2*u);
            p.x+=(35-p.x)*ease;p.z+=(72-p.z)*ease;
            p.scale+=(MinimumScale-p.scale)*ease;
#endif
        } else if(state.layout==OrbitLayout::Classic) {
            p.x=150;p.z=-70;p.yaw_deg=-36;p.pitch_deg=-3;p.alpha=.76f;
        } else {
            constexpr float unit=(425.0f-55.0f)/(5.0f-1.85f);
            const int sign=slot<0 ? -1 : 1;
            p.x=(slot<0 ? -.63f : .62f)*unit+sign*(std::abs(slot)-1)*.1f*unit;
            p.z=425-5*unit;
            p.yaw_deg=(slot<0 ? 1.7f : 1.5f)*180.0f/3.14159265358979323846f;
            p.pitch_deg=0;
        }
        out.push_back(p);
    };
    add(0);
    if(radius<=0) return out;
#ifdef PS3_GAME_ORBIT_FIX32
    if(state.layout==OrbitLayout::List || state.inspection_target || state.inspection_phase>0) return out;
#endif
    if(state.layout==OrbitLayout::Classic) add(1);
    else for(int distance=1;distance<=SpineRadius;++distance) {
        // Deduplicate short libraries; never repeat one game around the row.
        add(-distance);add(distance);
    }
    return out;
}
std::vector<CasePose> poses(const CoverflowState& state,int radius) {
    if(radius<=0 || !state.flow_initialized) return targets(state,radius);
    return state.flow;
}
void reset(CoverflowState& state) {
    state.flow=targets(state);state.flow_initialized=true;
}
bool can_admit(const CoverflowState& state) {
    std::unordered_set<int> wanted;
    for(const auto& p:state.flow) if(p.visibility>0) wanted.insert(p.game_index);
    for(const auto& p:targets(state)) wanted.insert(p.game_index);
    return wanted.size()<=MaxCases;
}
void advance(CoverflowState& state,float dt) {
    dt=std::clamp(dt,0.0f,.1f);
    if(!state.flow_initialized) {reset(state);return;}
    const auto desired=targets(state);
    // Browser delays a capacity-exceeding navigation request until an exit
    // has faded. This prevents eviction of a case still visible on screen.
    if(!can_admit(state)) return;
    for(const auto& target:desired) {
        const auto found=std::find_if(state.flow.begin(),state.flow.end(),[&](const CasePose& p){return p.game_index==target.game_index;});
        if(found!=state.flow.end()) continue;
        auto entry=target;
        const int sign=state.navigation_direction<0 ? -1 : 1;
        entry.x+=(state.layout==OrbitLayout::Spine ? 45 : 110)*sign;
        entry.z-=30;entry.visibility=0;entry.selected=false;
        state.flow.push_back(entry);
    }
    // Exponential easing preserves the current pose on every new request.
    // The small settled threshold gives exact stable geometry/UV tests.
    const float amount=1-std::exp(-15*dt);
    for(auto& p:state.flow) {
        const auto found=std::find_if(desired.begin(),desired.end(),[&](const CasePose& t){return p.game_index==t.game_index;});
        CasePose target;
        if(found!=desired.end()) {
            target=*found;p.visibility=std::min(1.0f,p.visibility+dt/.16f);
        } else {
            p.x-=state.navigation_direction*240*dt;p.z-=110*dt;
            target=p;
            p.visibility=std::max(0.0f,p.visibility-dt/.18f);
            target.selected=false;
        }
        approach(p.x,target.x,amount);approach(p.y,target.y,amount);approach(p.z,target.z,amount);
        approach_angle(p.yaw_deg,target.yaw_deg,amount);approach_angle(p.pitch_deg,target.pitch_deg,amount);
        approach(p.scale,target.scale,amount,.00001f);approach(p.alpha,target.alpha,amount);
        p.relative_slot=target.relative_slot;p.selected=target.selected;p.inspection_phase=target.inspection_phase;
    }
    state.flow.erase(std::remove_if(state.flow.begin(),state.flow.end(),[&](const CasePose& p) {
        if(p.visibility>0) return false;
        return std::none_of(desired.begin(),desired.end(),[&](const CasePose& t){return t.game_index==p.game_index;});
    }),state.flow.end());
}
}
#endif
