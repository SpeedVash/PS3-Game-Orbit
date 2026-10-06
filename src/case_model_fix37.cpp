#ifdef PS3_GAME_ORBIT_FIX37
#include "case_model_fix37.h"
#include "case_animation_fix32.h"
#include "jfx_case_fix29.h"
#include <algorithm>
#include <cmath>
#include <map>
#include <utility>
#ifdef PS3_GAME_ORBIT_FIX39
#include <tuple>
#endif

namespace CaseModelFix37 {
namespace {
constexpr float Pi=3.14159265358979323846f;
using Joint=V14MeshPart::Joint;
using Material=V14MeshPart::Material;
using Role=V14MeshPart::TextureRole;
struct Point {float x,y,z;};
Point sub(Point a,Point b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
Point cross(Point a,Point b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
Point normalized(Point p){const float n=std::sqrt(p.x*p.x+p.y*p.y+p.z*p.z);return n>1e-9f ? Point{p.x/n,p.y/n,p.z/n}:Point{0,0,1};}
V14MeshPart clear_part(Joint joint,float alpha=.14f){
    V14MeshPart p;p.joint=joint;p.material=Material::ClearPlastic;p.texture_role=Role::None;
#ifdef PS3_GAME_ORBIT_FIX39
    p.color={{1,1,1,alpha}}; // Colourless clear mouldings, including the spine.
#else
    p.color={{.84f,.85f,.86f,alpha}};
#endif
    return p;
}
void triangle(V14MeshPart& p,Point a,Point b,Point c,bool disc_uv=false){
    const auto n=normalized(cross(sub(b,a),sub(c,a)));
    const auto first=static_cast<std::uint16_t>(p.vertices.size());
    for(const auto v:{a,b,c})p.vertices.push_back({v.x,v.y,v.z,n.x,n.y,n.z,
        disc_uv ? .5f+v.x/120.f:0.f,disc_uv ? .5f-v.y/120.f:0.f});
    p.indices.insert(p.indices.end(),{first,std::uint16_t(first+1),std::uint16_t(first+2)});
}
void quad(V14MeshPart& p,Point a,Point b,Point c,Point d){triangle(p,a,b,c);triangle(p,a,c,d);}
void normals(V14MeshPart& p){
    for(auto& v:p.vertices)v.nx=v.ny=v.nz=0;
    for(std::size_t i=0;i<p.indices.size();i+=3){
        auto& a=p.vertices[p.indices[i]];auto& b=p.vertices[p.indices[i+1]];auto& c=p.vertices[p.indices[i+2]];
        const auto n=cross({b.x-a.x,b.y-a.y,b.z-a.z},{c.x-a.x,c.y-a.y,c.z-a.z});
        for(auto* v:{&a,&b,&c}){v->nx+=n.x;v->ny+=n.y;v->nz+=n.z;}
    }
    for(auto& v:p.vertices){const auto n=normalized({v.nx,v.ny,v.nz});v.nx=n.x;v.ny=n.y;v.nz=n.z;}
}
void reflect_lid(V14MeshPart& p){
    for(auto& v:p.vertices){v.z=-v.z;v.nz=-v.nz;}
    for(std::size_t i=0;i<p.indices.size();i+=3)std::swap(p.indices[i+1],p.indices[i+2]);
}
void move(V14MeshPart& p,float x,float y,float z,float angle=0){
    const float c=std::cos(angle),s=std::sin(angle);
    for(auto& v:p.vertices){
        const float vx=v.x,vy=v.y,nx=v.nx,ny=v.ny;
        v.x=x+vx*c-vy*s;v.y=y+vx*s+vy*c;v.z+=z;
        v.nx=nx*c-ny*s;v.ny=nx*s+ny*c;
    }
}
void merge(V14MeshPart& into,const V14MeshPart& from){
    const auto offset=static_cast<std::uint16_t>(into.vertices.size());
    into.vertices.insert(into.vertices.end(),from.vertices.begin(),from.vertices.end());
    for(auto i:from.indices)into.indices.push_back(std::uint16_t(offset+i));
}
std::vector<Point> outline(float w,float h,float radius){
    std::vector<Point> points;const float r=std::min(radius,std::min(w,h)/2);
    for(int corner=0;corner<4;++corner){
        const float x=(corner==0||corner==3 ? 1:-1)*(w/2-r);
        const float y=(corner<2 ? 1:-1)*(h/2-r);
        for(int i=0;i<=4;++i){const float a=(corner*90.f+i*22.5f)*Pi/180;
            points.push_back({x+r*std::cos(a),y+r*std::sin(a),0});}
    }
    return points;
}
void solid(V14MeshPart& into,float w,float h,float r,float depth,float x,float y,float z,float angle=0){
    auto p=clear_part(into.joint);const auto points=outline(w,h,r);
    for(std::size_t i=0;i<points.size();++i){
        const auto a=points[i],b=points[(i+1)%points.size()];
        triangle(p,{0,0,0},b,a);triangle(p,{0,0,depth},{a.x,a.y,depth},{b.x,b.y,depth});
        quad(p,a,b,{b.x,b.y,depth},{a.x,a.y,depth});
    }
    move(p,x,y,z,angle);merge(into,p);
}
void frame(V14MeshPart& into){
    const auto outer=outline(134,163,5),inner=outline(131.9f,160.9f,3.95f);
    auto p=clear_part(into.joint);constexpr float d=3.45f;
    for(std::size_t i=0;i<outer.size();++i){
        const auto j=(i+1)%outer.size();const auto a=outer[i],b=outer[j],c=inner[j],e=inner[i];
        quad(p,a,b,{b.x,b.y,d},{a.x,a.y,d});
        quad(p,c,e,{e.x,e.y,d},{c.x,c.y,d});
        quad(p,{a.x,a.y,d},{b.x,b.y,d},{c.x,c.y,d},{e.x,e.y,d});quad(p,e,c,b,a);
    }
    move(p,0,-1.2f,-5.3f);merge(into,p);
}
void annulus_face(V14MeshPart& p,float inner,float outer,float z,bool front,bool uv=false,
                  float begin=0,float end=2*Pi,int segments=96){
    for(int i=0;i<segments;++i){
        const float a=begin+(end-begin)*i/segments,b=begin+(end-begin)*(i+1)/segments;
        const Point ia{inner*std::cos(a),inner*std::sin(a),z},ib{inner*std::cos(b),inner*std::sin(b),z};
        const Point oa{outer*std::cos(a),outer*std::sin(a),z},ob{outer*std::cos(b),outer*std::sin(b),z};
        if(front){triangle(p,ia,oa,ob,uv);triangle(p,ia,ob,ib,uv);}
        else{triangle(p,ia,ob,oa,uv);triangle(p,ia,ib,ob,uv);}
    }
}
void wall(V14MeshPart& p,float radius,float z0,float z1,bool outer,float begin=0,float end=2*Pi,int segments=96){
    for(int i=0;i<segments;++i){
        const float a=begin+(end-begin)*i/segments,b=begin+(end-begin)*(i+1)/segments;
        const Point lo{radius*std::cos(a),radius*std::sin(a),z0},hi{lo.x,lo.y,z1};
        const Point lo2{radius*std::cos(b),radius*std::sin(b),z0},hi2{lo2.x,lo2.y,z1};
        if(outer)quad(p,lo,lo2,hi2,hi);else quad(p,lo2,lo,hi,hi2);
    }
}
void ring(V14MeshPart& into,float inner,float outer,float depth,float z,float start=0,float end=360){
    auto p=clear_part(into.joint);const float a=start*Pi/180,b=end*Pi/180;
    const int segments=std::max(8,int(std::ceil((end-start)/5)));
    annulus_face(p,inner,outer,0,false,false,a,b,segments);annulus_face(p,inner,outer,depth,true,false,a,b,segments);
    wall(p,outer,0,depth,true,a,b,segments);if(inner>0)wall(p,inner,0,depth,false,a,b,segments);
    if(end-start<359.9f){
        for(const auto v:{a,b}){
            const Point i{inner*std::cos(v),inner*std::sin(v),0},o{outer*std::cos(v),outer*std::sin(v),0};
            if(v==a)quad(p,i,o,{o.x,o.y,depth},{i.x,i.y,depth});
            else quad(p,o,i,{i.x,i.y,depth},{o.x,o.y,depth});
        }
    }
    move(p,0,-10,z);merge(into,p);
}
void clip_hook(V14MeshPart& p,float x,float y){
    // A curved D clip with rounded cross section, held at both ends.
    const Point path[]={{-3,-12,.2f},{3.5f,-11,1.4f},{5,-7,2.5f},{5,7,2.5f},{3.5f,11,1.4f},{-3,12,.2f}};
    auto curve=[&](float t){
        const int k=std::min(4,int(t));const float u=t-k;
        const auto a=path[std::max(0,k-1)],b=path[k],c=path[k+1],d=path[std::min(5,k+2)];
        auto f=[&](float aa,float bb,float cc,float dd){return .5f*((2*bb)+(-aa+cc)*u+(2*aa-5*bb+4*cc-dd)*u*u+(-aa+3*bb-3*cc+dd)*u*u*u);};
        return Point{f(a.x,b.x,c.x,d.x),f(a.y,b.y,c.y,d.y),f(a.z,b.z,c.z,d.z)};
    };
    auto tube=clear_part(p.joint);std::vector<std::array<Point,6>> rows;
    for(int j=0;j<=24;++j){
        const float t=j*5.f/24;const auto center=curve(t);
        const auto tangent=normalized(sub(curve(std::min(5.f,t+.01f)),curve(std::max(0.f,t-.01f))));
        const auto side=normalized(cross(tangent,{0,0,1})),up=cross(side,tangent);
        std::array<Point,6> row;
        for(int i=0;i<6;++i){const float a=i*2*Pi/6,c=.85f*std::cos(a),s=.85f*std::sin(a);
            row[i]={center.x+side.x*c+up.x*s,center.y+side.y*c+up.y*s,center.z+side.z*c+up.z*s};}
        rows.push_back(row);
    }
    for(std::size_t j=1;j<rows.size();++j)for(int i=0;i<6;++i){const int n=(i+1)%6;quad(tube,rows[j-1][i],rows[j][i],rows[j][n],rows[j-1][n]);}
    move(tube,x,y,-5.25f);merge(p,tube);solid(p,3,27,1,.7f,x-3.2f,y,-5.5f);
}
void interior(V14CaseMesh& mesh){
    for(const auto joint:{Joint::Base,Joint::Lid}){
        auto p=clear_part(joint),m=clear_part(joint,.22f),f=clear_part(joint,.30f);
        solid(p,131,157,4.4f,.48f,0,-1.3f,-5.75f);frame(p);
        if(joint==Joint::Lid){
            for(const float y:{-74.f,66.f})solid(p,113,2.6f,1.2f,1.7f,0,y,-5.2f);
            clip_hook(p,54.5f,48);clip_hook(p,54.5f,-49);
            for(const float y:{-65.f,57.f})solid(p,4,8,1.2f,1.5f,-60,y,-5.f);
        }else{
            for(const auto arc:std::array<std::array<float,2>,4>{{{{-29,38}},{{58,150}},{{171,218}},{{240,310}}}})ring(m,60.85f,62.5f,2.35f,-5,arc[0],arc[1]);
            ring(p,8.9f,60.8f,.22f,-5.05f);
            for(const float deg:{14.f,87.f,158.f,270.f}){const float a=deg*Pi/180;solid(p,5,3.1f,1,1.9f,59.5f*std::cos(a),-10+59.5f*std::sin(a),-4.9f,a);}
            for(const float deg:{30.f,150.f,270.f}){const float a=deg*Pi/180;solid(p,43,1.1f,.5f,.6f,32.5f*std::cos(a),-10+32.5f*std::sin(a),-4.9f,a);}
            ring(f,5.1f,7.05f,3.8f,-4.5f);
            for(int i=0;i<6;++i){const float a=(i*60+5)*Pi/180;
                solid(m,3.4f,2.8f,.65f,1.25f,6*std::cos(a),-10+6*std::sin(a),-.8f,a);
                solid(p,1.1f,7.8f,.45f,.7f,10.8f*std::cos(a),-10+10.8f*std::sin(a),-4.25f,a-Pi/2);}
            ring(f,0,4.5f,1,-.55f);
        }
        for(const float y:{-49.f,47.f})solid(joint==Joint::Lid ? p:m,joint==Joint::Lid ? 2.2f:2.4f,joint==Joint::Lid ? 10:11,1,joint==Joint::Lid ? 1.2f:2.4f,65.7f,y,joint==Joint::Lid ? -4.9f:-4.65f);
        for(const float y:{-77.f,77.f}){
            auto pin=clear_part(joint);ring(pin,0,.72f,5,0); // Local cylinder axis Z -> hinge axis Y.
            for(auto& v:pin.vertices){const float yy=v.y+10,zz=v.z;v.y=y+zz-2.5f;v.z=BackHingeZ+.1f-yy;v.x+=HingeX;
                const float ny=v.ny,nz=v.nz;v.ny=nz;v.nz=-ny;}
            merge(p,pin);
        }
        if(joint==Joint::Lid){reflect_lid(p);reflect_lid(m);reflect_lid(f);}
        for(auto* part:{&p,&m,&f})if(!part->indices.empty())mesh.parts.push_back(std::move(*part));
    }
}
V14Vertex interpolate(const V14Vertex& a,const V14Vertex& b,float t,int axis,float boundary){
    const auto lerp=[&](float x,float y){return x+(y-x)*t;};
    V14Vertex v{lerp(a.x,b.x),lerp(a.y,b.y),lerp(a.z,b.z),lerp(a.nx,b.nx),lerp(a.ny,b.ny),lerp(a.nz,b.nz),lerp(a.u,b.u),lerp(a.v,b.v)};
    if(axis==0)v.x=boundary;else v.z=boundary;
    const auto n=normalized({v.nx,v.ny,v.nz});v.nx=n.x;v.ny=n.y;v.nz=n.z;return v;
}
std::vector<V14Vertex> clip(const std::vector<V14Vertex>& polygon,int axis,float boundary,bool above){
    std::vector<V14Vertex> out;
    for(std::size_t i=0;i<polygon.size();++i){const auto& a=polygon[i];const auto& b=polygon[(i+1)%polygon.size()];
        const float av=axis==0?a.x:a.z,bv=axis==0?b.x:b.z;
        const bool ai=above ? av>=boundary:av<=boundary,bi=above ? bv>=boundary:bv<=boundary;
        if(ai)out.push_back(a);
        if(ai!=bi)out.push_back(interpolate(a,b,(boundary-av)/(bv-av),axis,boundary));
    }return out;
}
V14MeshPart shell_half(const V14MeshPart& source,Joint joint){
    auto p=source;p.vertices.clear();p.indices.clear();p.joint=joint;p.color[3]=.14f;
    for(std::size_t i=0;i<source.indices.size();i+=3){
        std::vector<V14Vertex> poly{source.vertices[source.indices[i]],source.vertices[source.indices[i+1]],source.vertices[source.indices[i+2]]};
        poly=clip(clip(poly,0,HingeX,true),2,0,joint==Joint::Lid);
        for(std::size_t j=1;j+1<poly.size();++j){const auto a=poly[0],b=poly[j],c=poly[j+1];
            const auto n=cross({b.x-a.x,b.y-a.y,b.z-a.z},{c.x-a.x,c.y-a.y,c.z-a.z});
            if(n.x*n.x+n.y*n.y+n.z*n.z<1e-12f)continue;
            const auto first=std::uint16_t(p.vertices.size());p.vertices.insert(p.vertices.end(),{a,b,c});
            p.indices.insert(p.indices.end(),{first,std::uint16_t(first+1),std::uint16_t(first+2)});
        }
    }return p;
}
#ifndef PS3_GAME_ORBIT_FIX39
void plastic_spine(V14CaseMesh& mesh,const V14MeshPart& paper){
    std::map<float,Point> stations;for(const auto& v:paper.vertices)stations[v.u]={v.x,0,v.z};
    std::vector<Point> curve;for(const auto& pair:stations)curve.push_back(pair.second);
    auto p=clear_part(Joint::Spine);const unsigned n=curve.size();
    for(unsigned layer=0;layer<2;++layer)for(unsigned row=0;row<2;++row)for(const auto v:curve){
        p.vertices.push_back({v.x,row?84.2f:-84.2f,v.z,0,0,1,0,0});p.flex_offsets.push_back(layer ? .58f:-.16f);}
    const auto at=[&](unsigned l,unsigned r,unsigned i){return std::uint16_t(l*2*n+r*n+i);};
    const auto q=[&](std::uint16_t a,std::uint16_t b,std::uint16_t c,std::uint16_t d){p.indices.insert(p.indices.end(),{a,b,c,a,c,d});};
    for(unsigned i=0;i+1<n;++i){
        q(at(0,0,i),at(0,0,i+1),at(0,1,i+1),at(0,1,i));q(at(1,1,i),at(1,1,i+1),at(1,0,i+1),at(1,0,i));
        q(at(0,0,i),at(1,0,i),at(1,0,i+1),at(0,0,i+1));q(at(0,1,i+1),at(1,1,i+1),at(1,1,i),at(0,1,i));
    }
    q(at(0,0,0),at(0,1,0),at(1,1,0),at(1,0,0));
    q(at(1,0,n-1),at(1,1,n-1),at(0,1,n-1),at(0,0,n-1));
    p.flex_rest=p.vertices;mesh.parts.push_back(std::move(p));
}

#else
void rounded_spine(V14CaseMesh& mesh,const V14MeshPart& shell,const V14MeshPart& paper){
    // Keep the curved top/bottom mouldings from the approved shell. Both
    // sides are cut on the same hinge plane, so their boundary is identical.
    // The old straight ribbon and its flat end caps are no longer inserted.
    std::map<float,Point> stations;
    for(const auto& v:paper.vertices)stations[v.u]={v.x,0,v.z};
    std::vector<std::pair<float,Point>> curve(stations.begin(),stations.end());
    auto p=clear_part(Joint::Spine);
    for(std::size_t i=0;i<shell.indices.size();i+=3){
        const std::vector<V14Vertex> tri{shell.vertices[shell.indices[i]],shell.vertices[shell.indices[i+1]],shell.vertices[shell.indices[i+2]]};
        const auto left=clip(tri,0,HingeX,false);
        // Split the central seam so the two top/bottom lips can follow their
        // own hinge at the boundary while the outer band remains flexible.
        for(const bool front:{false,true}){
            const auto poly=clip(left,2,0,front);
            for(std::size_t j=1;j+1<poly.size();++j){
                const auto a=poly[0],b=poly[j],c=poly[j+1];
                const auto n=cross({b.x-a.x,b.y-a.y,b.z-a.z},{c.x-a.x,c.y-a.y,c.z-a.z});
                if(n.x*n.x+n.y*n.y+n.z*n.z<1e-12f)continue;
                const auto first=std::uint16_t(p.vertices.size());
                for(const auto& v:{a,b,c}){
                    p.vertices.push_back(v);
                    float best=1e30f,qx=HingeX,qz=front?FrontHingeZ:BackHingeZ,t=front?1.f:0.f;
                    if(std::fabs(v.x-HingeX)>.0001f){
                        for(std::size_t k=1;k<curve.size();++k){
                            const auto aa=curve[k-1].second,bb=curve[k].second;
                            const float dx=bb.x-aa.x,dz=bb.z-aa.z;
                            const float length=dx*dx+dz*dz;
                            if(length<1e-12f)continue;
                            const float u=std::clamp(((v.x-aa.x)*dx+(v.z-aa.z)*dz)/length,0.f,1.f);
                            const float x=aa.x+u*dx,z=aa.z+u*dz,d=(v.x-x)*(v.x-x)+(v.z-z)*(v.z-z);
                            if(d>=best)continue;
                            best=d;qx=x;qz=z;
                            const float wrap=curve[k-1].first+u*(curve[k].first-curve[k-1].first);
                            t=std::clamp((wrap-JfxCaseFix29::BackEnd)/(JfxCaseFix29::FrontBegin-JfxCaseFix29::BackEnd),0.f,1.f);
                        }
                    }
                    p.flex_shell_anchors.push_back({{qx,qz,t}});
                }
                p.indices.insert(p.indices.end(),{first,std::uint16_t(first+1),std::uint16_t(first+2)});
            }
        }
    }
    p.flex_rest=p.vertices;
    using Key=std::tuple<long,long,long,long,long,long>;
    std::map<Key,std::uint16_t> groups;
    for(const auto& v:p.vertices){
        const Key key{std::lround(v.x*10000),std::lround(v.y*10000),std::lround(v.z*10000),
            std::lround(v.nx*1000),std::lround(v.ny*1000),std::lround(v.nz*1000)};
        auto found=groups.find(key);
        if(found==groups.end())found=groups.emplace(key,std::uint16_t(groups.size())).first;
        p.flex_normal_groups.push_back(found->second);
    }
    p.flex_normal_sums.resize(groups.size(),{{0,0,0}});
    mesh.parts.push_back(std::move(p));
}
void smooth_shell_normals(V14MeshPart& p){
    std::fill(p.flex_normal_sums.begin(),p.flex_normal_sums.end(),std::array<float,3>{{0,0,0}});
    for(std::size_t i=0;i<p.indices.size();i+=3){
        const auto a=p.vertices[p.indices[i]],b=p.vertices[p.indices[i+1]],c=p.vertices[p.indices[i+2]];
        const auto n=cross({b.x-a.x,b.y-a.y,b.z-a.z},{c.x-a.x,c.y-a.y,c.z-a.z});
        for(unsigned k=0;k<3;++k){auto& sum=p.flex_normal_sums[p.flex_normal_groups[p.indices[i+k]]];sum[0]+=n.x;sum[1]+=n.y;sum[2]+=n.z;}
    }
    for(std::size_t i=0;i<p.vertices.size();++i){
        const auto sum=p.flex_normal_sums[p.flex_normal_groups[i]];const auto n=normalized({sum[0],sum[1],sum[2]});
        auto& v=p.vertices[i];v.nx=n.x;v.ny=n.y;v.nz=n.z;
    }
}
#endif
void disc(V14CaseMesh& mesh){
    V14MeshPart label;label.joint=Joint::Disc;label.material=Material::Disc;label.textured=true;label.texture_role=Role::DiscLabel;label.color={{1,1,1,1}};
    auto back=label;back.texture_role=Role::DiscBack;
    annulus_face(label,DiscLabelInnerRadius,DiscLabelRadius,.6f,true,true);annulus_face(back,DiscLabelInnerRadius,DiscLabelRadius,-.6f,false,true);
    auto edge=label;edge.textured=false;edge.texture_role=Role::None;edge.color={{.72f,.75f,.77f,1}};
#ifdef PS3_GAME_ORBIT_FIX38
    // The old copy retained the entire label face, so its untextured gray
    // triangles overwrote ID_DISC.png at the same depth (LEQUAL).
    edge.vertices.clear();edge.indices.clear();
#endif
    wall(edge,DiscLabelRadius,-.6f,.6f,true);wall(edge,DiscLabelInnerRadius,-.6f,.6f,false);
    auto rim=clear_part(Joint::Disc,.22f);rim.material=Material::DiscGlass;
    annulus_face(rim,DiscLabelRadius,DiscRadius,.6f,true);annulus_face(rim,DiscLabelRadius,DiscRadius,-.6f,false);
    wall(rim,DiscRadius,-.6f,.6f,true);
#ifdef PS3_GAME_ORBIT_FIX38
    annulus_face(rim,DiscHoleRadius,DiscLabelInnerRadius,.6f,true);
    annulus_face(rim,DiscHoleRadius,DiscLabelInnerRadius,-.6f,false);
    wall(rim,DiscHoleRadius,-.6f,.6f,false);
#endif
    for(auto* p:{&label,&back,&edge,&rim})mesh.parts.push_back(std::move(*p));
}
}
float opening_angle(float phase){return
#ifdef PS3_GAME_ORBIT_FIX38
    160
#else
    180
#endif
    *CaseAnimationFix32::smooth(phase,.20f,.65f);}
Mat4 joint_transform(Joint joint,float phase){
    const float a=opening_angle(phase),half=a*Pi/360;
    if(joint==Joint::Spine)return mat4_mul(mat4_translate(HingeX,0,BackHingeZ),mat4_mul(mat4_rotate_y_deg(-a/2),mat4_translate(-HingeX,0,-BackHingeZ)));
    if(joint==Joint::Lid){const float d=FrontHingeZ-BackHingeZ;
        return mat4_mul(mat4_translate(HingeX-d*std::sin(half),0,BackHingeZ+d*std::cos(half)),
            mat4_mul(mat4_rotate_y_deg(-a),mat4_translate(-HingeX,0,-FrontHingeZ)));}
    if(joint==Joint::Disc){const float s=CaseAnimationFix32::disc_slide(phase),lift=CaseAnimationFix32::smooth(phase,.65f,.76f);
        return mat4_mul(mat4_translate(
#ifdef PS3_GAME_ORBIT_FIX38
            96*s,-10,-2.3f+12*lift+15*s
#else
            82*s,-10,-2.3f+12*lift+7*s
#endif
            ),mat4_rotate_y_deg(-6*s));}
    return mat4_identity();
}
void deform_spine(V14CaseMesh& mesh,float phase){
    const float half=opening_angle(phase)*Pi/360,c=std::cos(half),s=std::sin(half);
    const float base=-InsideZ-BackHingeZ,lid=InsideZ-FrontHingeZ;
    for(auto& p:mesh.parts){if(p.flex_rest.empty())continue;
        for(std::size_t i=0;i<p.vertices.size();++i){const auto& rest=p.flex_rest[i];auto& v=p.vertices[i];
#ifdef PS3_GAME_ORBIT_FIX39
            if(!p.flex_shell_anchors.empty()){
                const auto q=p.flex_shell_anchors[i];const float t=q[2],dx=rest.x-q[0],dz=rest.z-q[1];
                // Transport offsets around the closest point of the wrap. At
                // the shared boundary this equals the rigid base/lid exactly.
                v.x=HingeX+(q[0]-HingeX)*c+dx*c+(1-2*t)*dz*s;
                v.z=q[1]+(2*t-1)*dx*s+dz*c;
                continue;
            }
#endif
            const float t=p.flex_inside ? std::clamp((1-rest.u-JfxCaseFix29::BackEnd)/(JfxCaseFix29::FrontBegin-JfxCaseFix29::BackEnd),0.f,1.f):0.f;
            const float dx=p.flex_inside ? base*s*(1-t)-lid*s*t:0.f;
            const float dz=p.flex_inside ? base*c*(1-t)+lid*c*t:0.f;
            v.x=HingeX+(rest.x-HingeX)*c+(p.flex_offsets.empty()?0:p.flex_offsets[i])+dx;
            v.z=rest.z+dz;
        }
#ifdef PS3_GAME_ORBIT_FIX39
        if(!p.flex_shell_anchors.empty()){
            if(half==0){for(std::size_t i=0;i<p.vertices.size();++i){auto& v=p.vertices[i];const auto& r=p.flex_rest[i];v.nx=r.nx;v.ny=r.ny;v.nz=r.nz;}}
            else smooth_shell_normals(p);
        }else
#endif
        normals(p);
    }
}
V14CaseMesh build(const V14CaseMesh& original){
    V14CaseMesh mesh=original;
#ifdef PS3_GAME_ORBIT_FIX38
    mesh.parts.clear(); // Build the approved articulated model first.
#endif
    for(const auto& source:original.parts){
        if(source.material==Material::ClearPlastic){mesh.parts.push_back(shell_half(source,Joint::Base));mesh.parts.push_back(shell_half(source,Joint::Lid));
#ifdef PS3_GAME_ORBIT_FIX39
            for(const auto& paper:original.parts)if(paper.material==Material::Paper && paper.surface==V14Surface::CoverSpine){rounded_spine(mesh,source,paper);break;}
#endif
            continue;}
        auto p=source;p.joint=source.surface==V14Surface::CoverBack ? Joint::Base:source.surface==V14Surface::CoverSpine ? Joint::Spine:Joint::Lid;
        if(p.joint==Joint::Spine)p.flex_rest=p.vertices;
        mesh.parts.push_back(p);
        if(source.material!=Material::Paper)continue;
        auto inside=p;inside.texture_role=Role::Inside;inside.color={{1,1,1,1}};
        for(auto& v:inside.vertices){v.u=1-v.u;v.nx=-v.nx;v.ny=-v.ny;v.nz=-v.nz;
            if(p.joint==Joint::Base)v.z=-InsideZ;else if(p.joint==Joint::Lid)v.z=InsideZ;}
        for(std::size_t i=0;i<inside.indices.size();i+=3)std::swap(inside.indices[i+1],inside.indices[i+2]);
        if(p.joint==Joint::Spine){inside.flex_rest=inside.vertices;inside.flex_inside=true;}
        mesh.parts.push_back(std::move(inside));
#ifndef PS3_GAME_ORBIT_FIX39
        if(p.joint==Joint::Spine)plastic_spine(mesh,source);
#endif
    }
    interior(mesh);disc(mesh);deform_spine(mesh,0);
#ifdef PS3_GAME_ORBIT_FIX38
    // Six closed draw packets are still needed for the bounded Spine/FIFO
    // transition pool. Bake them from THIS model at rest, including its new
    // mouldings, spine and clear header. No old closed shell is inserted.
    // Inside paper and the seated disc are entirely behind the opaque wrap.
    std::array<V14MeshPart,5> closed;
    closed[0]=clear_part(Joint::Closed);
    for(const auto& p:mesh.parts){
        if(p.material==Material::ClearPlastic){merge(closed[0],p);continue;}
        if(p.texture_role!=Role::Cover)continue;
        const unsigned i=p.surface==V14Surface::CoverFront?1:p.surface==V14Surface::CoverBack?2:p.surface==V14Surface::CoverSpine?3:4;
        closed[i]=p;closed[i].joint=Joint::Closed;
        closed[i].flex_rest.clear();closed[i].flex_offsets.clear();closed[i].flex_inside=false;
    }
    mesh.parts.insert(mesh.parts.begin(),std::make_move_iterator(closed.begin()),std::make_move_iterator(closed.end()));
#endif
    return mesh;
}
}
#endif
