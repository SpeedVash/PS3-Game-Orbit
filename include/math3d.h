#pragma once
#include <array>

struct Mat4 {
    // CPU matrices are column-major and multiply column vectors.
    std::array<float,16> m{};
};

// The archived Cg shader uses mul(matrix, vector). PSL1GHT uploads the
// supplied float4 vectors verbatim, so its constants must contain matrix rows.
std::array<float,16> mat4_shader_rows(const Mat4& matrix);
Mat4 mat4_identity();
Mat4 mat4_mul(const Mat4& a,const Mat4& b);
Mat4 mat4_translate(float x,float y,float z);
Mat4 mat4_scale(float s);
Mat4 mat4_rotate_x_deg(float deg);
Mat4 mat4_rotate_y_deg(float deg);
Mat4 mat4_perspective(float fovy_deg,float aspect,float znear,float zfar);
Mat4 mat4_look_at(float ex,float ey,float ez,float tx,float ty,float tz,float ux,float uy,float uz);
