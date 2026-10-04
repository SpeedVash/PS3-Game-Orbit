#include "rsx_stage1.h"
#ifdef PS3_GAME_ORBIT_FIX31
#include "cover_limits_fix31.h"
#endif
#include "image_decode.h"
#include "cover_orientation_fix28.h"
#include <cstring>
#include <cstdlib>
#ifdef __PSL1GHT__
#include <malloc.h>
#endif

#ifdef __PSL1GHT__
#include <rsx/rsx.h>
#include "runtime_diag.h"
#endif

static std::size_t align_up(std::size_t v, std::size_t a) {
    return (v + a - 1) & ~(a - 1);
}

bool RsxStage1::init() {
    last_error_.clear();
#ifdef __PSL1GHT__
    constexpr std::size_t IO_SIZE = 8u << 20;
    constexpr std::size_t CMD_SIZE = 1u << 20;
    void* p = memalign(1u << 20, IO_SIZE);
    if (!p) {
        last_error_ = "Could not allocate 1 MiB-aligned RSX IO buffer";
        return false;
    }
    io_buffer_ = p;
    gcmContextData* ctx = nullptr;
    if (rsxInit(&ctx, CMD_SIZE, IO_SIZE, io_buffer_) != 0 || !ctx) {
        std::free(io_buffer_); io_buffer_ = nullptr;
        last_error_ = "rsxInit failed";
        return false;
    }
    context_ = ctx;
#endif
    initialized_ = true;
    return true;
}

void RsxStage1::shutdown() {
#ifdef __PSL1GHT__
    if (io_buffer_) std::free(io_buffer_);
    io_buffer_ = nullptr;
    context_ = nullptr;
#endif
    initialized_ = false;
}

bool RsxStage1::prepare_cover(const CoverImage& image, GpuTextureStage1& out,unsigned manual_orientation) {
#ifdef PS3_GAME_ORBIT_FIX30
    manual_orientation=default_cover_orientation_fix30(image);
#endif
    out = {};
    last_error_.clear();
    if (!initialized_) { last_error_ = "RSX stage not initialized"; return false; }
    if (!image.valid()) { last_error_ = "Invalid cover image"; return false; }
#ifdef PS3_GAME_ORBIT_FIX31
    if(image.width>8192 || image.height>8192 ||
       std::size_t(image.width)*std::size_t(image.height)>CoverLimitsFix31::MaximumDecodePixels) {
        last_error_="Cover exceeds the bounded decode size";return false;
    }
#endif

    DecodedImageRGBA decoded;
#if defined(__PSL1GHT__) && defined(PS3_SP_LOADER_FIX28)
    RuntimeDiag::log("UPLOAD 00: decode begin; path=%s encoded=%u",image.path.c_str(),unsigned(image.encoded.size()));
#endif
    if (!decode_cover_rgba(image, decoded, last_error_)) return false;
    if(!orient_cover_rgba_fix28(decoded,read_cover_orientation_fix28(image),last_error_) ||
       !orient_cover_rgba_fix28(decoded,manual_orientation,last_error_)) return false;
#ifdef PS3_GAME_ORBIT_FIX31
    if(!CoverLimitsFix31::fit_texture(decoded)) {last_error_="Could not bound cover texture";return false;}
#endif
#if defined(__PSL1GHT__) && defined(PS3_SP_LOADER_FIX28)
    RuntimeDiag::log("UPLOAD 01: decode returned; dimensions=%dx%d pitch=%d bytes=%u",decoded.width,decoded.height,decoded.pitch,unsigned(decoded.rgba.size()));
#endif

    const std::size_t pitch = align_up(static_cast<std::size_t>(decoded.width) * 4u, 64u);
    const std::size_t bytes = pitch * static_cast<std::size_t>(decoded.height);
    void* mem = nullptr;
#ifdef __PSL1GHT__
    mem = rsxMemalign(128, static_cast<u32>(bytes));
#else
    if (posix_memalign(&mem, 128, bytes) != 0) mem = nullptr;
#endif
    if (!mem) { last_error_ = "Could not allocate texture memory"; return false; }
    std::memset(mem, 0, bytes);

    // GCM_TEXTURE_FORMAT_A8R8G8B8 is fed in ARGB byte order on the big-endian PPU.
    // Convert canonical RGBA to ARGB during upload so the decoder stays renderer-agnostic.
    auto* dst = static_cast<std::uint8_t*>(mem);
    for (int y=0; y<decoded.height; ++y) {
        const auto* s = decoded.rgba.data() + static_cast<std::size_t>(y) * decoded.pitch;
        auto* d = dst + static_cast<std::size_t>(y) * pitch;
        for (int x=0; x<decoded.width; ++x) {
            d[x*4+0] = s[x*4+3];
            d[x*4+1] = s[x*4+0];
            d[x*4+2] = s[x*4+1];
            d[x*4+3] = s[x*4+2];
        }
    }

    out.gpu_ptr = mem;
    out.width = decoded.width;
    out.height = decoded.height;
    out.pitch = static_cast<int>(pitch);
#ifdef __PSL1GHT__
    if (rsxAddressToOffset(mem, &out.gpu_offset) != 0) {
        rsxFree(mem); out = {}; last_error_ = "rsxAddressToOffset failed"; return false;
    }
    __asm__ volatile("sync" ::: "memory");
#ifdef PS3_SP_LOADER_FIX28
    const auto* samples=static_cast<volatile u32*>(mem);
    RuntimeDiag::log("UPLOAD 02: ARGB upload ordered; offset=0x%08x pitch=%u bytes=%u first=0x%08x center=0x%08x",
                     out.gpu_offset,unsigned(pitch),unsigned(bytes),samples[0],
                     samples[std::size_t(decoded.height/2)*(pitch/sizeof(u32))+decoded.width/2]);
#endif
#endif
    out.uploaded = true;
    out.full_cover = uses_full_cover_layout_fix28(image,manual_orientation);
    if (out.full_cover) {
        out.back_uv = FullCoverLayout::Back;
        out.spine_uv = FullCoverLayout::Spine;
        out.front_uv = FullCoverLayout::Front;
    } else {
        // ICON0/front-only fallback: front uses the entire image; shell/back/spine stay untextured.
        out.front_uv = {0,0,1,1};
        out.back_uv = {0,0,0,0};
        out.spine_uv = {0,0,0,0};
    }
    return true;
}

