#include "rsx_command_stream_fix25.h"
#ifdef __PSL1GHT__
#include <cstddef>
#include <new>
#include <rsx/rsx.h>
#include <ppu-asm.h>
#include <lv2/systime.h>
#include <sysutil/sysutil.h>
#include <unistd.h>
#include "ps3_lifecycle.h"
#include "runtime_diag.h"

namespace {
constexpr unsigned SegmentBytes=64u<<10;
constexpr unsigned LabelIndex=254;
constexpr s64 TimeoutUs=3000000;
volatile unsigned callback_failures=0;
s32 refuse_overflow(gcmContextData*,u32){
    ++callback_failures;
    return -1; // Never enter the default GCM rollover callback.
}
gcmContextCallback prx_callback(gcmContextCallback callback){
    return reinterpret_cast<gcmContextCallback>(__get_opd32(callback));
}
std::uintptr_t address(u32* p){ return reinterpret_cast<std::uintptr_t>(p); }
void order_cpu_writes(){ __asm__ volatile("sync" ::: "memory"); }
}

bool RsxCommandStreamFix25::room(unsigned words) const {
    if(!context_) return false;
    const auto b=address(static_cast<u32*>(context_->begin));
    const auto c=address(static_cast<u32*>(context_->current));
    const auto e=address(static_cast<u32*>(context_->end));
    return c>=b && c<=e && e-c>=std::size_t(words)*sizeof(u32);
}

void RsxCommandStreamFix25::select_segment(unsigned segment){
    segment_=segment;
    context_->begin=segments_[segment];
    context_->current=segments_[segment];
    context_->end=segments_[segment]+SegmentBytes/sizeof(u32)-1;
    // Match the pinned SDK's OPD32 conversion; pointer fields use PRXPTR32.
    context_->callback=prx_callback(refuse_overflow);
    stage_->use_native_context(context_);
}

bool RsxCommandStreamFix25::wait_complete(const char* tag){
    const s64 start=sysGetSystemTime();
    unsigned polls=0;
    while(control_->get!=expected_end_ || control_->ref!=sequence_ || *label_!=sequence_){
        if(sysGetSystemTime()-start>=TimeoutUs){
            error_="RSX command completion polling reached its deadline";
            break;
        }
        sysUtilCheckCallback();
        if(Ps3Lifecycle::global_exit_requested()){
            error_="Exit requested during command completion";
            break;
        }
        usleep(200);++polls;
    }
    acknowledged_=control_->get==expected_end_ && control_->ref==sequence_ && *label_==sequence_;
    if(!acknowledged_ || completed_frames_<3 || completed_frames_%60==0){
        RuntimeDiag::log("FIFO %s: segment=%u seq=0x%08x expected_get=0x%08x put=0x%08x get=0x%08x ref=0x%08x label=0x%08x ack=%u polls=%u elapsed_us=%lld switches=%u frames=%u",
                         tag,segment_,sequence_,expected_end_,control_->put,control_->get,control_->ref,*label_,
                         unsigned(acknowledged_),polls,static_cast<long long>(sysGetSystemTime()-start),switches_,completed_frames_);
    }
    return acknowledged_;
}

bool RsxCommandStreamFix25::submit_marker(const char* tag){
    if(!room(6) || callback_failures){error_="RSX marker command room/overflow check failed";return false;}
    ++sequence_;
    rsxSetWriteBackendLabel(context_,LabelIndex,sequence_);
    rsxSetReferenceCommand(context_,sequence_);
    if(rsxAddressToOffset(static_cast<void*>(context_->current),&expected_end_)!=0){
        error_="RSX end address could not be mapped";return false;
    }
    acknowledged_=false;
    rsxFlushBuffer(context_);
    return wait_complete(tag);
}

bool RsxCommandStreamFix25::init(RsxStage1& stage){
    if(ready_){error_="Command stream already initialized";return false;}
    std::size_t arena_bytes=0;
    auto* arena=static_cast<u32*>(stage.command_arena(arena_bytes));
    auto* previous=static_cast<gcmContextData*>(stage.native_context());
    gcmConfiguration config{};
    gcmGetConfiguration(&config); // The real API has a void return.
    const auto io=reinterpret_cast<std::uintptr_t>(static_cast<void*>(config.ioAddress));
    const auto a=address(arena);
    if(!previous || !arena || arena_bytes!=2*SegmentBytes || !io || a<io ||
       config.ioSize<arena_bytes || a-io>config.ioSize-arena_bytes ||
       a-io!=(2u<<20) || (a&(SegmentBytes-1))){
        error_="Command arena is outside the reserved mapped IO range";return false;
    }
    segments_[0]=arena;segments_[1]=arena+SegmentBytes/sizeof(u32);
    for(unsigned i=0;i<2;++i){
        if(rsxAddressToOffset(segments_[i],&offsets_[i])!=0 || (offsets_[i]&3u)){
            error_="Command segment could not be mapped";return false;
        }
    }
    if(offsets_[1]!=offsets_[0]+SegmentBytes){error_="Command segment offsets are not contiguous";return false;}
    control_=gcmGetControlRegister();label_=gcmGetLabelAddress(LabelIndex);
    if(!control_ || !label_){error_="RSX completion registers unavailable";return false;}
    stage_=&stage;
    context_=previous;
    if(!room(1)){error_="Initial FIFO has no jump command room";return false;}
    auto* user_context=new(std::nothrow) gcmContextData{};
    if(!user_context){error_="Command context allocation failed";return false;}
    *label_=0;order_cpu_writes();
    // Write a real jump from the stopped default FIFO to the reserved arena.
    // Leave the default segment and its context alive until process teardown.
    rsxSetJumpCommand(previous,offsets_[0]);
    context_=user_context;
    select_segment(0);
    RuntimeDiag::log("FIFO INIT: arena=%p bytes=%u segments=2 segment_bytes=%u offsets=0x%08x/0x%08x label=%u",
                     arena,unsigned(arena_bytes),SegmentBytes,offsets_[0],offsets_[1],LabelIndex);
    if(!submit_marker("INIT")) return false;
    ready_=true;
    return true;
}

bool RsxCommandStreamFix25::begin_frame(){
    error_.clear();
    if(!ready_ || !acknowledged_ || frame_open_ || callback_failures ||
       control_->get!=expected_end_ || control_->ref!=sequence_ || *label_!=sequence_ || !room(1)){
        error_="FIFO reuse refused: previous commands are not fully acknowledged";return false;
    }
    const unsigned next=segment_^1u;
    // Previous GET is stopped at expected_end_. Only now may we write its
    // jump and reuse the other, already consumed segment. No raw cursor rewind.
    rsxSetJumpCommand(context_,offsets_[next]);
    select_segment(next);
    ++switches_;
    frame_open_=true;
    acknowledged_=false;
    return true;
}

bool RsxCommandStreamFix25::complete_frame(){
    if(!ready_ || !frame_open_){error_="No open frame to acknowledge";return false;}
    if(!submit_marker("FRAME")) return false;
    frame_open_=false;
    ++completed_frames_;
    return true;
}
#endif
