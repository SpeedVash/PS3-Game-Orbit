#pragma once
#include <cstdint>
#include <string>
#include "cover_image.h"
#include "full_cover_layout.h"
#ifdef PS3_GAME_ORBIT_FIX35
#include "hud_cache_fix35.h"
#endif

struct DecodedImageRGBA;

struct GpuTextureStage1 {
    void* gpu_ptr = nullptr;
    std::uint32_t gpu_offset = 0;
    int width = 0;
    int height = 0;
    int pitch = 0;
    bool uploaded = false;
    bool full_cover = false;
    UVRect back_uv{0,0,1,1};
    UVRect spine_uv{0,0,1,1};
    UVRect front_uv{0,0,1,1};
};

// V04: RSX bootstrap + decoded cover upload. The V14 CPU mesh and exact UV atlas are ready;
// shader/framebuffer draw submission is intentionally kept separate so hardware validation can
// isolate image decoding/upload problems from render-state problems.
class RsxStage1 {
public:
    bool init();
    void shutdown();
    bool initialized() const { return initialized_; }

    bool prepare_cover(const CoverImage& image, GpuTextureStage1& out,unsigned manual_orientation=1);
    // Call between acknowledged frames, like cover replacement. The decoded HUD
    // is bounded and goes through the same RGBA -> ARGB byte convention.
    bool prepare_overlay(const DecodedImageRGBA& image,GpuTextureStage1& out);
#ifdef PS3_GAME_ORBIT_FIX32
    bool update_overlay(const DecodedImageRGBA& image,GpuTextureStage1& out);
#endif
#ifdef PS3_GAME_ORBIT_FIX35
    bool update_overlay_regions(const DecodedImageRGBA& image,GpuTextureStage1& out,const std::vector<HudRectFix35>& regions);
#endif
    void release_cover(GpuTextureStage1& tex);
    const std::string& last_error() const { return last_error_; }
#ifdef __PSL1GHT__
    void* native_context() const { return context_; }
    void use_native_context(void* context) { context_=context; }
    void* command_arena(std::size_t& bytes) const {
        bytes=128u<<10;
        return io_buffer_ ? static_cast<std::uint8_t*>(io_buffer_)+(2u<<20) : nullptr;
    }
#endif

private:
    bool initialized_ = false;
    std::string last_error_;
#ifdef __PSL1GHT__
    void* io_buffer_ = nullptr;
    void* context_ = nullptr;
#endif
};
