#pragma once
#include "coverflow_state.h"
#include "input_state.h"

class InspectCaseFix25 {
public:
    // One automatic revolution at launch; X can enable continuous rotation.
    bool update(CoverflowState& state,const InputFrame& input,float dt);
    bool automatic() const { return automatic_; }
    void stop_auto() { automatic_=false; }
    float scale() const { return scale_; }
private:
    bool automatic_=true;
    float remaining_turn_=360.0f;
#ifdef PS3_GAME_ORBIT_FIX31
    float scale_=0.55f;
#else
    float scale_=0.72f;
#endif
};
