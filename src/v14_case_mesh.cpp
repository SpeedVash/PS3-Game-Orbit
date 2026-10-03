#include "v14_case_mesh.h"
#include <cmath>

namespace {
constexpr float W = 135.0f;
constexpr float H = 170.0f;
constexpr float D = 15.0f;
constexpr float R = 5.0f;
constexpr float COVER_W = 130.0f;
constexpr float COVER_H = 147.0f;
constexpr float COVER_Y = -6.5f;
constexpr float PI = 3.14159265358979323846f;

struct P2 { float x,y; };

std::vector<P2> perimeter(int seg) {
    if (seg < 2) seg = 2;
    std::vector<P2> p;
    const float cx[4] = { W*.5f-R, -W*.5f+R, -W*.5f+R, W*.5f-R };
    const float cy[4] = { H*.5f-R, H*.5f-R, -H*.5f+R, -H*.5f+R };
    const float a0[4] = { 0, PI*.5f, PI, PI*1.5f };
    for (int c=0;c<4;++c) {
        for (int i=0;i<=seg;++i) {
            if (c>0 && i==0) continue;
            const float a=a0[c]+(PI*.5f)*(float(i)/seg);
            p.push_back({cx[c]+std::cos(a)*R, cy[c]+std::sin(a)*R});
        }
    }
    return p;
}

V14MeshPart cover_plane(V14Surface s, float x0,float x1,float y0,float y1,float z,
                        float nx,float ny,float nz, UVRect uv, bool reverse=false) {
    V14MeshPart m; m.surface=s; m.textured=true; m.uv=uv;
    m.vertices = {
        {x0,y0,z,nx,ny,nz,uv.u0,uv.v0},
        {x1,y0,z,nx,ny,nz,uv.u1,uv.v0},
        {x1,y1,z,nx,ny,nz,uv.u1,uv.v1},
        {x0,y1,z,nx,ny,nz,uv.u0,uv.v1}
    };
    m.indices = reverse ? std::vector<std::uint16_t>{0,2,1,0,3,2}
                        : std::vector<std::uint16_t>{0,1,2,0,2,3};
    return m;
}
}

V14CaseMesh build_v14_case_mesh(int corner_segments) {
    V14CaseMesh mesh;
    const auto ring=perimeter(corner_segments);

    // Rounded front/back plastic faces as triangle fans.
    for (int side=0; side<2; ++side) {
        V14MeshPart m;
        m.surface = side==0 ? V14Surface::ShellFront : V14Surface::ShellBack;
        const float z = side==0 ? D*.5f : -D*.5f;
        const float nz = side==0 ? 1.0f : -1.0f;
        m.vertices.push_back({0,0,z,0,0,nz,0,0});
        for (const auto& p:ring) m.vertices.push_back({p.x,p.y,z,0,0,nz,0,0});
        for (std::uint16_t i=0;i<ring.size();++i) {
            const std::uint16_t a=1+i, b=1+((i+1)%ring.size());
            if (side==0) m.indices.insert(m.indices.end(), {0,a,b});
            else m.indices.insert(m.indices.end(), {0,b,a});
        }
        mesh.parts.push_back(std::move(m));
    }

    // Continuous rounded edge shell.
    V14MeshPart edge; edge.surface=V14Surface::ShellEdge;
    for (std::size_t i=0;i<ring.size();++i) {
        const auto& p=ring[i];
        const float len=std::sqrt(p.x*p.x+p.y*p.y);
        const float nx=len>0?p.x/len:0, ny=len>0?p.y/len:0;
        edge.vertices.push_back({p.x,p.y, D*.5f,nx,ny,0,0,0});
        edge.vertices.push_back({p.x,p.y,-D*.5f,nx,ny,0,0,0});
    }
    for (std::uint16_t i=0;i<ring.size();++i) {
        const std::uint16_t j=(i+1)%ring.size();
        const std::uint16_t a=i*2,b=i*2+1,c=j*2,d=j*2+1;
        edge.indices.insert(edge.indices.end(), {a,c,d,a,d,b});
    }
    mesh.parts.push_back(std::move(edge));

    const float y0=COVER_Y+COVER_H*.5f, y1=COVER_Y-COVER_H*.5f;
    // Front leaves the official 5 mm opening-edge margin: x=-67.5..62.5.
    mesh.parts.push_back(cover_plane(V14Surface::CoverFront,-W*.5f,-W*.5f+COVER_W,y0,y1,D*.5f+0.12f,0,0,1,FullCoverLayout::Front));
    // Back leaves the inverse 5 mm opening-edge margin: x=-62.5..67.5.
    mesh.parts.push_back(cover_plane(V14Surface::CoverBack,W*.5f-COVER_W,W*.5f,y0,y1,-D*.5f-0.12f,0,0,-1,FullCoverLayout::Back,true));

    // Spine is the central 15 mm strip of the SAME full-cover texture.
    V14MeshPart spine; spine.surface=V14Surface::CoverSpine; spine.textured=true; spine.uv=FullCoverLayout::Spine;
    const float x=-W*.5f-0.12f, z0=-D*.5f, z1=D*.5f;
    spine.vertices = {
        {x,y0,z0,-1,0,0, FullCoverLayout::Spine.u0,0},
        {x,y0,z1,-1,0,0, FullCoverLayout::Spine.u1,0},
        {x,y1,z1,-1,0,0, FullCoverLayout::Spine.u1,1},
        {x,y1,z0,-1,0,0, FullCoverLayout::Spine.u0,1}
    };
    spine.indices={0,1,2,0,2,3};
    mesh.parts.push_back(std::move(spine));
    return mesh;
}
