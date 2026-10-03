#include "jfx_case_fix29.h"
#include <algorithm>
#include <cmath>
#include <iterator>
#include <utility>
#include "jfx_case_data_fix29.inc"

bool JfxCaseFix29::surface_point(const V14MeshPart& part,float u,float v,std::array<float,3>& result){
    u=std::clamp(u,0.0f,1.0f);v=std::clamp(v,0.0f,1.0f);
    if(part.surface==V14Surface::CoverFront) u=FrontBegin+(1-FrontBegin)*u;
    else if(part.surface==V14Surface::CoverBack) u=BackEnd*u;
    else if(part.surface==V14Surface::CoverSpine) u=BackEnd+(FrontBegin-BackEnd)*u;
    else return false;
    for(std::size_t i=0;i+2<part.indices.size();i+=3){
        const auto& a=part.vertices[part.indices[i]];
        const auto& b=part.vertices[part.indices[i+1]];
        const auto& c=part.vertices[part.indices[i+2]];
        const float denom=(b.v-c.v)*(a.u-c.u)+(c.u-b.u)*(a.v-c.v);
        if(std::fabs(denom)<1e-10f) continue;
        const float w0=((b.v-c.v)*(u-c.u)+(c.u-b.u)*(v-c.v))/denom;
        const float w1=((c.v-a.v)*(u-c.u)+(a.u-c.u)*(v-c.v))/denom;
        const float w2=1-w0-w1;
        if(std::min({w0,w1,w2}) < -0.001f) continue;
        result={{a.x*w0+b.x*w1+c.x*w2,a.y*w0+b.y*w1+c.y*w2,a.z*w0+b.z*w1+c.z*w2}};
        return true;
    }
    return false;
}
