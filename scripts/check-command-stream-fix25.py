#!/usr/bin/env python3
"""Exercise the actual two-segment controller against a small FIFO API model.

The model checks jumps, acknowledgements and refusal paths. It does not emulate
RSX prefetch, cache coherence, driver behavior, GPU draws or real hardware.
"""
from pathlib import Path
import hashlib
import subprocess
import tempfile

root=Path(__file__).resolve().parent.parent
source=(root/'src/rsx_command_stream_fix25.cpp').read_text()
source=source.replace('__asm__ volatile("sync" ::: "memory");','/* CPU barrier omitted only in the host model. */')
header=r'''
#pragma once
#include <cstdint>
using u32=std::uint32_t; using s32=std::int32_t; using s64=std::int64_t;
struct _gcmCtxData;
using gcmContextCallback=s32 (*)(_gcmCtxData*,u32);
struct _gcmCtxData { u32* begin;u32* end;u32* current;gcmContextCallback callback; };
using gcmContextData=_gcmCtxData;
struct _gcmCtrlRegister { volatile u32 put,get,ref; };
using gcmControlRegister=_gcmCtrlRegister;
struct gcmConfiguration { void* ioAddress;u32 ioSize; };
static unsigned char* io_memory;
static gcmConfiguration config;
static gcmControlRegister control;
static u32 labels[256];
static s64 clock_us;
static bool exit_request,fail_map,hold_get,hold_ref,hold_label;
static unsigned jumps,flushes;
static void gcmGetConfiguration(gcmConfiguration* p){*p=config;}
static gcmControlRegister* gcmGetControlRegister(){return &control;}
static u32* gcmGetLabelAddress(unsigned index){return &labels[index];}
static s32 rsxAddressToOffset(const void* p,u32* offset){
    const auto a=reinterpret_cast<std::uintptr_t>(p),b=reinterpret_cast<std::uintptr_t>(io_memory);
    if(fail_map || a<b || a-b>8u*1024*1024-4) return -1;
    *offset=static_cast<u32>(a-b);return 0;
}
static void rsxSetJumpCommand(gcmContextData* c,u32 offset){
    assert(c->current<c->end);*c->current++=0x20000000u|offset;
}
static void rsxSetWriteBackendLabel(gcmContextData* c,unsigned index,u32 value){
    assert(c->current+3<c->end);
    *c->current++=0x30000000;*c->current++=index;*c->current++=value;
}
static void rsxSetReferenceCommand(gcmContextData* c,u32 value){
    assert(c->current+2<c->end);*c->current++=0x40000000;*c->current++=value;
}
static void rsxFlushBuffer(gcmContextData* c){
    ++flushes;
    const u32 old_get=control.get,old_ref=control.ref,old_label=labels[254];
    assert(rsxAddressToOffset(c->current,const_cast<u32*>(&control.put))==0);
    unsigned count=0;
    while(control.get!=control.put){
        assert(++count<20000 && control.get<8u*1024*1024-8);
        const u32 word=*reinterpret_cast<u32*>(io_memory+control.get);
        control.get+=4;
        if((word&0xf0000000u)==0x20000000u){control.get=word&0x0fffffffu;++jumps;}
        else if(word==0x30000000){
            const u32 index=*reinterpret_cast<u32*>(io_memory+control.get);control.get+=4;
            labels[index]=*reinterpret_cast<u32*>(io_memory+control.get);control.get+=4;
        }else if(word==0x40000000){
            control.ref=*reinterpret_cast<u32*>(io_memory+control.get);control.get+=4;
        }else assert(word==0); // Payload stands in for already consumed drawing commands.
    }
    if(hold_get) control.get=old_get;
    if(hold_ref) control.ref=old_ref;
    if(hold_label) labels[254]=old_label;
}
static s64 sysGetSystemTime(){return clock_us;}
static int usleep(unsigned usec){clock_us+=usec;return 0;}
static void sysUtilCheckCallback(){}
'''
prefix=r'''
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <rsx/rsx.h>
#define private public
#include "rsx_command_stream_fix25.h"
#undef private
namespace RuntimeDiag { void log(const char*,...){} }
bool Ps3Lifecycle::global_exit_requested(){return exit_request;}
'''
# Include the lifecycle declaration before defining its mocked static method.
prefix=prefix.replace('namespace RuntimeDiag','\n#include "ps3_lifecycle.h"\nnamespace RuntimeDiag')
suffix=r'''
static gcmContextData initial;
static void reset(RsxStage1& stage){
    std::memset(io_memory,0,8u<<20);std::memset(labels,0,sizeof(labels));
    initial={reinterpret_cast<u32*>(io_memory+0x1000),reinterpret_cast<u32*>(io_memory+0x7ffc),
             reinterpret_cast<u32*>(io_memory+0x1000),nullptr};
    stage.io_buffer_=io_memory;stage.context_=&initial;
    config={io_memory,8u<<20};control.put=control.get=0x1000;control.ref=0xffffffff;
    clock_us=0;exit_request=fail_map=hold_get=hold_ref=hold_label=false;
    jumps=flushes=callback_failures=0;
}
int main(){
    void* allocation=nullptr;assert(posix_memalign(&allocation,1u<<20,8u<<20)==0);
    io_memory=static_cast<unsigned char*>(allocation);
    RsxStage1 stage;
    reset(stage);
    RsxCommandStreamFix25 stream;assert(stream.init(stage));
    assert(jumps==1 && control.get==stream.expected_end_ && labels[254]==stream.sequence_);
    for(unsigned frame=0;frame<10000;++frame){
        assert(stream.begin_frame());
        auto* c=static_cast<gcmContextData*>(stage.native_context());
        assert(c->current==stream.segments_[(frame+1)%2]);
        for(unsigned n=0;n<1024;++n) *c->current++=0;
        rsxFlushBuffer(c); // Renderer submits and flips before the completion marker.
        assert(control.get==control.put && !stream.acknowledged_);
        assert(stream.complete_frame());
        assert(control.get==control.put && control.ref==stream.sequence_ && labels[254]==stream.sequence_);
    }
    assert(stream.completed_frames()==10000 && stream.switches()==10000 && jumps==10001);
    puts("PASS: actual stream controller hands off the default FIFO and executes 10000 acknowledged alternating segments");
    const unsigned saved_jumps=jumps,saved_flushes=flushes;
    control.ref=0;
    assert(!stream.begin_frame() && jumps==saved_jumps && flushes==saved_flushes);
    control.ref=stream.sequence_;assert(stream.begin_frame());
    assert(!stream.begin_frame());
    assert(stream.complete_frame());
    puts("PASS: unacknowledged completion and a second open frame refuse reuse");
    for(unsigned fault=0;fault<3;++fault){
        reset(stage);RsxCommandStreamFix25 failed;assert(failed.init(stage));assert(failed.begin_frame());
        hold_get=fault==0;hold_ref=fault==1;hold_label=fault==2;
        assert(!failed.complete_frame() && clock_us==3000000);
        const unsigned before=flushes;
        assert(!failed.begin_frame() && flushes==before && failed.completed_frames()==0);
    }
    puts("PASS: missing GET, REF or backend label each time out and block further segment reuse");
    reset(stage);RsxCommandStreamFix25 exiting;assert(exiting.init(stage));assert(exiting.begin_frame());
    hold_label=true;exit_request=true;
    assert(!exiting.complete_frame() && clock_us==0);
    reset(stage);RsxCommandStreamFix25 overflow;assert(overflow.init(stage));
    auto* c=static_cast<gcmContextData*>(stage.native_context());
    assert(c->callback(c,100000)==-1 && !overflow.begin_frame());
    puts("PASS: exit interrupts CPU polling; overflow callback fails without invoking SDK rollover");
    reset(stage);config.ioSize=1u<<20;RsxCommandStreamFix25 range;
    assert(!range.init(stage) && flushes==0);
    reset(stage);fail_map=true;RsxCommandStreamFix25 mapping;
    assert(!mapping.init(stage) && flushes==0);
    reset(stage);initial.end=initial.current;RsxCommandStreamFix25 room;
    assert(!room.init(stage) && flushes==0);
    puts("PASS: invalid arena, mapping and initial jump room fail before submission");
    std::free(allocation);
    puts("HOST FIFO MODEL: not a GPU/driver test; real command fetch, coherence and segment reuse need PS3 validation");
}
'''
print('SOURCE_SHA256: '+hashlib.sha256((root/'src/rsx_command_stream_fix25.cpp').read_bytes()).hexdigest(),flush=True)
with tempfile.TemporaryDirectory(prefix='fix25-fifo-') as name:
    folder=Path(name)
    for path,contents in {
        'rsx/rsx.h':header,
        'ppu-asm.h':'#pragma once\n#define __get_opd32(x) reinterpret_cast<std::uintptr_t>(x)\n',
        'lv2/systime.h':'#pragma once\n',
        'sysutil/sysutil.h':'#pragma once\n',
        'unistd.h':'#pragma once\n',
    }.items():
        p=folder/path;p.parent.mkdir(parents=True,exist_ok=True);p.write_text(contents)
    cpp=folder/'check.cpp';binary=folder/'check'
    cpp.write_text(prefix+source+suffix)
    result=subprocess.run(['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror','-D__PSL1GHT__=1',
                          '-I'+str(folder),'-I'+str(root/'include'),str(cpp),'-o',str(binary)],
                         capture_output=True,text=True,timeout=30)
    if result.returncode: print(result.stderr)
    assert result.returncode==0
    subprocess.run([str(binary)],check=True,timeout=20)
