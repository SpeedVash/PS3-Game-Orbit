#include "rsx_stage1.h"
#ifdef PS3_GAME_ORBIT_FIX38
#include "image_decode.h"
#include "cover_orientation_fix28.h"
#include "cover_limits_fix31.h"
#include "performance_fix35.h"
#include <cstdlib>
#include <cstring>
#ifdef __PSL1GHT__
#include <rsx/rsx.h>
#endif
bool RsxStage1::prepare_disc_artwork(const CoverImage& image,GpuTextureStage1& out){
    out={};last_error_.clear();
    if(!initialized_){last_error_="RSX stage not initialized";return false;}
    if(!image.valid() || image.width>8192 || image.height>8192 ||
       std::size_t(image.width)*image.height>CoverLimitsFix31::MaximumDecodePixels){
        last_error_="Invalid or oversized disc artwork";return false;
    }
    auto& decoded=decode_scratch_;
    if(!decode_cover_rgba(image,decoded,last_error_) ||
       !orient_cover_rgba_fix28(decoded,read_cover_orientation_fix28(image),last_error_) ||
       !CoverLimitsFix31::fit_texture(decoded))return false;
    // A transparent PNG print sits on an opaque disc substrate. The physical
    // hole and the two 0.5-mm clear rings are separate geometry, not this PNG.
    constexpr unsigned silver[3]={211,216,223};
    for(int y=0;y<decoded.height;++y)for(int x=0;x<decoded.width;++x){
        auto* p=decoded.rgba.data()+std::size_t(y)*decoded.pitch+x*4;
        const unsigned alpha=p[3];
        for(unsigned c=0;c<3;++c)p[c]=std::uint8_t((p[c]*alpha+silver[c]*(255-alpha)+127)/255);
        p[3]=255;
    }
    OrbitPerformanceFix35::Scope timer(OrbitPerformanceFix35::Kind::Upload);
    const std::size_t pitch=(std::size_t(decoded.width)*4+63)&~std::size_t(63);
    const auto bytes=pitch*decoded.height;
    std::size_t allocation_bytes=0;auto* memory=acquire_texture_buffer(bytes,allocation_bytes);
    if(!memory){last_error_="Could not allocate disc artwork texture";return false;}
    std::memset(memory,0,bytes);auto* dst=static_cast<std::uint8_t*>(memory);
    for(int y=0;y<decoded.height;++y)for(int x=0;x<decoded.width;++x){
        const auto* p=decoded.rgba.data()+std::size_t(y)*decoded.pitch+x*4;
        auto* d=dst+std::size_t(y)*pitch+x*4;
        d[0]=255;d[1]=p[0];d[2]=p[1];d[3]=p[2];
    }
    out.gpu_ptr=memory;out.width=decoded.width;out.height=decoded.height;out.pitch=int(pitch);
    out.allocation_bytes=allocation_bytes;out.reusable=true;
#ifdef __PSL1GHT__
    if(rsxAddressToOffset(memory,&out.gpu_offset)!=0){rsxFree(memory);out={};last_error_="Disc artwork address mapping failed";return false;}
    __asm__ volatile("sync" ::: "memory");
#endif
    out.uploaded=true;return true;
}
#endif
