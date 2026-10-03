#pragma once
#include <cstdint>
#include <string>
#include "rsx_stage1.h"

#ifdef __PSL1GHT__
struct _gcmCtxData;
struct _gcmCtrlRegister;

// Serial two-segment FIFO. A segment is reused only after REF, backend label
// and GET all acknowledge the previous end. CPU polling has a deadline.
class RsxCommandStreamFix25 {
public:
    bool init(RsxStage1& stage);
    bool begin_frame();
    bool complete_frame();
    const std::string& last_error() const { return error_; }
    std::uint32_t completed_frames() const { return completed_frames_; }
    std::uint32_t switches() const { return switches_; }
private:
    bool wait_complete(const char* tag);
    bool submit_marker(const char* tag);
    bool room(unsigned words) const;
    void select_segment(unsigned segment);

    RsxStage1* stage_=nullptr;
    _gcmCtxData* context_=nullptr;
    volatile _gcmCtrlRegister* control_=nullptr;
    volatile std::uint32_t* label_=nullptr;
    std::uint32_t* segments_[2]{nullptr,nullptr};
    std::uint32_t offsets_[2]{0,0};
    std::uint32_t sequence_=0x25000000;
    std::uint32_t expected_end_=0;
    std::uint32_t completed_frames_=0;
    std::uint32_t switches_=0;
    unsigned segment_=0;
    bool ready_=false;
    bool acknowledged_=false;
    bool frame_open_=false;
    std::string error_;
};
#endif
