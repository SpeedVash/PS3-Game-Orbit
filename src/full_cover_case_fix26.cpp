#include "full_cover_case_fix26.h"
#include <algorithm>
#include <cmath>

V14CaseMesh build_full_cover_case_fix26(int corner_segments){
    auto mesh=build_v14_case_mesh(corner_segments);
    if(mesh.parts.size()!=6) return mesh;
    auto& front=mesh.parts[3];
    auto& back=mesh.parts[4];
    auto& spine=mesh.parts[5];
    if(front.vertices.size()!=4 || back.vertices.size()!=4) return mesh;

    // Opening is model +X for BOTH sides. The old back had its 5 mm
    // opening margin on the spine side instead, leaving an uncovered gap.
    const float back_shift=front.vertices[0].x-back.vertices[0].x;
    for(auto& v:back.vertices) v.x+=back_shift;

    const float half_depth=7.5f;
    const float paper_offset=front.vertices[0].z-half_depth;
    const float spine_x=front.vertices[0].x;
    const float y_top=front.vertices[0].y,y_bottom=front.vertices[3].y;
    constexpr float Pi=3.14159265358979323846f;
    constexpr int FoldSegments=5;
    const float fold_length=paper_offset*Pi*0.5f;
    const float path_length=2*half_depth+2*fold_length;
    const float u0=FullCoverLayout::Spine.u0,u1=FullCoverLayout::Spine.u1;
    spine.vertices.clear();spine.indices.clear();
    auto point=[&](float x,float z,float nx,float nz,float distance){
        const float u=u0+(u1-u0)*distance/path_length;
        spine.vertices.push_back({x,y_top,z,nx,0,nz,u,0});
        spine.vertices.push_back({x,y_bottom,z,nx,0,nz,u,1});
    };
    for(int i=0;i<=FoldSegments;++i){
        const float angle=(Pi*0.5f)*float(i)/FoldSegments;
        const float sa=std::sin(angle),ca=std::cos(angle);
        point(spine_x-paper_offset*sa,-half_depth-paper_offset*ca,
              -sa,-ca,paper_offset*angle);
    }
    // One uninterrupted vertical spine: middle of the image at model Z=0.
    point(spine_x-paper_offset,half_depth,-1,0,fold_length+2*half_depth);
    for(int i=1;i<=FoldSegments;++i){
        const float angle=(Pi*0.5f)*float(i)/FoldSegments;
        const float sa=std::sin(angle),ca=std::cos(angle);
        point(spine_x-paper_offset*ca,half_depth+paper_offset*sa,
              -ca,sa,fold_length+2*half_depth+paper_offset*angle);
    }
    // Weld endpoints numerically to the adjoining paper planes. The GPU draws
    // use separate material ranges, but position, effective UV and normal agree.
    spine.vertices[0]=back.vertices[0];
    spine.vertices[0].u=u0;spine.vertices[0].v=0;
    spine.vertices[1]=back.vertices[3];
    spine.vertices[1].u=u0;spine.vertices[1].v=1;
    spine.vertices[spine.vertices.size()-2]=front.vertices[0];
    spine.vertices[spine.vertices.size()-2].u=u1;
    spine.vertices.back()=front.vertices[3];spine.vertices.back().u=u1;
    for(std::size_t i=0;i+3<spine.vertices.size();i+=2){
        const auto a=static_cast<std::uint16_t>(i);
        const auto b=static_cast<std::uint16_t>(i+1);
        const auto c=static_cast<std::uint16_t>(i+2);
        const auto d=static_cast<std::uint16_t>(i+3);
        spine.indices.insert(spine.indices.end(),{a,d,c,a,b,d});
    }
    return mesh;
}

std::array<float,3> full_cover_surface_point_fix26(const V14MeshPart& part,float u,float v){
    u=std::clamp(u,0.0f,1.0f);v=std::clamp(v,0.0f,1.0f);
    if(part.vertices.size()==4){
        const auto& a=part.vertices[0];const auto& b=part.vertices[1];const auto& d=part.vertices[3];
        return {a.x+(b.x-a.x)*u+(d.x-a.x)*v,
                a.y+(b.y-a.y)*u+(d.y-a.y)*v,
                a.z+(b.z-a.z)*u+(d.z-a.z)*v};
    }
    if(part.surface!=V14Surface::CoverSpine || part.vertices.size()<4 || part.vertices.size()%2) return {0,0,0};
    const float target=part.vertices.front().u+(part.vertices[part.vertices.size()-2].u-part.vertices.front().u)*u;
    for(std::size_t i=0;i+3<part.vertices.size();i+=2){
        const auto& a=part.vertices[i];const auto& b=part.vertices[i+2];const auto& d=part.vertices[i+1];
        if(target>b.u && i+4<part.vertices.size()) continue;
        const float t=b.u>a.u ? (target-a.u)/(b.u-a.u) : 0;
        return {a.x+(b.x-a.x)*t+(d.x-a.x)*v,
                a.y+(b.y-a.y)*t+(d.y-a.y)*v,
                a.z+(b.z-a.z)*t+(d.z-a.z)*v};
    }
    return {0,0,0};
}
