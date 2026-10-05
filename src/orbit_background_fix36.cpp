#include "orbit_background_fix36.h"
#ifdef PS3_GAME_ORBIT_FIX36
#include <algorithm>
#include <cmath>
namespace OrbitBackgroundFix36 {
namespace {
DecodedImageRGBA bitmap(int w,int h){DecodedImageRGBA p;p.width=w;p.height=h;p.pitch=w*4;p.rgba.resize(std::size_t(p.pitch)*h);return p;}
V14Vertex point(float u,float v,float t,unsigned layer){
    const float center=.655f-u*.13f+.088f*std::sin(u*5.4f-t*.22f+layer*.61f)+.026f*std::sin(u*10.4f+t*.1f+layer);
    const float thickness=(.081f+.019f*std::sin(u*4.8f+t*.12f+layer))*std::sin(u*4.9f-t*.14f+.45f+layer*.72f);
    const float x=u+.008f*v*std::sin(u*5.5f-t*.1f),y=center+v*thickness+(float(layer)-1)*.018f;
    return {2*x-1,1-2*y,0,-.352208f,.553469f,.754730f,(u+.08f)/1.16f,(v+1)*.5f};
}
}
void update(Mesh& mesh,float phase){
    if(mesh.vertices.size()!=Ribbons*VerticesPerRibbon)return;
    for(unsigned layer=0;layer<Ribbons;++layer)for(unsigned i=0;i<=Segments;++i){
        const float u=-.08f+1.16f*i/Segments;const auto k=layer*VerticesPerRibbon+i*2;
        mesh.vertices[k]=point(u,-1,phase,layer);mesh.vertices[k+1]=point(u,1,phase,layer);
    }
}
Mesh build(float phase){
    Mesh mesh;mesh.vertices.resize(Ribbons*VerticesPerRibbon);mesh.indices.reserve(Ribbons*IndicesPerRibbon);
    for(unsigned layer=0;layer<Ribbons;++layer)for(unsigned i=0;i<Segments;++i){
        const auto a=std::uint16_t(layer*VerticesPerRibbon+i*2),b=std::uint16_t(a+1),c=std::uint16_t(a+2),d=std::uint16_t(a+3);
        for(const auto index:{a,b,c,c,b,d})mesh.indices.push_back(index);
    }
    update(mesh,phase);return mesh;
}
DecodedImageRGBA gradient(){
    auto p=bitmap(1280,360);
    for(int y=0;y<p.height;++y)for(int x=0;x<p.width;++x){
        const float u=float(x)/p.width,v=float(y)/p.height,dx=(u-.44f)/.68f,dy=(v-.4f)/.85f;
        const float base=v<.5f?23+18*v:32-48*(v-.5f),glow=5*std::max(0.f,1-dx*dx-dy*dy);
        const int value=int((base+glow)*std::max(0.f,1-.45f*((u-.5f)*(u-.5f)+(v-.5f)*(v-.5f))));
        const auto k=std::size_t(y)*p.pitch+x*4;
        p.rgba[k]=std::uint8_t(std::clamp(value-2,0,255));p.rgba[k+1]=std::uint8_t(value);p.rgba[k+2]=std::uint8_t(value+3);p.rgba[k+3]=255;
    }
    return p;
}
DecodedImageRGBA ribbon_texture(){
    auto p=bitmap(256,128);
    for(int y=0;y<p.height;++y)for(int x=0;x<p.width;++x){
        const float edge=std::min(float(y),float(p.height-1-y));
        const float alpha=(.018f+.31f*std::exp(-edge*.38f)+(y%4==0?.073f:.007f)+(x%5==0?.012f:0))*(.48f+.52f*std::sin(float(x)*3.14159265f/(p.width-1)));
        const auto k=std::size_t(y)*p.pitch+x*4;p.rgba[k]=236;p.rgba[k+1]=240;p.rgba[k+2]=244;p.rgba[k+3]=std::uint8_t(std::clamp(alpha*255,0.f,255.f));
    }
    return p;
}
}
#endif
