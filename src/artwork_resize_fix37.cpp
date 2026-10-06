#include "artwork_resize_fix37.h"
#ifdef PS3_GAME_ORBIT_FIX37
#include <algorithm>
#include <cmath>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wunused-function"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO
#include "third_party/stb_image_write.h"
#pragma GCC diagnostic pop
namespace ArtworkResizeFix37 {
bool resize(const DecodedImageRGBA& source,int width,int height,DecodedImageRGBA& target){
    if(!source.valid() || width<=0 || height<=0 || width>1000 || height>550)return false;
    target.width=width;target.height=height;target.pitch=width*4;target.rgba.resize(std::size_t(target.pitch)*height);
    // Premultiplied-alpha bilinear sampling prevents dark rims on disc PNGs.
    for(int y=0;y<height;++y){
        const float sy=std::clamp((y+.5f)*source.height/height-.5f,0.f,float(source.height-1));
        const int y0=int(sy),y1=std::min(y0+1,source.height-1);const float fy=sy-y0;
        for(int x=0;x<width;++x){
            const float sx=std::clamp((x+.5f)*source.width/width-.5f,0.f,float(source.width-1));
            const int x0=int(sx),x1=std::min(x0+1,source.width-1);const float fx=sx-x0;
            const std::uint8_t* samples[]={source.rgba.data()+std::size_t(y0)*source.pitch+x0*4,
                source.rgba.data()+std::size_t(y0)*source.pitch+x1*4,source.rgba.data()+std::size_t(y1)*source.pitch+x0*4,
                source.rgba.data()+std::size_t(y1)*source.pitch+x1*4};
            const float weights[]={(1-fx)*(1-fy),fx*(1-fy),(1-fx)*fy,fx*fy};
            float alpha=0,rgb[3]={0,0,0};
            for(unsigned i=0;i<4;++i){const float a=samples[i][3]*weights[i];alpha+=a;for(unsigned c=0;c<3;++c)rgb[c]+=samples[i][c]*a;}
            auto* pixel=target.rgba.data()+std::size_t(y)*target.pitch+x*4;
            for(unsigned c=0;c<3;++c)pixel[c]=alpha>.001f?std::uint8_t(std::clamp(rgb[c]/alpha+.5f,0.f,255.f)):0;
            pixel[3]=std::uint8_t(std::clamp(alpha+.5f,0.f,255.f));
        }
    }
    return target.valid();
}
namespace {
void append(void* context,void* data,int size){
    auto& bytes=*static_cast<std::vector<std::uint8_t>*>(context);const auto* first=static_cast<std::uint8_t*>(data);
    bytes.insert(bytes.end(),first,first+size);
}
}
bool encode(const DecodedImageRGBA& image,bool disc,std::vector<std::uint8_t>& bytes){
    bytes.clear();if(!image.valid())return false;
    if(disc)return stbi_write_png_to_func(append,&bytes,image.width,image.height,4,image.rgba.data(),image.pitch)!=0 && !bytes.empty();
    std::vector<std::uint8_t> rgb(std::size_t(image.width)*image.height*3);
    for(int y=0;y<image.height;++y)for(int x=0;x<image.width;++x){
        const auto* source=image.rgba.data()+std::size_t(y)*image.pitch+x*4;
        auto* target=rgb.data()+(std::size_t(y)*image.width+x)*3;
        for(unsigned c=0;c<3;++c)target[c]=std::uint8_t((unsigned(source[c])*source[3]+255u*(255u-source[3])+127u)/255u);
    }
    return stbi_write_jpg_to_func(append,&bytes,image.width,image.height,3,rgb.data(),90)!=0 && !bytes.empty();
}
}
#endif
