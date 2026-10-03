#include "rsx_present_fix20.h"
#ifdef __PSL1GHT__
#include <cstdint>
#include <cstdio>
#include <unistd.h>
#include <lv2/systime.h>
#include <rsx/rsx.h>
#include <sysutil/sysutil.h>
#include "ps3_lifecycle.h"
#include "runtime_diag.h"

namespace {
constexpr s64 TimeoutUs = 3000000;
constexpr u32 Magenta = 0xffff00ffu;
constexpr unsigned ExtraFlips = 60;

bool exit_pending(std::string& error) {
    sysUtilCheckCallback();
    if (!Ps3Lifecycle::global_exit_requested()) return false;
    error = "Exit requested during presentation";
    return true;
}

bool has_command_room(gcmContextData* ctx, unsigned words) {
    const auto begin = reinterpret_cast<std::uintptr_t>(static_cast<void*>(ctx->begin));
    const auto current = reinterpret_cast<std::uintptr_t>(static_cast<void*>(ctx->current));
    const auto end = reinterpret_cast<std::uintptr_t>(static_cast<void*>(ctx->end));
    return current >= begin && current <= end && end - current >= words * sizeof(u32);
}

bool wait_until(const char* tag, s64 deadline, std::string& error) {
    const s64 start = sysGetSystemTime();
    unsigned polls = 0;
    u32 status;
    do {
        status = gcmGetFlipStatus();
        if (status == 0) break;
        if (sysGetSystemTime() >= deadline) break;
        if (exit_pending(error)) return false;
        usleep(200);
        ++polls;
    } while (sysGetSystemTime() < deadline);
    RuntimeDiag::log("%s: flip status=%u polls=%u elapsed_us=%lld", tag,
                     status, polls, static_cast<long long>(sysGetSystemTime() - start));
    if (status == 0) return true;
    error = "Flip polling reached its deadline";
    return false;
}

bool submit(gcmContextData* ctx, u8 target, const char* tag,
            s64 deadline, std::string& error) {
    if (exit_pending(error)) return false;
    if (sysGetSystemTime() >= deadline) {
        error = "Presentation sequence deadline reached before submission";
        return false;
    }
    if (!has_command_room(ctx, 32)) {
        error = "Presentation command buffer has insufficient room";
        return false;
    }
    gcmResetFlipStatus();
    RuntimeDiag::log("%s: gcmSetFlip begin; target=%u current=%p end=%p", tag,
                     unsigned(target), static_cast<void*>(ctx->current), static_cast<void*>(ctx->end));
    const s32 rc = gcmSetFlip(ctx, target);
    RuntimeDiag::log("%s: gcmSetFlip rc=%d", tag, int(rc));
    if (rc != 0) { error = "gcmSetFlip failed"; return false; }
    rsxFlushBuffer(ctx);
    RuntimeDiag::log("%s: flush returned", tag);
    if (!wait_until(tag, deadline, error)) return false;
    RsxPresentFix20::log_display(tag);
    return true;
}
}

