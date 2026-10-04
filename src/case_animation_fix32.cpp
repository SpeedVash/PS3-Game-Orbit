#ifdef PS3_GAME_ORBIT_FIX32
#include "case_animation_fix32.h"
#include <algorithm>
#include <cmath>
#include <iterator>
#include <utility>
#ifdef PS3_GAME_ORBIT_FIX33
#include "jfx_case_fix29.h"
#endif
#include "disc_data_fix32.inc"

namespace CaseAnimationFix32 {
float smooth(float t,float a,float b) {
    const float v=std::clamp((t-a)/(b-a),0.0f,1.0f);return v*v*(3-2*v);
}
float lid_angle(float t) {return -115*smooth(t,.20f,.65f);}
float disc_slide(float t) {return smooth(t,.76f,1.0f);}
Mat4 joint_transform(V14MeshPart::Joint joint,float phase) {
    if(joint==V14MeshPart::Joint::Lid) {
        return mat4_mul(mat4_translate(HingeX,0,0),
            mat4_mul(mat4_rotate_y_deg(lid_angle(phase)),mat4_translate(-HingeX,0,0)));
    }
    if(joint==V14MeshPart::Joint::Disc) {
        const float lift=smooth(phase,.65f,.76f),slide=disc_slide(phase);
        return mat4_translate(124*slide,-10,-3.6f+18*lift+30*slide);
    }
    return mat4_identity();
}
namespace {
V14Vertex interpolate(const V14Vertex& a,const V14Vertex& b,float t) {
    auto mix=[&](float x,float y){return x+(y-x)*t;};
    V14Vertex v{mix(a.x,b.x),mix(a.y,b.y),0,mix(a.nx,b.nx),mix(a.ny,b.ny),
                mix(a.nz,b.nz),mix(a.u,b.u),mix(a.v,b.v)};
    const float length=std::sqrt(v.nx*v.nx+v.ny*v.ny+v.nz*v.nz);
    if(length>1e-8f){v.nx/=length;v.ny/=length;v.nz/=length;}
    return v;
}
std::vector<V14Vertex> clip(const std::vector<V14Vertex>& triangle,bool lid) {
    std::vector<V14Vertex> out;
    const auto inside=[&](const V14Vertex& v){return lid ? v.z>=0 : v.z<=0;};
    for(std::size_t i=0;i<triangle.size();++i) {
        const auto& a=triangle[i];const auto& b=triangle[(i+1)%triangle.size()];
        const bool ai=inside(a),bi=inside(b);
        if(ai) out.push_back(a);
        if(ai!=bi) out.push_back(interpolate(a,b,-a.z/(b.z-a.z)));
    }
    return out;
}
void append(V14MeshPart& part,const std::vector<V14Vertex>& polygon) {
    for(std::size_t i=1;i+1<polygon.size();++i) {
        const auto& a=polygon[0];const auto& b=polygon[i];const auto& c=polygon[i+1];
        const float ux=b.x-a.x,uy=b.y-a.y,uz=b.z-a.z;
        const float vx=c.x-a.x,vy=c.y-a.y,vz=c.z-a.z;
        const float nx=uy*vz-uz*vy,ny=uz*vx-ux*vz,nz=ux*vy-uy*vx;
        if(nx*nx+ny*ny+nz*nz<1e-12f) continue;
        const auto first=part.vertices.size();
        part.vertices.insert(part.vertices.end(),{a,b,c});
        for(unsigned k=0;k<3;++k) part.indices.push_back(std::uint16_t(first+k));
    }
}
void interior(V14CaseMesh& mesh,V14MeshPart::Joint joint,float z,float normal) {
    V14MeshPart p;p.joint=joint;p.material=V14MeshPart::Material::Paper;
#ifdef PS3_GAME_ORBIT_FIX33
    p.texture_role=V14MeshPart::TextureRole::Inside;p.textured=true;
#else
    p.texture_role=V14MeshPart::TextureRole::None;
#endif
    p.color={{.76f,.78f,.79f,1}};p.surface=V14Surface::ShellEdge;
    p.vertices={{-61,-75,z,0,0,normal,0,0},{61,-75,z,0,0,normal,1,0},
                {61,63,z,0,0,normal,1,1},{-61,63,z,0,0,normal,0,1}};
#ifdef PS3_GAME_ORBIT_FIX33
    const bool lid=joint==V14MeshPart::Joint::Lid;
    const float left=lid ? JfxCaseFix29::BackEnd : JfxCaseFix29::FrontBegin;
    const float right=lid ? 0.0f : 1.0f;
    for(auto& v:p.vertices) {v.u=v.x<0 ? left : right;v.v=v.y<0 ? 1.0f : 0.0f;}
#endif
    p.indices=normal>0 ? std::vector<std::uint16_t>{0,1,2,0,2,3} :
                        std::vector<std::uint16_t>{0,2,1,0,3,2};
    mesh.parts.push_back(std::move(p));
}
#ifdef PS3_GAME_ORBIT_FIX33
void inner_spine(V14CaseMesh& mesh,V14MeshPart::Joint joint,float z,float normal) {
    V14MeshPart p;p.joint=joint;p.material=V14MeshPart::Material::Paper;
    p.texture_role=V14MeshPart::TextureRole::Inside;p.textured=true;
    p.color={{.76f,.78f,.79f,1}};p.surface=V14Surface::ShellEdge;
    const float edge=joint==V14MeshPart::Joint::Lid ? JfxCaseFix29::BackEnd : JfxCaseFix29::FrontBegin;
    p.vertices={{HingeX,-75,z,0,0,normal,.5f,1},{-61,-75,z,0,0,normal,edge,1},
                {-61,63,z,0,0,normal,edge,0},{HingeX,63,z,0,0,normal,.5f,0}};
    p.indices=normal>0 ? std::vector<std::uint16_t>{0,1,2,0,2,3} : std::vector<std::uint16_t>{0,2,1,0,3,2};
    mesh.parts.push_back(std::move(p));
}
#endif
void hub(V14CaseMesh& mesh) {
    V14MeshPart p;p.joint=V14MeshPart::Joint::Base;p.material=V14MeshPart::Material::Disc;
    p.texture_role=V14MeshPart::TextureRole::None;p.color={{.68f,.70f,.72f,1}};
    p.surface=V14Surface::ShellEdge;
    p.vertices.push_back({0,-10,-3.6f,0,0,1,0,0});
    constexpr int segments=48;
    for(int i=0;i<=segments;++i) {
        const float angle=i*6.28318530718f/segments,x=6.8f*std::cos(angle),y=6.8f*std::sin(angle);
        p.vertices.push_back({x,y-10,-3.6f,0,0,1,0,0});
        if(i>0) p.indices.insert(p.indices.end(),{0,std::uint16_t(i),std::uint16_t(i+1)});
    }
    mesh.parts.push_back(std::move(p));
}
}
V14CaseMesh build(const V14CaseMesh& original) {
    V14CaseMesh mesh=original;
    for(const auto& source:original.parts) {
        for(bool lid:{false,true}) {
            V14MeshPart p=source;p.vertices.clear();p.indices.clear();
            p.joint=lid ? V14MeshPart::Joint::Lid : V14MeshPart::Joint::Base;
            for(std::size_t i=0;i+2<source.indices.size();i+=3) {
                std::vector<V14Vertex> tri{source.vertices[source.indices[i]],
                    source.vertices[source.indices[i+1]],source.vertices[source.indices[i+2]]};
                if(lid && std::all_of(tri.begin(),tri.end(),[](const V14Vertex& v){return v.z==0;})) continue;
                append(p,clip(tri,lid));
            }
            if(!p.indices.empty()) mesh.parts.push_back(std::move(p));
        }
    }
    interior(mesh,V14MeshPart::Joint::Base,-5.8f,1);
    interior(mesh,V14MeshPart::Joint::Lid,5.8f,-1);
#ifdef PS3_GAME_ORBIT_FIX33
    inner_spine(mesh,V14MeshPart::Joint::Base,-5.8f,1);
    inner_spine(mesh,V14MeshPart::Joint::Lid,5.8f,-1);
#endif
    hub(mesh);add_disc32(mesh);
#ifdef PS3_GAME_ORBIT_FIX35
    const auto center_index=mesh.parts.size()-3;
    auto source=std::move(mesh.parts[center_index]);
    std::array<V14MeshPart,3> center;
    for(unsigned i=0;i<center.size();++i){
        center[i]=source;center[i].vertices.clear();center[i].indices.clear();
        center[i].textured=i<2;center[i].texture_role=i==0 ? V14MeshPart::TextureRole::DiscLabel : i==1 ? V14MeshPart::TextureRole::DiscBack : V14MeshPart::TextureRole::None;
        if(i<2)center[i].color={{1,1,1,1}};
    }
    for(std::size_t i=0;i<source.indices.size();i+=3){
        float nz=0;for(unsigned j=0;j<3;++j)nz+=source.vertices[source.indices[i+j]].nz;
        auto& part=center[nz>.01f ? 0 : nz<-.01f ? 1 : 2];
        const auto first=std::uint16_t(part.vertices.size());
        for(unsigned j=0;j<3;++j)part.vertices.push_back(source.vertices[source.indices[i+j]]);
        part.indices.insert(part.indices.end(),{first,std::uint16_t(first+1),std::uint16_t(first+2)});
    }
    mesh.parts[center_index]=std::move(center[0]);
    mesh.parts.push_back(std::move(center[1]));mesh.parts.push_back(std::move(center[2]));
#endif
    return mesh;
}
}
#endif
