#include "orbit_ui_fix30.h"
#include <algorithm>
#include <cmath>
#include <vector>
#include "orbit_font_fix30.inc"
#ifdef PS3_GAME_ORBIT_FIX35
#include "hud_cache_fix35.h"
#endif

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
    // The shipped atlas contains these three sizes. An unsupported size must
    // not recurse forever while looking up its missing fallback glyph.
    if(size!=18 && size!=20 && size!=28) size=20;
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
    else if(symbol=="SQUARE"){line(p,x-8,y-8,x+8,y-8,c,2);line(p,x+8,y-8,x+8,y+8,c,2);line(p,x+8,y+8,x-8,y+8,c,2);line(p,x-8,y+8,x-8,y-8,c,2);}
    else if(symbol=="SELECT") text(p,"SELECT",x-28,y-7,18,Muted,84);
    else if(symbol=="ARROWS"){line(p,x-12,y,x-4,y-5,c);line(p,x-12,y,x-4,y+5,c);line(p,x+12,y,x+4,y-5,c);line(p,x+12,y,x+4,y+5,c);}
    else if(symbol=="UP"){line(p,x-7,y+4,x,y-5,c);line(p,x,y-5,x+7,y+4,c);}
    else text(p,symbol,x-12,y-10,18,c,82);
    text(p,label,x+(symbol=="SELECT" ? 47 : symbol=="START" ? 50 : symbol.size()>6 ? 16 : symbol=="L2/R2" || symbol=="L1/R1" ? 50 : 25),y-10,20,White,270);
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
#ifdef PS3_GAME_ORBIT_FIX35
void clear_rect_fix35(DecodedImageRGBA& p,const HudRectFix35& r){
    for(int y=r.y;y<r.y+r.height;++y)std::fill_n(p.rgba.data()+std::size_t(y)*p.pitch+r.x*4,std::size_t(r.width)*4,0);
}
void draw_list_row_fix35(DecodedImageRGBA& p,const std::vector<std::string>& rows,int i,int active){
    const int y=185+i*35;clear_rect_fix35(p,{62,y-1,566,35});rect(p,62,y-1,566,35,{{16,19,23,205}});
    if(i==active){rect(p,69,y-1,550,33,{{106,141,152,75}});rect(p,69,y-1,3,33,{{180,216,221,230}});}
    text(p,rows[std::size_t(i)],88,y+3,20,i==active ? White : Muted,514);
}
void paint_hud_fix35(DecodedImageRGBA& p,const std::array<std::string,6>& s,const std::vector<std::string>& rows,int active_row,unsigned regions,const GameMenuStateFix35* menu){
    const bool help=s[4].rfind("HELP",0)==0;
    if(regions&1){
    text(p,s[0],64,13,28,White,760);
#ifdef PS3_GAME_ORBIT_FIX31
#ifdef PS3_GAME_ORBIT_FIX36
    text(p,"1.3.3",390,20,18,Muted,100);
#elif defined(PS3_GAME_ORBIT_FIX35)
    text(p,"1.3.2",390,20,18,Muted,100);
#elif defined(PS3_GAME_ORBIT_FIX34)
    text(p,"1.3.1",390,20,18,Muted,100);
#elif defined(PS3_GAME_ORBIT_FIX33)
    text(p,"1.3",390,20,18,Muted,100);
#elif defined(PS3_GAME_ORBIT_FIX32)
    text(p,"1.2",390,20,18,Muted,100);
#else
    text(p,"1.1",390,20,18,Muted,100);
#endif
    text(p,
#ifdef PS3_GAME_ORBIT_FIX32
        s[4].find("LIST")!=std::string::npos ? "Lista" :
#endif
        s[4].find("SPINE")!=std::string::npos ? "Spine" : "Clássico",850,19,20,Muted,180);
#endif
    const int filter_width=measure(s[5],20);
    text(p,"L1 / R1",1136-filter_width,20,18,Muted,120);
    text(p,s[5],1216-filter_width,19,20,White,220);
    }
    if(regions&2){
    // Footer maps atlas row 64 to screen row 576. No opaque status panels.
    text(p,s[1],64,98,28,White,1152);
    text(p,s[3].empty() ? s[2] : s[3],64,128,18,Muted,1152);
    if(menu && menu->open){
        button(p,"X",73,157,"Selecionar",Blue);button(p,"ARROWS",300,157,"Opções");
        button(p,"START",810,157,"Fechar menu");button(p,"O",1123,157,"Fechar",Red);
    }else{
        const bool opened=s[4].find("OPEN")!=std::string::npos;
        const bool mounting=s[4].find("MOUNT")!=std::string::npos;
        button(p,"X",73,157,mounting ? "Montando" : "Montar",Blue);
        if(opened){
            button(p,"L3",300,157,"Fechar");button(p,"START",530,157,"Opções");button(p,"SELECT",820,157,"Ajuda");
        }else{
            button(p,"ARROWS",210,157,"Jogos");button(p,"TRIANGLE",328,157,"Favorito",Green);
            button(p,"L3",463,157,"Abrir");button(p,"SQUARE",580,157,"Layout",{{211,169,203,255}});
            button(p,"START",730,157,"Opções");button(p,"SELECT",923,157,"Ajuda");
        }
        button(p,"O",1123,157,mounting ? "Cancelar" : opened ? "Fechar" : "Sair",Red);
    }
    }
    if(regions&4){
#ifdef PS3_GAME_ORBIT_FIX32
    if(!help && !(menu && menu->open) && s[4].find("LIST")!=std::string::npos && s[4].find("OPEN")==std::string::npos) {
        rect(p,62,177,566,333,{{16,19,23,205}});
        if(rows.empty()) text(p,"Nenhum jogo neste filtro",86,223,20,Muted,510);
        for(std::size_t i=0;i<rows.size();++i) draw_list_row_fix35(p,rows,int(i),active_row);
    }
#endif
    if(help){
#if defined(PS3_GAME_ORBIT_FIX34) && !defined(PS3_GAME_ORBIT_FIX35)
        rect(p,610,178,606,334,{{18,20,23,240}});
        text(p,"Imagens e cache",630,188,28,White,566);
        text(p,"/dev_hdd0/PS3COVERS",630,229,18,Muted,566);
        text(p,"ISO: nome exato do arquivo, sem .iso",630,257,18,White,566);
        text(p,"Ex.: Bayonetta.iso: Bayonetta.jpg",630,285,18,White,566);
        text(p,"Inside: Bayonetta_INSIDE.jpg",630,313,18,White,566);
        text(p,"Disco: Bayonetta_DISC.png",630,341,18,White,566);
        text(p,"JPG / JPEG / PNG",630,369,18,White,566);
        text(p,"Cache: 15 capas, 15 inside, 15 discos",630,397,18,White,566);
        text(p,"Feche a caixa e use START após copiar",630,425,18,White,566);
        text(p,"ID no nome do ISO: BLES01287 (sem hífen)",630,453,18,Muted,566);
        text(p,"A ID interna do ISO ainda não é lida",630,481,18,Muted,566);
#endif
#ifndef PS3_GAME_ORBIT_FIX32
        rect(p,54,178,400,206,{{18,20,23,240}});
        text(p,"Controles",74,188,28,White,350);
        text(p,"Analógico direito   Girar a caixa",74,229,18,White,356);
        text(p,"L2 / R2   Aproximar / afastar",74,257,18,White,356);
        text(p,"R3   Restaurar posição e zoom",74,285,18,White,356);
#endif
#ifdef PS3_GAME_ORBIT_FIX31
#ifdef PS3_GAME_ORBIT_FIX32
        rect(p,54,178,530,334,{{18,20,23,240}});
        text(p,"Controles",74,188,28,White,480);
        text(p,"Analógico direito   Girar a caixa",74,229,18,White,480);
#ifdef PS3_GAME_ORBIT_FIX33
        text(p,"L2 / R2   Zoom    R3   Restaurar",74,257,18,White,480);
        text(p,"Quadrado   Clássico / Spine / Lista",74,285,18,White,480);
        text(p,"L3   Abrir / fechar caixa e disco",74,313,18,White,480);
        text(p,"X   Montar com caixa aberta ou fechada",74,341,18,White,480);
#else
        text(p,"L2 / R2   Aproximar / afastar",74,257,18,White,480);
        text(p,"R3   Restaurar posição e zoom",74,285,18,White,480);
        text(p,"Quadrado   Clássico / Spine / Lista",74,313,18,White,480);
        text(p,"L3   Abrir / fechar caixa e disco",74,341,18,White,480);
#endif
        text(p,"Lista   Cima / baixo: navegar",74,369,18,White,480);
        text(p,"Cima   Autogiro no Clássico / Spine",74,397,18,White,480);
#ifdef PS3_GAME_ORBIT_FIX35
        text(p,"START   Opções do jogo",74,425,18,White,480);
#else
        text(p,"START   Atualizar biblioteca",74,425,18,White,480);
#endif
#ifdef PS3_GAME_ORBIT_FIX33
        text(p,"Círculo   Fechar / cancelar / sair",74,453,18,White,480);
#else
        text(p,"Círculo   Voltar / sair",74,453,18,White,480);
#endif
        text(p,"SELECT   Fechar ajuda",74,484,18,Muted,480);
#else
        text(p,"Quadrado   Clássico / Spine",74,313,18,White,356);
#endif
#else
        text(p,"L1 / R1   Alterar filtro",74,313,18,White,356);
#endif
#ifndef PS3_GAME_ORBIT_FIX32
        text(p,"START   Atualizar biblioteca",74,341,18,White,356);
        text(p,"SELECT   Fechar ajuda",74,366,18,Muted,356);
#endif
    }
    if(menu && menu->open){
        rect(p,370,185,540,307,{{18,20,23,242}});
        text(p,"Opções do jogo",394,200,28,White,492);
        text(p,s[1],394,242,20,Muted,492);
#ifdef PS3_GAME_ORBIT_FIX36
        const char* options[]={"Alterar nome do jogo","Recarregar capas","Copiar capas do pendrive",menu->animated_background?"Fundo animado: Ligado":"Fundo animado: Desligado","Atualizar biblioteca"};
        constexpr int Count=5,Spacing=31;
#else
        constexpr const char* options[]={"Alterar nome do jogo","Recarregar capas","Copiar capas do pendrive","Atualizar biblioteca"};
        constexpr int Count=4,Spacing=35;
#endif
        for(int i=0;i<Count;++i){const int y=283+i*Spacing;
            if(i==menu->selected){rect(p,382,y-3,516,33,{{106,141,152,80}});rect(p,382,y-3,3,33,Blue);}
            text(p,options[i],402,y,20,i==menu->selected ? White : Muted,474);
        }
        text(p,menu->status.empty() ? "X selecionar   Círculo voltar" : menu->status,394,448,18,Muted,492);
    }
    }
}
#endif
DecodedImageRGBA hud(const std::array<std::string,6>& s
#ifdef PS3_GAME_ORBIT_FIX32
    ,const std::vector<std::string>& rows,int active_row
#endif
){
    auto p=bitmap(Width,HudHeight);
#ifdef PS3_GAME_ORBIT_FIX35
    paint_hud_fix35(p,s,rows,active_row,7,nullptr);return p;
#endif
    text(p,s[0],64,13,28,White,760);
#ifdef PS3_GAME_ORBIT_FIX31
#ifdef PS3_GAME_ORBIT_FIX36
    text(p,"1.3.3",390,20,18,Muted,100);
#elif defined(PS3_GAME_ORBIT_FIX35)
    text(p,"1.3.2",390,20,18,Muted,100);
#elif defined(PS3_GAME_ORBIT_FIX34)
    text(p,"1.3.1",390,20,18,Muted,100);
#elif defined(PS3_GAME_ORBIT_FIX33)
    text(p,"1.3",390,20,18,Muted,100);
#elif defined(PS3_GAME_ORBIT_FIX32)
    text(p,"1.2",390,20,18,Muted,100);
#else
    text(p,"1.1",390,20,18,Muted,100);
#endif
    text(p,
#ifdef PS3_GAME_ORBIT_FIX32
        s[4].find("LIST")!=std::string::npos ? "Lista" :
#endif
        s[4].find("SPINE")!=std::string::npos ? "Spine" : "Clássico",850,19,20,Muted,180);
#endif
    const int filter_width=measure(s[5],20);
    text(p,"L1 / R1",1136-filter_width,20,18,Muted,120);
    text(p,s[5],1216-filter_width,19,20,White,220);
    // Footer maps atlas row 64 to screen row 576. No opaque status panels.
    text(p,s[1],64,98,28,White,1152);
    text(p,s[3].empty() ? s[2] : s[3],64,128,18,Muted,1152);
    button(p,"X",73,157,
#ifdef PS3_GAME_ORBIT_FIX33
        s[4].find("MOUNT")!=std::string::npos ? "Montando" :
#elif defined(PS3_GAME_ORBIT_FIX32)
        s[4].find("OPEN")!=std::string::npos ? "Voltar" :
#endif
        "Montar",Blue);
#ifdef PS3_GAME_ORBIT_FIX31
    button(p,"ARROWS",230,157,"Jogos");
    button(p,"TRIANGLE",365,157,"Favorito",Green);
#ifdef PS3_GAME_ORBIT_FIX32
    button(p,"L3",547,157,s[4].find("OPEN")!=std::string::npos ? "Fechar" : "Abrir");
#else
    button(p,"UP",547,157,"Autogiro");
#endif
    button(p,"SQUARE",725,157,"Layout",{{211,169,203,255}});
    button(p,"SELECT",922,157,"Ajuda");
    button(p,"O",1123,157,
#ifdef PS3_GAME_ORBIT_FIX33
        s[4].find("MOUNT")!=std::string::npos ? "Cancelar" :
        s[4].find("OPEN")!=std::string::npos ? "Fechar" :
#elif defined(PS3_GAME_ORBIT_FIX32)
        s[4].find("OPEN")!=std::string::npos ? "Voltar" :
#endif
        "Sair",Red);
    const bool help=s[4].rfind("HELP",0)==0;
#else
    button(p,"ARROWS",230,157,"Jogos");
    button(p,"TRIANGLE",387,157,"Favorito",Green);
    button(p,"UP",596,157,"Autogiro");
    button(p,"SELECT",814,157,"Ajuda");
    button(p,"O",1072,157,"Sair",Red);
    const bool help=s[4]=="HELP";
#endif
#ifdef PS3_GAME_ORBIT_FIX32
    if(!help && s[4].find("LIST")!=std::string::npos && s[4].find("OPEN")==std::string::npos) {
        rect(p,62,177,566,333,{{16,19,23,205}});
        if(rows.empty()) text(p,"Nenhum jogo neste filtro",86,223,20,Muted,510);
        for(std::size_t i=0;i<rows.size();++i) {
            const int y=185+int(i)*35;
            if(int(i)==active_row) {
                rect(p,69,y-1,550,33,{{106,141,152,75}});
                rect(p,69,y-1,3,33,{{180,216,221,230}});
            }
            text(p,rows[i],88,y+3,20,int(i)==active_row ? White : Muted,514);
        }
    }
#endif
    if(help){
#if defined(PS3_GAME_ORBIT_FIX34) && !defined(PS3_GAME_ORBIT_FIX35)
        rect(p,610,178,606,334,{{18,20,23,240}});
        text(p,"Imagens e cache",630,188,28,White,566);
        text(p,"/dev_hdd0/PS3COVERS",630,229,18,Muted,566);
        text(p,"ISO: nome exato do arquivo, sem .iso",630,257,18,White,566);
        text(p,"Ex.: Bayonetta.iso: Bayonetta.jpg",630,285,18,White,566);
        text(p,"Inside: Bayonetta_INSIDE.jpg",630,313,18,White,566);
        text(p,"Disco: Bayonetta_DISC.png",630,341,18,White,566);
        text(p,"JPG / JPEG / PNG",630,369,18,White,566);
        text(p,"Cache: 15 capas, 15 inside, 15 discos",630,397,18,White,566);
        text(p,"Feche a caixa e use START após copiar",630,425,18,White,566);
        text(p,"ID no nome do ISO: BLES01287 (sem hífen)",630,453,18,Muted,566);
        text(p,"A ID interna do ISO ainda não é lida",630,481,18,Muted,566);
#endif
#ifndef PS3_GAME_ORBIT_FIX32
        rect(p,54,178,400,206,{{18,20,23,240}});
        text(p,"Controles",74,188,28,White,350);
        text(p,"Analógico direito   Girar a caixa",74,229,18,White,356);
        text(p,"L2 / R2   Aproximar / afastar",74,257,18,White,356);
        text(p,"R3   Restaurar posição e zoom",74,285,18,White,356);
#endif
#ifdef PS3_GAME_ORBIT_FIX31
#ifdef PS3_GAME_ORBIT_FIX32
        rect(p,54,178,530,334,{{18,20,23,240}});
        text(p,"Controles",74,188,28,White,480);
        text(p,"Analógico direito   Girar a caixa",74,229,18,White,480);
#ifdef PS3_GAME_ORBIT_FIX33
        text(p,"L2 / R2   Zoom    R3   Restaurar",74,257,18,White,480);
        text(p,"Quadrado   Clássico / Spine / Lista",74,285,18,White,480);
        text(p,"L3   Abrir / fechar caixa e disco",74,313,18,White,480);
        text(p,"X   Montar com caixa aberta ou fechada",74,341,18,White,480);
#else
        text(p,"L2 / R2   Aproximar / afastar",74,257,18,White,480);
        text(p,"R3   Restaurar posição e zoom",74,285,18,White,480);
        text(p,"Quadrado   Clássico / Spine / Lista",74,313,18,White,480);
        text(p,"L3   Abrir / fechar caixa e disco",74,341,18,White,480);
#endif
        text(p,"Lista   Cima / baixo: navegar",74,369,18,White,480);
        text(p,"Cima   Autogiro no Clássico / Spine",74,397,18,White,480);
#ifdef PS3_GAME_ORBIT_FIX35
        text(p,"START   Opções do jogo",74,425,18,White,480);
#else
        text(p,"START   Atualizar biblioteca",74,425,18,White,480);
#endif
#ifdef PS3_GAME_ORBIT_FIX33
        text(p,"Círculo   Fechar / cancelar / sair",74,453,18,White,480);
#else
        text(p,"Círculo   Voltar / sair",74,453,18,White,480);
#endif
        text(p,"SELECT   Fechar ajuda",74,484,18,Muted,480);
#else
        text(p,"Quadrado   Clássico / Spine",74,313,18,White,356);
#endif
#else
        text(p,"L1 / R1   Alterar filtro",74,313,18,White,356);
#endif
#ifndef PS3_GAME_ORBIT_FIX32
        text(p,"START   Atualizar biblioteca",74,341,18,White,356);
        text(p,"SELECT   Fechar ajuda",74,366,18,Muted,356);
#endif
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
#ifdef PS3_GAME_ORBIT_FIX35
#ifdef PS3_GAME_ORBIT_FIX36
    const std::string credit="v1.3.3  |  Criado por SpeedVash";
#else
    const std::string credit="v1.3.2  |  Criado por SpeedVash";
#endif
    text(p,credit,(Width-measure(credit,20))/2,615,20,White,900);
#endif
    const std::string status="Carregando sua biblioteca...";
    text(p,status,(Width-measure(status,20))/2,660,20,White,760);
    return p;
}
std::array<V14Vertex,12> hud_vertices(){
#ifdef PS3_GAME_ORBIT_FIX32
    return {{vertex(0,24,0,0),vertex(1280,24,1,0),vertex(1280,88,1,64.0f/512),vertex(0,88,0,64.0f/512),
             vertex(0,576,0,64.0f/512),vertex(1280,576,1,64.0f/512),vertex(1280,688,1,176.0f/512),vertex(0,688,0,176.0f/512),
             vertex(0,180,0,176.0f/512),vertex(1280,180,1,176.0f/512),vertex(1280,516,1,1),vertex(0,516,0,1)}};
#else
    return {{vertex(0,24,0,0),vertex(1280,24,1,0),vertex(1280,88,1,64.0f/384),vertex(0,88,0,64.0f/384),
             vertex(0,576,0,64.0f/384),vertex(1280,576,1,64.0f/384),vertex(1280,688,1,176.0f/384),vertex(0,688,0,176.0f/384),
             vertex(0,180,0,176.0f/384),vertex(1280,180,1,176.0f/384),vertex(1280,388,1,1),vertex(0,388,0,1)}};
#endif
}
std::array<V14Vertex,4> background_vertices(){return {{vertex(0,0,0,0),vertex(1280,0,1,0),vertex(1280,720,1,1),vertex(0,720,0,1)}};}
}

