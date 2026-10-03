#include "case_render_fix25.h"
#include <algorithm>

namespace CaseRenderFix25 {
std::vector<std::uint16_t> outward_indices(const V14MeshPart& part) {
    auto indices=part.indices;
    for(std::size_t i=0;i+2<indices.size();i+=3){
        const auto& a=part.vertices.at(indices[i]);
        const auto& b=part.vertices.at(indices[i+1]);
        const auto& c=part.vertices.at(indices[i+2]);
        const float bx=b.x-a.x,by=b.y-a.y,bz=b.z-a.z;
        const float cx=c.x-a.x,cy=c.y-a.y,cz=c.z-a.z;
        const float nx=a.nx+b.nx+c.nx,ny=a.ny+b.ny+c.ny,nz=a.nz+b.nz+c.nz;
        const float dot=(by*cz-bz*cy)*nx+(bz*cx-bx*cz)*ny+(bx*cy-by*cx)*nz;
        if(dot<0) std::swap(indices[i+1],indices[i+2]);
    }
    return indices;
}

std::vector<std::size_t> submission_order(const V10FramePlan& plan,const V14CaseMesh& mesh){
    std::vector<std::size_t> order;
    std::vector<float> depth(plan.packets.size(),0);
    if(mesh.parts.empty()) return order;
    for(std::size_t i=0;i<plan.packets.size();++i){
        order.push_back(i);
        const auto& part=mesh.parts[i%mesh.parts.size()];
        if(part.vertices.empty()) continue;
        float x=0,y=0,z=0;
        for(const auto& v:part.vertices){x+=v.x;y+=v.y;z+=v.z;}
        const float inv=1.0f/part.vertices.size();x*=inv;y*=inv;z*=inv;
        const auto& m=plan.packets[i].mvp.m;
        const float clip_z=m[2]*x+m[6]*y+m[10]*z+m[14];
        const float clip_w=m[3]*x+m[7]*y+m[11]*z+m[15];
        depth[i]=clip_w>0 ? clip_z/clip_w : 0;
    }
    std::stable_sort(order.begin(),order.end(),[&](std::size_t a,std::size_t b){
        if(plan.packets[a].mesh_textured!=plan.packets[b].mesh_textured)
            return plan.packets[a].mesh_textured;
        if(plan.packets[a].mesh_textured) return false;
        return depth[a]>depth[b];
    });
    return order;
}
}
