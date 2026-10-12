#include "prepared_artwork_fix45.h"
#ifdef PS3_GAME_ORBIT_FIX45
#include "cover_limits_fix31.h"
#include "cover_orientation_fix28.h"
#include "image_decode.h"
#include <cstring>

bool ArtworkRequestFix45::operator==(const ArtworkRequestFix45& other) const {
    return game_index==other.game_index && game_path==other.game_path && path==other.path &&
        role==other.role && kind==other.kind && orientation==other.orientation && generation==other.generation;
}
bool PreparedArtworkFix45::valid() const {
    return error.empty() && width>0 && height>0 && width<=1024 && height<=1024 &&
        pitch>=width*4 && (pitch&63)==0 && argb.size()==std::size_t(pitch)*height;
}
PreparedArtworkFix45 prepare_artwork_fix45(const ArtworkRequestFix45& request) {
    PreparedArtworkFix45 out;out.request=request;
    const auto image=load_cover_file(request.path,request.kind);
    if(!image.valid() || image.width>8192 || image.height>8192 ||
       std::size_t(image.width)*image.height>CoverLimitsFix31::MaximumDecodePixels){
        out.error="Unreadable, unsupported or oversized artwork";return out;
    }
    const bool disc=request.role==ArtworkRoleFix45::Disc;
    const unsigned manual=disc?1:default_cover_orientation_fix30(image);
    const unsigned exif=read_cover_orientation_fix28(image);
    const bool direct=!disc && exif==1 && manual==1 && image.width<=1024 && image.height<=1024;
    DecodedImageRGBA rgba;DecodedImageARGB argb;
    if(direct){
        if(!decode_cover_argb_fix35(image,argb,out.error))return out;
        out.width=argb.width;out.height=argb.height;
    }else{
        if(!decode_cover_rgba(image,rgba,out.error) ||
           !orient_cover_rgba_fix28(rgba,exif,out.error) ||
           (!disc && !orient_cover_rgba_fix28(rgba,manual,out.error)) ||
           !CoverLimitsFix31::fit_texture(rgba)){
            if(out.error.empty())out.error="Could not prepare bounded artwork";
            return out;
        }
        out.width=rgba.width;out.height=rgba.height;
    }
    out.pitch=(out.width*4+63)&~63;
    out.argb.assign(std::size_t(out.pitch)*out.height,0);
    constexpr unsigned silver[3]={211,216,223};
    for(int y=0;y<out.height;++y){
        auto* dst=out.argb.data()+std::size_t(y)*out.pitch;
        if(direct){std::memcpy(dst,argb.argb.data()+std::size_t(y)*argb.pitch,std::size_t(out.width)*4);continue;}
        const auto* src=rgba.rgba.data()+std::size_t(y)*rgba.pitch;
        for(int x=0;x<out.width;++x){
            const auto* p=src+x*4;auto* d=dst+x*4;
            d[0]=disc?255:p[3];
            for(unsigned c=0;c<3;++c)
                d[c+1]=disc?std::uint8_t((unsigned(p[c])*p[3]+silver[c]*(255-p[3])+127)/255):p[c];
        }
    }
    out.full_cover=!disc && uses_full_cover_layout_fix28(image,manual);
    return out;
}
#endif