#ifdef PS3_GAME_ORBIT_FIX35
std::vector<HudRectFix35> HudCacheFix35::update(const std::array<std::string,6>& lines,const std::vector<std::string>& rows,int active,const GameMenuStateFix35& menu){
    const auto layout=[](const auto& s){return s.find("LIST")!=std::string::npos ? 2 : s.find("SPINE")!=std::string::npos ? 1 : 0;};
    const auto mode=[&](const auto& s,const auto& m){return m.open ? 3 : s.rfind("HELP",0)==0 ? 2 : layout(s)==2 && s.find("OPEN")==std::string::npos ? 1 : 0;};
    const auto controls=[](const auto& s){return std::make_pair(s.find("OPEN")!=std::string::npos,s.find("MOUNT")!=std::string::npos);};
    std::vector<HudRectFix35> dirty;unsigned regions=0;
    if(!image_.valid()){
        image_.width=1280;image_.height=512;image_.pitch=5120;image_.rgba.resize(std::size_t(image_.pitch)*512);
        regions=7;dirty.push_back({0,0,1280,512});
    }else{
        if(lines[0]!=lines_[0] || lines[5]!=lines_[5] || layout(lines[4])!=layout(lines_[4])){regions|=1;dirty.push_back({0,0,1280,64});}
        if(lines[1]!=lines_[1] || lines[2]!=lines_[2] || lines[3]!=lines_[3] || controls(lines[4])!=controls(lines_[4]) || menu.open!=menu_.open){regions|=2;dirty.push_back({0,64,1280,112});}
        const int old_mode=mode(lines_[4],menu_),new_mode=mode(lines[4],menu);
        if(old_mode!=new_mode || (new_mode==3 && (menu!=menu_ || lines[1]!=lines_[1])) || (new_mode==1 && (rows.size()!=rows_.size()))){regions|=4;dirty.push_back({0,176,1280,336});}
        else if(new_mode==1){
            for(std::size_t i=0;i<rows.size();++i)if(rows[i]!=rows_[i] || ((int(i)==active)!=(int(i)==active_))){
                const HudRectFix35 r{62,184+int(i)*35,566,35};dirty.push_back(r);OrbitUiFix30::draw_list_row_fix35(image_,rows,int(i),active);
            }
        }
    }
    if(regions&1)OrbitUiFix30::clear_rect_fix35(image_,{0,0,1280,64});
    if(regions&2)OrbitUiFix30::clear_rect_fix35(image_,{0,64,1280,112});
    if(regions&4)OrbitUiFix30::clear_rect_fix35(image_,{0,176,1280,336});
    if(regions)OrbitUiFix30::paint_hud_fix35(image_,lines,rows,active,regions,&menu);
    lines_=lines;rows_=rows;active_=active;menu_=menu;last_changed_bytes_=0;
    for(const auto& r:dirty)last_changed_bytes_+=std::size_t(r.width)*r.height*4;
    return dirty;
}
#endif
