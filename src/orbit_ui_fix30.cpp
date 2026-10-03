#include "orbit_ui_fix30.h"
#include <algorithm>
#include <cmath>
#include <vector>
#include "orbit_font_fix30.inc"

namespace {
using Color=std::array<unsigned char,4>;
constexpr Color White{{239,242,245,255}},Muted{{180,185,191,255}},Blue{{156,198,230,255}},Green{{151,211,179,255}},Red{{229,159,167,255}};
DecodedImageRGBA bitmap(int w,int h){
    DecodedImageRGBA p;p.width=w;p.height=h;p.pitch=w*4;p.rgba.resize(std::size_t(p.pitch)*h);return p;
}
void pixel(DecodedImageRGBA& p,int x,int y,Color c,unsigned coverage=255){
    if(x<0 || y<0 || x>=p.width || y>=p.height) return;
    auto* q=p.rgba.data()+std::size_t(y)*p.pitch+x*4;
    const unsigned a=(unsigned(c[3])*coverage+127)/255,old=q[3];
    const unsigned result=a+(old*(255-a)+127)/255;
    if(!result) return;
    for(int i=0;i<3;++i) q[i]=static_cast<unsigned char>((unsigned(c[i])*a+(unsigned(q[i])*old*(255-a)+127)/255+result/2)/result);
    q[3]=static_cast<unsigned char>(result);
}
std::vector<unsigned> codepoints(const std::string& s){
    std::vector<unsigned> out;
    for(std::size_t i=0;i<s.size() && out.size()<4096;){
        unsigned a=static_cast<unsigned char>(s[i++]),cp=a,n=0,minimum=0;
        if(a>=0xc2 && a<=0xdf){cp=a&31;n=1;minimum=128;}
        else if(a>=0xe0 && a<=0xef){cp=a&15;n=2;minimum=2048;}
        else if(a>=0xf0 && a<=0xf4){cp=a&7;n=3;minimum=65536;}
        else if(a>=128){out.push_back('?');continue;}
        bool valid=i+n<=s.size();
        for(unsigned j=0;j<n && valid;++j){const unsigned b=static_cast<unsigned char>(s[i]);if((b&0xc0)!=0x80) valid=false;else{++i;cp=(cp<<6)|(b&63);}}
        if(!valid || cp<minimum || cp>0x10ffff || (cp>=0xd800 && cp<=0xdfff)) cp='?';
        if(cp<32 || cp==127) cp=' ';
        out.push_back(cp);
    }
    return out;
}
const OrbitGlyph& glyph(unsigned code,unsigned size){
    const auto* first=std::begin(OrbitGlyphs);const auto* end=std::end(OrbitGlyphs);
    const auto* p=std::lower_bound(first,end,std::pair<unsigned,unsigned>{size,code},[](const OrbitGlyph& g,const auto& value){return g.size<value.first || (g.size==value.first && g.code<value.second);});
    if(p!=end && p->size==size && p->code==code) return *p;
    return glyph('?',size);
}
int measure(const std::string& s,unsigned size){int width=0;for(auto cp:codepoints(s)) width+=glyph(cp,size).advance;return width;}
void text(DecodedImageRGBA& p,const std::string& s,int x,int y,unsigned size,Color color,int limit){
    auto cps=codepoints(s);int width=0;for(auto cp:cps) width+=glyph(cp,size).advance;
    if(width>limit){
        const int dots=glyph(0x2026,size).advance;
        while(!cps.empty() && width+dots>limit){width-=glyph(cps.back(),size).advance;cps.pop_back();}
        cps.push_back(0x2026);
    }
    const int stop=x+limit;
    for(auto cp:cps){const auto& g=glyph(cp,size);
        for(unsigned yy=0;yy<g.height;++yy) for(unsigned xx=0;xx<g.width;++xx){
            const int px=x+g.x+int(xx);if(px>=stop) continue;
            pixel(p,px,y+g.y+int(yy),color,OrbitGlyphPixels[g.offset+yy*g.width+xx]);
        }
        x+=g.advance;
    }
}
void line(DecodedImageRGBA& p,float ax,float ay,float bx,float by,Color c,float thickness=1.6f){
    const float dx=bx-ax,dy=by-ay,len=dx*dx+dy*dy;
    for(int y=int(std::floor(std::min(ay,by)-thickness-1));y<=int(std::ceil(std::max(ay,by)+thickness+1));++y)
    for(int x=int(std::floor(std::min(ax,bx)-thickness-1));x<=int(std::ceil(std::max(ax,bx)+thickness+1));++x){
        const float t=len>0 ? std::clamp(((x+.5f-ax)*dx+(y+.5f-ay)*dy)/len,0.0f,1.0f) : 0;
        const float dist=std::hypot(x+.5f-ax-t*dx,y+.5f-ay-t*dy);
        const float v=std::clamp(thickness*.5f+.65f-dist,0.0f,1.0f);if(v>0) pixel(p,x,y,c,unsigned(v*255));
    }
}
void circle(DecodedImageRGBA& p,float cx,float cy,float r,Color c){
    for(int y=int(cy-r-2);y<=int(cy+r+2);++y) for(int x=int(cx-r-2);x<=int(cx+r+2);++x){
        const float d=std::fabs(std::hypot(x+.5f-cx,y+.5f-cy)-r),v=std::clamp(1.5f-d,0.0f,1.0f);
        if(v>0) pixel(p,x,y,c,unsigned(v*255));
    }
}
void button(DecodedImageRGBA& p,const std::string& symbol,int x,int y,const std::string& label,Color c=White){
    if(symbol=="X"){line(p,x-7,y-7,x+7,y+7,c,2);line(p,x+7,y-7,x-7,y+7,c,2);}
    else if(symbol=="O") circle(p,float(x),float(y),9,c);
    else if(symbol=="TRIANGLE"){line(p,x,y-10,x+10,y+8,c,2);line(p,x+10,y+8,x-10,y+8,c,2);line(p,x-10,y+8,x,y-10,c,2);}
    else if(symbol=="SELECT") text(p,"SELECT",x-28,y-7,18,Muted,84);
    else if(symbol=="ARROWS"){line(p,x-12,y,x-4,y-5,c);line(p,x-12,y,x-4,y+5,c);line(p,x+12,y,x+4,y-5,c);line(p,x+12,y,x+4,y+5,c);}
    else if(symbol=="UP"){line(p,x-7,y+4,x,y-5,c);line(p,x,y-5,x+7,y+4,c);}
    else text(p,symbol,x-12,y-10,18,c,82);
    text(p,label,x+(symbol=="SELECT" ? 47 : symbol.size()>6 ? 16 : symbol=="L2/R2" || symbol=="L1/R1" ? 50 : 25),y-10,20,White,270);
}
void rect(DecodedImageRGBA& p,int x,int y,int w,int h,Color c){for(int yy=y;yy<y+h;++yy) for(int xx=x;xx<x+w;++xx) pixel(p,xx,yy,c);}
constexpr float Nx=-.352208f,Ny=.553469f,Nz=.754730f;
V14Vertex vertex(int x,int y,float u,float v){return {2.0f*x/1280-1,1-2.0f*y/720,0,Nx,Ny,Nz,u,v};}
}

