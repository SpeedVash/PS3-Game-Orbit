#include "cover_orientation_fix28.h"
#include <cstring>
#include <utility>

namespace {
unsigned tiff_orientation(const unsigned char* p,std::size_t size){
    if(size<8) return 1;
    const bool little=p[0]=='I' && p[1]=='I';
    if(!little && !(p[0]=='M' && p[1]=='M')) return 1;
    auto u16=[&](std::size_t x){return little ? unsigned(p[x])|(unsigned(p[x+1])<<8) : (unsigned(p[x])<<8)|unsigned(p[x+1]);};
    auto u32=[&](std::size_t x){return little ? unsigned(p[x])|(unsigned(p[x+1])<<8)|(unsigned(p[x+2])<<16)|(unsigned(p[x+3])<<24) :
                                             (unsigned(p[x])<<24)|(unsigned(p[x+1])<<16)|(unsigned(p[x+2])<<8)|unsigned(p[x+3]);};
    if(u16(2)!=42) return 1;
    const auto offset=u32(4);
    if(offset>size-2) return 1;
    const auto count=u16(offset);
    if(count>(size-offset-2)/12) return 1;
    for(unsigned i=0;i<count;++i){
        const auto entry=std::size_t(offset)+2+12*i;
        if(u16(entry)==0x112 && u16(entry+2)==3 && u32(entry+4)==1){
            const auto value=u16(entry+8);return value>=1 && value<=8 ? value : 1;
        }
    }
    return 1;
}
bool swaps_axes(unsigned orientation){return orientation>=5 && orientation<=8;}
}

unsigned read_cover_orientation_fix28(const CoverImage& image){
    const auto& b=image.encoded;
    if(image.format==CoverFileFormat::JPEG){
        std::size_t p=2;
        while(p<b.size()){
            if(b[p++]!=0xff) break;
            while(p<b.size() && b[p]==0xff) ++p;
            if(p==b.size()) break;
            const auto marker=b[p++];
            if(marker==0xda || marker==0xd9) break;
            if(marker==0x01 || (marker>=0xd0 && marker<=0xd7)) continue;
            if(b.size()-p<2) break;
            const auto length=(unsigned(b[p])<<8)|b[p+1];
            if(length<2 || length>b.size()-p) break;
            if(marker==0xe1 && length>=8 && !std::memcmp(b.data()+p+2,"Exif\0\0",6))
                return tiff_orientation(b.data()+p+8,length-8);
            p+=length;
        }
    }else if(image.format==CoverFileFormat::PNG){
        std::size_t p=8;
        while(p<=b.size() && b.size()-p>=12){
            const auto length=(unsigned(b[p])<<24)|(unsigned(b[p+1])<<16)|(unsigned(b[p+2])<<8)|b[p+3];
            if(length>b.size()-p-12) break;
            if(!std::memcmp(b.data()+p+4,"eXIf",4)) return tiff_orientation(b.data()+p+8,length);
            p+=std::size_t(length)+12;
        }
    }
    return 1;
}

bool orient_cover_rgba_fix28(DecodedImageRGBA& image,unsigned orientation,std::string& error){
    if(!image.valid() || orientation<1 || orientation>8){error="Invalid cover orientation or raster";return false;}
    if(orientation==1) return true;
    DecodedImageRGBA out;
    out.width=swaps_axes(orientation) ? image.height : image.width;
    out.height=swaps_axes(orientation) ? image.width : image.height;
    out.pitch=out.width*4;out.rgba.resize(std::size_t(out.pitch)*out.height);
    for(int y=0;y<image.height;++y) for(int x=0;x<image.width;++x){
        int dx=x,dy=y;
        switch(orientation){
            case 2:dx=image.width-1-x;break;
            case 3:dx=image.width-1-x;dy=image.height-1-y;break;
            case 4:dy=image.height-1-y;break;
            case 5:dx=y;dy=x;break;
            case 6:dx=image.height-1-y;dy=x;break;
            case 7:dx=image.height-1-y;dy=image.width-1-x;break;
            case 8:dx=y;dy=image.width-1-x;break;
        }
        std::memcpy(out.rgba.data()+std::size_t(dy)*out.pitch+dx*4,
                    image.rgba.data()+std::size_t(y)*image.pitch+x*4,4);
    }
    image=std::move(out);return true;
}

bool uses_full_cover_layout_fix28(const CoverImage& image,unsigned manual_orientation){
    if(!image.valid() || image.kind!=GameCoverKind::FullCover) return false;
    int width=image.width,height=image.height;
    if(swaps_axes(read_cover_orientation_fix28(image))) std::swap(width,height);
    if(swaps_axes(manual_orientation)) std::swap(width,height);
    // Full scan artwork includes different amounts of trim and plastic margins.
    // ICON0 and portrait front artwork remain front-only regardless of width.
    const float aspect=height>0 ? float(width)/height : 0;
    return aspect>=1.30f && aspect<=2.70f;
}

const char* cover_orientation_name_fix28(unsigned orientation){
    switch(orientation){
        case 3:return "180 GRAUS";
        case 6:return "90 DIREITA";
        case 8:return "90 ESQUERDA";
        case 2:return "ESPELHO H";
        case 4:return "ESPELHO V";
        default:return "NORMAL";
    }
}

unsigned default_cover_orientation_fix30(const CoverImage& image){
    if(!image.valid() || image.kind!=GameCoverKind::FullCover) return 1;
    int width=image.width,height=image.height;
    if(swaps_axes(read_cover_orientation_fix28(image))) std::swap(width,height);
    if(width<height && width>0 && float(height)/width>=1.30f && float(height)/width<=2.70f) return 6;
    return 1;
}
