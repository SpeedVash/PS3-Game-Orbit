#include "library_hud_fix28.h"
#include "font_bitmap_fix28.h"
#include <algorithm>

#ifndef PS3_GAME_ORBIT_FIX30
namespace {
unsigned next_codepoint(const std::string& text,std::size_t& pos){
    const unsigned a=static_cast<unsigned char>(text[pos++]);
    if(a<128) return a;
    unsigned value=0,count=0,minimum=0;
    if(a>=0xc2 && a<=0xdf){value=a&31;count=1;minimum=0x80;}
    else if(a>=0xe0 && a<=0xef){value=a&15;count=2;minimum=0x800;}
    else if(a>=0xf0 && a<=0xf4){value=a&7;count=3;minimum=0x10000;}
    else return '?';
    if(pos+count>text.size()){pos=text.size();return '?';}
    for(unsigned i=0;i<count;++i){
        const unsigned b=static_cast<unsigned char>(text[pos]);
        if((b&0xc0)!=0x80) return '?';
        ++pos;value=(value<<6)|(b&63);
    }
    if(value<minimum || value>0x10ffff || (value>=0xd800 && value<=0xdfff)) return '?';
    return value<256 ? value : '?';
}
}
#endif
DecodedImageRGBA rasterize_library_hud_fix28(const LibraryHudLinesFix28& lines){
#ifdef PS3_GAME_ORBIT_FIX30
    return OrbitUiFix30::hud(lines);
#else
    DecodedImageRGBA image;image.width=1024;image.height=192;image.pitch=image.width*4;
    image.rgba.resize(std::size_t(image.pitch)*image.height);
    for(std::size_t p=0;p<image.rgba.size();p+=4){
        image.rgba[p]=20;image.rgba[p+1]=20;image.rgba[p+2]=20;image.rgba[p+3]=220;
    }
    for(unsigned row=0;row<6;++row){
        std::size_t pos=0;unsigned count=0;
        const auto& text=lines[row];
        std::array<unsigned,64> codes{};
        while(pos<text.size() && count<codes.size()) codes[count++]=next_codepoint(text,pos);
        if(count>63){count=63;codes[60]=codes[61]=codes[62]='.';}
        for(unsigned col=0;col<count;++col){
            unsigned code=codes[col];
            if(code<32 || code==127) code=' ';
            for(unsigned y=0;y<16;++y) for(unsigned x=0;x<8;++x){
                if(!(FontBitmapFix28[code][y] & (1u<<(7-x)))) continue;
                for(unsigned sy=0;sy<2;++sy) for(unsigned sx=0;sx<2;++sx){
                    const auto p=std::size_t(row*32+y*2+sy)*image.pitch+(8+col*16+x*2+sx)*4;
                    image.rgba[p]=image.rgba[p+1]=image.rgba[p+2]=row==0 ? 208 : 244;
                    image.rgba[p+3]=255;
                }
            }
        }
    }
    return image;
#endif
}
#ifdef PS3_GAME_ORBIT_FIX30
std::array<V14Vertex,12> library_hud_vertices_fix28(){return OrbitUiFix30::hud_vertices();}
#else
std::array<V14Vertex,8> library_hud_vertices_fix28(){
    // Same lighting direction as the frozen shader: the HUD normal gives unit
    // brightness without modifying the shader or any normals of the case.
    constexpr float nx=-0.352208f,ny=0.553469f,nz=0.754730f;
    return {{{-0.9375f,0.95f,0,nx,ny,nz,0,0},
             {0.9375f,0.95f,0,nx,ny,nz,1,0},
             {0.9375f,0.73f,0,nx,ny,nz,1,0.5f},
             {-0.9375f,0.73f,0,nx,ny,nz,0,0.5f},
             {-0.9375f,-0.73f,0,nx,ny,nz,0,0.5f},
             {0.9375f,-0.73f,0,nx,ny,nz,1,0.5f},
             {0.9375f,-0.95f,0,nx,ny,nz,1,1},
             {-0.9375f,-0.95f,0,nx,ny,nz,0,1}}};
}
#endif