namespace OrbitUiFix30 {
DecodedImageRGBA background(){
    auto p=bitmap(Width,BackgroundHeight);
    for(int y=0;y<p.height;++y) for(int x=0;x<p.width;++x){
        const float dx=(x-520)/1000.0f,dy=(y-145)/430.0f;
        const int value=int(16+9*std::max(0.0f,1-dx*dx-dy*dy)+3*(1-float(y)/p.height));
        const auto k=std::size_t(y)*p.pitch+x*4;
        p.rgba[k]=p.rgba[k+1]=p.rgba[k+2]=static_cast<unsigned char>(value);p.rgba[k+3]=255;
    }
    for(int wave=0;wave<5;++wave) for(int x=0;x<p.width;++x){
        const float phase=float(x)/p.width;
        const float cy=249+15*wave+25*std::sin(phase*6.2f+.43f*wave)+11*std::sin(phase*3.4f+1.7f*wave);
        for(int y=int(cy)-8;y<=int(cy)+8;++y){
            const float dist=std::fabs(y+.5f-cy);
            const unsigned a=unsigned(std::max(0.0f,1-dist/8)*10+std::max(0.0f,1-dist/1.3f)*55);
            pixel(p,x,y,{{209,214,218,static_cast<unsigned char>(a)}});
        }
    }
    return p;
}
DecodedImageRGBA hud(const std::array<std::string,6>& s){
    auto p=bitmap(Width,HudHeight);
    text(p,s[0],64,13,28,White,760);
    const int filter_width=measure(s[5],20);
    text(p,"L1 / R1",1136-filter_width,20,18,Muted,120);
    text(p,s[5],1216-filter_width,19,20,White,220);
    // Footer maps atlas row 64 to screen row 576. No opaque status panels.
    text(p,s[1],64,98,28,White,1152);
    text(p,s[3].empty() ? s[2] : s[3],64,128,18,Muted,1152);
    button(p,"X",73,157,"Montar",Blue);
    button(p,"ARROWS",230,157,"Jogos");
    button(p,"TRIANGLE",387,157,"Favorito",Green);
    button(p,"UP",596,157,"Autogiro");
    button(p,"SELECT",814,157,"Ajuda");
    button(p,"O",1072,157,"Sair",Red);
    if(s[4]=="HELP"){
        rect(p,54,178,400,206,{{18,20,23,240}});
        text(p,"Controles",74,188,28,White,350);
        text(p,"Analógico direito   Girar a caixa",74,229,18,White,356);
        text(p,"L2 / R2   Aproximar / afastar",74,257,18,White,356);
        text(p,"R3   Restaurar posição e zoom",74,285,18,White,356);
        text(p,"L1 / R1   Alterar filtro",74,313,18,White,356);
        text(p,"START   Atualizar biblioteca",74,341,18,White,356);
        text(p,"SELECT   Fechar ajuda",74,366,18,Muted,356);
    }
    return p;
}
DecodedImageRGBA splash(const DecodedImageRGBA* artwork){
    auto p=bitmap(Width,720);
    const auto bg=background();
    for(int y=0;y<720;++y) for(int x=0;x<Width;++x){
        const auto src=std::size_t(y/2)*bg.pitch+x*4, dst=std::size_t(y)*p.pitch+x*4;
        std::copy_n(bg.rgba.data()+src,4,p.rgba.data()+dst);
    }
    if(artwork && artwork->valid()){
        for(int y=0;y<720;++y) for(int x=0;x<Width;++x){
            const int sx=x*artwork->width/Width,sy=y*artwork->height/720;
            const auto* c=artwork->rgba.data()+std::size_t(sy)*artwork->pitch+sx*4;
            pixel(p,x,y,{{c[0],c[1],c[2],c[3]}});
        }
    }else text(p,"PS3 Game Orbit",(Width-measure("PS3 Game Orbit",28))/2,315,28,White,800);
    const std::string status="Carregando sua biblioteca...";
    text(p,status,(Width-measure(status,20))/2,660,20,White,760);
    return p;
}
std::array<V14Vertex,12> hud_vertices(){
    return {{vertex(0,24,0,0),vertex(1280,24,1,0),vertex(1280,88,1,64.0f/384),vertex(0,88,0,64.0f/384),
             vertex(0,576,0,64.0f/384),vertex(1280,576,1,64.0f/384),vertex(1280,688,1,176.0f/384),vertex(0,688,0,176.0f/384),
             vertex(0,180,0,176.0f/384),vertex(1280,180,1,176.0f/384),vertex(1280,388,1,1),vertex(0,388,0,1)}};
}
std::array<V14Vertex,4> background_vertices(){return {{vertex(0,0,0,0),vertex(1280,0,1,0),vertex(1280,720,1,1),vertex(0,720,0,1)}};}
}
