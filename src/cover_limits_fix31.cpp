#include "cover_limits_fix31.h"
#include <algorithm>
#include <cmath>
namespace CoverLimitsFix31 {
bool fit_texture(DecodedImageRGBA& image) {
    if(!image.valid()) return false;
    if(image.width<=TextureEdge && image.height<=TextureEdge) return true;
    const float ratio=float(TextureEdge)/std::max(image.width,image.height);
    DecodedImageRGBA out;
    out.width=std::max(1,int(image.width*ratio));out.height=std::max(1,int(image.height*ratio));
    out.pitch=out.width*4;out.rgba.resize(std::size_t(out.pitch)*out.height);
    for(int y=0;y<out.height;++y) for(int x=0;x<out.width;++x) {
        const float sx=std::clamp((x+.5f)*image.width/out.width-.5f,0.0f,float(image.width-1));
        const float sy=std::clamp((y+.5f)*image.height/out.height-.5f,0.0f,float(image.height-1));
        const int x0=int(sx),y0=int(sy),x1=std::min(x0+1,image.width-1),y1=std::min(y0+1,image.height-1);
        const float fx=sx-x0,fy=sy-y0;
        for(int c=0;c<4;++c) {
            const auto value=[&](int xx,int yy){return image.rgba[std::size_t(yy)*image.pitch+xx*4+c];};
            const float top=value(x0,y0)*(1-fx)+value(x1,y0)*fx;
            const float bottom=value(x0,y1)*(1-fx)+value(x1,y1)*fx;
            out.rgba[std::size_t(y)*out.pitch+x*4+c]=static_cast<unsigned char>(std::lround(top*(1-fy)+bottom*fy));
        }
    }
    image=std::move(out);return true;
}
}
