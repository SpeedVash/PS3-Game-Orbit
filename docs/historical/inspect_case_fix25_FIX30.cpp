#include "inspect_case_fix25.h"
#include <algorithm>
#include <cmath>

namespace {
float axis(float v){
    if(std::fabs(v)<=0.18f) return 0;
    return std::copysign((std::min(1.0f,std::fabs(v))-0.18f)/0.82f,v);
}
float wrap(float yaw){
    const float value=std::fmod(yaw,360.0f);
    return value<0 ? value+360.0f : value;
}
}

bool InspectCaseFix25::update(CoverflowState& state,const InputFrame& in,float dt){
    dt=std::clamp(dt,0.0f,0.1f);
    if(in.connected && in.circle.pressed) return false;
    if(in.connected && in.cross.pressed){
        automatic_=!automatic_;
        remaining_turn_=-1; // Explicitly requested rotation runs until paused.
    }
    float yaw_input=0,pitch_input=0;
    if(in.connected){
        yaw_input=std::clamp(axis(in.right_x)+float(in.right.held)-float(in.left.held),-1.0f,1.0f);
        pitch_input=std::clamp(axis(in.right_y)+float(in.down.held)-float(in.up.held),-1.0f,1.0f);
        if(yaw_input!=0 || pitch_input!=0) automatic_=false;
        if(in.r3.pressed || in.square.pressed || in.triangle.pressed){
            state.center_yaw_deg=in.triangle.pressed ? 152.0f : 28.0f;
            state.center_pitch_deg=-5.0f;
            automatic_=false;
            if(in.r3.pressed) scale_=0.72f;
        }
        scale_=std::clamp(scale_+(float(in.r2.held)-float(in.l2.held))*0.20f*dt,0.55f,0.80f);
    }
    if(automatic_){
        float step=30.0f*dt;
        if(remaining_turn_>=0){
            step=std::min(step,remaining_turn_);
            remaining_turn_=std::max(0.0f,remaining_turn_-step);
            if(remaining_turn_==0) automatic_=false;
        }
        state.center_yaw_deg+=step;
    }else{
        state.center_yaw_deg+=yaw_input*120.0f*dt;
        state.center_pitch_deg=std::clamp(state.center_pitch_deg+pitch_input*65.0f*dt,-28.0f,22.0f);
    }
    state.center_yaw_deg=wrap(state.center_yaw_deg);
    return true;
}