bool RsxStage1::prepare_overlay(const DecodedImageRGBA& image,GpuTextureStage1& out){
    out={};last_error_.clear();
    if(!initialized_){last_error_="RSX stage not initialized";return false;}
    if(!image.valid() || image.width>2048 || image.height>512){last_error_="Invalid or oversized HUD bitmap";return false;}
    const auto pitch=align_up(std::size_t(image.width)*4,64);
    const auto bytes=pitch*std::size_t(image.height);
    void* mem=nullptr;
#ifdef __PSL1GHT__
    mem=rsxMemalign(128,static_cast<u32>(bytes));
#else
    if(posix_memalign(&mem,128,bytes)!=0) mem=nullptr;
#endif
    if(!mem){last_error_="Could not allocate HUD texture";return false;}
    std::memset(mem,0,bytes);
    auto* dst=static_cast<std::uint8_t*>(mem);
    for(int y=0;y<image.height;++y){
        const auto* src=image.rgba.data()+std::size_t(y)*image.pitch;
        auto* row=dst+std::size_t(y)*pitch;
        for(int x=0;x<image.width;++x){
            row[x*4]=src[x*4+3];row[x*4+1]=src[x*4];
            row[x*4+2]=src[x*4+1];row[x*4+3]=src[x*4+2];
        }
    }
    out.gpu_ptr=mem;out.width=image.width;out.height=image.height;out.pitch=int(pitch);
#ifdef __PSL1GHT__
    if(rsxAddressToOffset(mem,&out.gpu_offset)!=0){rsxFree(mem);out={};last_error_="HUD address mapping failed";return false;}
    __asm__ volatile("sync" ::: "memory");
#endif
    out.uploaded=true;
    return true;
}

void RsxStage1::release_cover(GpuTextureStage1& tex) {
    if (tex.gpu_ptr) {
#ifdef __PSL1GHT__
        rsxFree(tex.gpu_ptr);
#else
        std::free(tex.gpu_ptr);
#endif
    }
    tex = {};
}
