#include "math3d.h"
#include <cmath>

namespace {
constexpr float PI=3.14159265358979323846f;
struct V3{float x,y,z;};
V3 sub(V3 a,V3 b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
float dot(V3 a,V3 b){return a.x*b.x+a.y*b.y+a.z*b.z;}
V3 cross(V3 a,V3 b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
V3 norm(V3 v){float l=std::sqrt(dot(v,v)); return l>0?V3{v.x/l,v.y/l,v.z/l}:V3{0,0,0};}
}

std::array<float,16> mat4_shader_rows(const Mat4& matrix){
    std::array<float,16> rows{};
    for(int row=0;row<4;++row) for(int col=0;col<4;++col)
        rows[row*4+col]=matrix.m[col*4+row];
    return rows;
}
Mat4 mat4_identity(){ Mat4 r{}; r.m={1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1}; return r; }
Mat4 mat4_mul(const Mat4&a,const Mat4&b){
    Mat4 r{};
    for(int c=0;c<4;++c) for(int rr=0;rr<4;++rr){
        float v=0; for(int k=0;k<4;++k) v+=a.m[k*4+rr]*b.m[c*4+k];
        r.m[c*4+rr]=v;
    }
    return r;
}
Mat4 mat4_translate(float x,float y,float z){ Mat4 r=mat4_identity(); r.m[12]=x;r.m[13]=y;r.m[14]=z; return r; }
Mat4 mat4_scale(float s){ Mat4 r=mat4_identity(); r.m[0]=r.m[5]=r.m[10]=s; return r; }
Mat4 mat4_rotate_x_deg(float d){float a=d*PI/180.0f,c=std::cos(a),s=std::sin(a);Mat4 r=mat4_identity();r.m[5]=c;r.m[6]=s;r.m[9]=-s;r.m[10]=c;return r;}
Mat4 mat4_rotate_y_deg(float d){float a=d*PI/180.0f,c=std::cos(a),s=std::sin(a);Mat4 r=mat4_identity();r.m[0]=c;r.m[2]=-s;r.m[8]=s;r.m[10]=c;return r;}
Mat4 mat4_perspective(float fovy,float aspect,float zn,float zf){
    float f=1.0f/std::tan(fovy*PI/360.0f); Mat4 r{};
    r.m[0]=f/aspect; r.m[5]=f; r.m[10]=(zf+zn)/(zn-zf); r.m[11]=-1.0f; r.m[14]=(2*zf*zn)/(zn-zf); return r;
}
Mat4 mat4_look_at(float ex,float ey,float ez,float tx,float ty,float tz,float ux,float uy,float uz){
    V3 e{ex,ey,ez}, t{tx,ty,tz}, up{ux,uy,uz}; V3 f=norm(sub(t,e)); V3 s=norm(cross(f,up)); V3 u=cross(s,f);
    Mat4 r=mat4_identity();
    r.m[0]=s.x;r.m[4]=s.y;r.m[8]=s.z;
    r.m[1]=u.x;r.m[5]=u.y;r.m[9]=u.z;
    r.m[2]=-f.x;r.m[6]=-f.y;r.m[10]=-f.z;
    r.m[12]=-dot(s,e);r.m[13]=-dot(u,e);r.m[14]=dot(f,e);
    return r;
}