namespace RsxPresentFix20 {
void log_display(const char* tag) {
    u8 id = 255;
    const s32 rc = gcmGetCurrentDisplayBufferId(&id);
    RuntimeDiag::log("DISPLAY %s: rc=%d id=%u", tag, int(rc), unsigned(id));
}

void log_buffers(const char* tag, const std::uint32_t* offsets,
                 int width, int height, std::uint32_t pitch) {
    RuntimeDiag::log("DISPLAY_INFO %s: request", tag);
    const volatile gcmDisplayInfo* info = gcmGetDisplayInfo();
    RuntimeDiag::log("DISPLAY_INFO %s: address=%p", tag, const_cast<const gcmDisplayInfo*>(info));
    if (!info) return;
    for (unsigned i = 0; i < 2; ++i) {
        const u32 offset = info[i].offset;
        const u32 registered_pitch = info[i].pitch;
        const u32 registered_width = info[i].width;
        const u32 registered_height = info[i].height;
        RuntimeDiag::log("DISPLAY_INFO %s.%u: offset=0x%08x pitch=%u width=%u height=%u expected_offset=0x%08x matches_expected=%u",
                         tag, i, offset, registered_pitch, registered_width, registered_height, offsets[i],
                         unsigned(offset == offsets[i] && registered_pitch == pitch &&
                                  registered_width == unsigned(width) && registered_height == unsigned(height)));
    }
}

bool acknowledge(void* native_context, std::string& error) {
    auto* ctx = static_cast<gcmContextData*>(native_context);
    RuntimeDiag::log("ACK 00: pre-video acknowledgement; timeout_us=%lld", static_cast<long long>(TimeoutUs));
    if (!ctx || !has_command_room(ctx, 6)) { error = "ACK context/command room unavailable"; return false; }
    auto* control = gcmGetControlRegister();
    volatile u32* label = gcmGetLabelAddress(255);
    RuntimeDiag::log("ACK 01: control=%p label=%p", control, const_cast<u32*>(label));
    if (!control || !label) { error = "ACK control/label unavailable"; return false; }
    constexpr u32 Reference = 0x18000001u;
    constexpr u32 Label = 0x18000002u;
    *label = 0;
    __asm__ volatile("sync" ::: "memory");
    rsxSetReferenceCommand(ctx, Reference);
    rsxSetWriteBackendLabel(ctx, 255, Label);
    rsxFlushBuffer(ctx);
    RuntimeDiag::log("ACK 04: flush returned; put=0x%08x get=0x%08x", control->put, control->get);
    const s64 start = sysGetSystemTime();
    unsigned polls = 0;
    while (control->ref != Reference || *label != Label) {
        if (sysGetSystemTime() - start >= TimeoutUs) break;
        if (exit_pending(error)) return false;
        usleep(200);
        ++polls;
    }
    RuntimeDiag::log("ACK 05: put=0x%08x get=0x%08x ref=0x%08x label=0x%08x polls=%u elapsed_us=%lld",
                     control->put, control->get, control->ref, *label, polls,
                     static_cast<long long>(sysGetSystemTime() - start));
    if (control->ref != Reference || *label != Label) {
        error = "Pre-video RSX acknowledgement timed out";
        return false;
    }
    RuntimeDiag::log("ACK 06: reference/backend label matched");
    return true;
}

bool wait_flip(const char* tag, std::string& error) {
    return wait_until(tag, sysGetSystemTime() + TimeoutUs, error);
}

bool warm_up(void* native_context, void* const* buffers,
             const std::uint32_t* offsets, int width, int height,
             std::uint32_t pitch, std::string& error) {
    auto* ctx = static_cast<gcmContextData*>(native_context);
    if (!ctx || width != 1280 || height != 720 || pitch != 5120 || !buffers[0] || !buffers[1]) {
        error = "Renderer warm-up context/buffers/configuration unavailable";
        return false;
    }
    gcmConfiguration memory{};
    gcmGetConfiguration(&memory); // Real API is void; ignore the header's return declaration.
    const auto base = reinterpret_cast<std::uintptr_t>(static_cast<void*>(memory.localAddress));
    const std::size_t bytes = std::size_t(pitch) * unsigned(height);
    const std::size_t pixels = bytes / sizeof(u32);
    for (unsigned i = 0; i < 2; ++i) {
        const auto address = reinterpret_cast<std::uintptr_t>(buffers[i]);
        if (!base || memory.localSize < bytes || address < base ||
            address - base > memory.localSize - bytes || address - base != offsets[i] || (address & 63u)) {
            error = "Renderer framebuffer is outside the expected RSX local allocation";
            return false;
        }
    }
    log_buffers("before_warmup", offsets, width, height, pitch);
    RuntimeDiag::log("WARMUP 20: CPU fill both renderer buffers; color=0xffff00ff pixels=%u", unsigned(pixels));
    for (unsigned b = 0; b < 2; ++b) {
        auto* data = static_cast<volatile u32*>(buffers[b]);
        for (std::size_t i = 0; i < pixels; ++i) data[i] = Magenta;
    }
    __asm__ volatile("sync" ::: "memory");
    for (unsigned b = 0; b < 2; ++b) {
        const auto* data = static_cast<volatile u32*>(buffers[b]);
        RuntimeDiag::log("WARMUP 21.%u: first=0x%08x middle=0x%08x last=0x%08x", b,
                         data[0], data[pixels / 2], data[pixels - 1]);
        if (data[0] != Magenta || data[pixels / 2] != Magenta || data[pixels - 1] != Magenta) {
            error = "Renderer CPU magenta verification failed";
            return false;
        }
    }
    if (!submit(ctx, 1, "WARMUP initial", sysGetSystemTime() + TimeoutUs, error)) return false;
    const s64 sequence_start = sysGetSystemTime();
    const s64 deadline = sequence_start + TimeoutUs;
    for (unsigned frame = 0; frame < ExtraFlips; ++frame) {
        char tag[32];
        ::snprintf(tag, sizeof(tag), "WARMUP extra.%u", frame);
        if (!submit(ctx, u8(frame & 1u), tag, deadline, error)) return false;
    }
    RuntimeDiag::log("WARMUP 29: completed extra_flips=%u last_target=1 elapsed_us=%lld; next_draw_buffer=0",
                     ExtraFlips, static_cast<long long>(sysGetSystemTime() - sequence_start));
    log_buffers("after_warmup", offsets, width, height, pitch);
    return true;
}
}
#endif
