#pragma once
#ifdef PS3_GAME_ORBIT_FIX41
#include "input_state.h"
#include "coverflow_state.h"
#include <cmath>
#include <cstdint>
class InteractionPolicyFix41 {
public:
    static constexpr std::int64_t QuietUs=650000;
    void note(const InputFrame& input,std::int64_t now){
        if(!input.connected)return;
        bool used=std::fabs(input.right_x)>.18f||std::fabs(input.right_y)>.18f||std::fabs(input.left_x)>.18f||std::fabs(input.left_y)>.18f;
        const ButtonEdge* buttons[]={&input.left,&input.right,&input.up,&input.down,&input.cross,&input.circle,&input.square,&input.triangle,
            &input.l1,&input.r1,&input.l2,&input.r2,&input.start,&input.select,&input.l3,&input.r3};
        for(const auto* b:buttons)used=used||b->pressed||b->held;
        if(used){last_=now;seen_=true;}
    }
    bool quiet(std::int64_t now) const{return !seen_||(now>=last_&&now-last_>=QuietUs);}
    bool idle(std::int64_t now,const CoverflowState& state) const{
        if(!quiet(now)||state.transition<.999f)return false;
        for(const auto& p:state.flow)if(p.visibility<.999f)return false;
        return true;
    }
private:
    std::int64_t last_=0;bool seen_=false;
};
#endif
