#include "rsx_stage1.h"
#ifdef PS3_GAME_ORBIT_FIX45
#include "prepared_artwork_fix45.h"
#include "performance_fix35.h"
#include <algorithm>
#include <cstring>
#ifdef __PSL1GHT__
#include <rsx/rsx.h>
#endif
bool RsxStage1::begin_artwork_upload(const PreparedArtworkFix45& image,GpuTextureStage1& out){
    out={};last_error_.clear();
    if(!initialized_ || !image.valid()){last_error_="Invalid prepared artwork or RSX stage";return false;}
    auto* memory=acquire_texture_buffer(image.argb.size(),out.allocation_bytes);
    if(!memory){last_error_="Could not allocate streamed artwork texture";return false;}
    out.gpu_ptr=memory;out.reusable=true;out.width=image.width;out.height=image.height;out.pitch=image.pitch;
#ifdef __PSL1GHT__
    if(rsxAddressToOffset(memory,&out.gpu_offset)!=0){release_cover(out);last_error_="Streamed texture address mapping failed";return false;}
#endif
    out.full_cover=image.full_cover;
    if(out.full_cover){out.back_uv=FullCoverLayout::Back;out.spine_uv=FullCoverLayout::Spine;out.front_uv=FullCoverLayout::Front;}
    else{out.back_uv={0,0,0,0};out.spine_uv={0,0,0,0};out.front_uv={0,0,1,1};}
    // Invisible until every byte, including row padding, is copied and ordered.
    // No whole-image memset or conversion runs on the rendering thread.
    return true;
}
std::size_t RsxStage1::advance_artwork_upload(const PreparedArtworkFix45& image,GpuTextureStage1& out,
                                            std::size_t& copied,std::size_t budget){
    if(!out.gpu_ptr || out.uploaded || copied>=image.argb.size())return 0;
    OrbitPerformanceFix35::Scope timer(OrbitPerformanceFix35::Kind::Upload);
    const auto bytes=std::min(budget,image.argb.size()-copied);
    std::memcpy(static_cast<std::uint8_t*>(out.gpu_ptr)+copied,image.argb.data()+copied,bytes);copied+=bytes;
    if(copied==image.argb.size()){
#ifdef __PSL1GHT__
        __asm__ volatile("sync" ::: "memory");
#endif
        out.uploaded=true;
    }
    return bytes;
}
#endif
