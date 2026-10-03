#!/usr/bin/env python3
"""Exercise the actual warm_up/submit/poll code with simulated APIs, not a PS3."""
import hashlib
from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parent.parent
path = root/'src/rsx_present_fix20.cpp'
source = path.read_text()
helpers = source[source.index('namespace {'):source.index('namespace RsxPresentFix20 {')]

def function(signature):
    start = source.index(signature)
    body = source.index('{', start)
    depth, end = 1, body+1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]

warmup = function('bool warm_up(').replace('initial_pixels,std::size_t initial_pixel_count','initial_pixels=nullptr,std::size_t initial_pixel_count=0').replace(
    '__asm__ volatile("sync" ::: "memory");',
    'std::atomic_thread_fence(std::memory_order_seq_cst);')
prefix = r'''
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <atomic>
#include <string>
using u8=std::uint8_t;
using u32=std::uint32_t;
using s32=std::int32_t;
using s64=std::int64_t;
struct gcmContextData { u32 *begin,*current,*end; };
struct gcmDisplayInfo { u32 offset,pitch,width,height; };
struct gcmConfiguration { void* localAddress; u32 localSize; };
constexpr std::size_t Pixels=1280*720;
constexpr u32 Bytes=Pixels*sizeof(u32);
alignas(64) static u32 framebuffer[Pixels*2];
static u32 commands[4096];
static void* buffers[2]={framebuffer,framebuffer+Pixels};
static u32 offsets[2]={0,Bytes};
static gcmDisplayInfo display_info[2]={{0,5120,1280,720},{Bytes,5120,1280,720}};
static gcmContextData ctx;
static s64 fake_time,initial_complete;
static unsigned calls,pending_each,pending,fail_call;
static bool never_complete,exit_requested;
static u8 targets[64],displayed;
namespace RuntimeDiag { void log(const char*,...) {} }
struct Ps3Lifecycle { static bool global_exit_requested() { return exit_requested; } };
static void sysUtilCheckCallback() {}
static s64 sysGetSystemTime() { return ++fake_time; }
static void usleep(unsigned delay) { fake_time+=delay; }
static void gcmResetFlipStatus() { pending=pending_each; }
static s32 gcmSetFlip(gcmContextData* context,u8 target) {
    assert(calls<64);
    targets[calls++]=target;
    if(calls==fail_call) return -7;
    context->current+=29; // FIX19 on this console consumed 116 command bytes.
    displayed=target;
    return 0;
}
static void rsxFlushBuffer(gcmContextData*) {}
static u32 gcmGetFlipStatus() {
    if(never_complete) return 1;
    if(pending){ --pending; return 1; }
    if(calls==1) initial_complete=fake_time;
    return 0;
}
static s32 gcmGetCurrentDisplayBufferId(u8* id) { *id=displayed; return 0; }
static const gcmDisplayInfo* gcmGetDisplayInfo() { return display_info; }
static void gcmGetConfiguration(gcmConfiguration* memory) {
    memory->localAddress=framebuffer;
    memory->localSize=Bytes*2;
}
namespace RsxPresentFix20 { void log_display(const char*); }
static void reset() {
    ctx={commands,commands,commands+4096};
    fake_time=initial_complete=0;
    calls=fail_call=pending=0;
    pending_each=2;
    never_complete=exit_requested=false;
    displayed=255;
    offsets[1]=Bytes;
}
'''
suffix = r'''
int main() {
    std::string error;
    auto run=[&] { error.clear(); return RsxPresentFix20::warm_up(&ctx,buffers,offsets,1280,720,5120,error); };
    reset();
    assert(run());
    assert(calls==ExtraFlips+1 && displayed==1 && targets[0]==1);
    for(unsigned i=1;i<calls;++i) assert(targets[i]==((i-1)&1u));
    assert(framebuffer[0]==Magenta && framebuffer[Pixels-1]==Magenta && framebuffer[Pixels*2-1]==Magenta);
    assert(ctx.current==commands+61*29);
    puts("PASS: both buffers magenta; initial target 1 plus 60 alternating flips; final display 1");

    static u32 startup[Pixels];
    for(std::size_t i=0;i<Pixels;++i) startup[i]=0xff000000u|u32(i & 0x00ffffffu);
    reset();assert(RsxPresentFix20::warm_up(&ctx,buffers,offsets,1280,720,5120,error,startup,Pixels));
    for(std::size_t i=0;i<Pixels;++i) assert(framebuffer[i]==startup[i] && framebuffer[Pixels+i]==startup[i]);
    assert(calls==61 && displayed==1);
    puts("PASS: actual startup pixels copied to both buffers; same 61 bounded flips, no magenta substitution");
    reset();assert(!RsxPresentFix20::warm_up(&ctx,buffers,offsets,1280,720,5120,error,startup,Pixels-1) && calls==0);
    puts("PASS: mismatched startup bitmap rejected before writing/submitting");
    reset(); never_complete=true;
    assert(!run() && calls==1 && error=="Flip polling reached its deadline");
    assert(fake_time>=TimeoutUs && fake_time<TimeoutUs+1000);
    puts("PASS: stalled initial flip stops at its 3-second polling deadline");

    reset(); pending_each=8000;
    assert(!run() && calls==3 && error=="Flip polling reached its deadline");
    assert(fake_time-initial_complete>=TimeoutUs && fake_time-initial_complete<TimeoutUs+1000);
    puts("PASS: extra flips share one 3-second deadline, including a slow later flip");

    reset(); fail_call=4;
    assert(!run() && calls==4 && error=="gcmSetFlip failed");
    puts("PASS: submission error stops further flips");

    reset(); exit_requested=true;
    assert(!run() && calls==0 && error=="Exit requested during presentation");
    puts("PASS: exit request prevents submission");

    reset(); ctx.current=ctx.end-24;
    assert(!run() && calls==0 && error=="Presentation command buffer has insufficient room");
    puts("PASS: insufficient initial command space prevents submission");

    reset(); ctx.end=commands+100;
    assert(!run() && calls==3 && error=="Presentation command buffer has insufficient room");
    puts("PASS: command-space exhaustion during the sequence prevents the next submission");

    reset(); offsets[1]=Bytes+4;
    assert(!run() && calls==0 && error=="Renderer framebuffer is outside the expected RSX local allocation");
    puts("PASS: framebuffer offset mismatch stops before CPU fill/flip");
    puts("SIMULATION ONLY: no PS3 syscalls, GPU, memory ordering or TV visibility were tested");
}
'''
actual_functions = '\n'.join(function(name) for name in ('void log_display(', 'void log_buffers('))
print('SOURCE_SHA256: '+hashlib.sha256(path.read_bytes()).hexdigest(), flush=True)
with tempfile.TemporaryDirectory(prefix='fix20-presentation-') as directory:
    folder=Path(directory)
    cpp=folder/'presentation.cpp'
    binary=folder/'presentation'
    cpp.write_text(prefix+helpers+'\nnamespace RsxPresentFix20 {\n'+actual_functions+'\n'+warmup+'\n}\n'+suffix)
    subprocess.run(['g++','-std=c++17','-Wall','-Wextra','-Werror','-O2',str(cpp),'-o',str(binary)],check=True,timeout=30)
    subprocess.run([str(binary)],check=True,timeout=20)
