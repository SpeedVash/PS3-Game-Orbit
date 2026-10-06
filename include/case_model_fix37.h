#pragma once
#include "v14_case_mesh.h"
#include "math3d.h"
namespace CaseModelFix37 {
inline constexpr float HingeX=-66.163f;
inline constexpr float BackHingeZ=-6.69577f;
inline constexpr float FrontHingeZ=6.62623f;
inline constexpr float InsideZ=5.02f;
#ifdef PS3_GAME_ORBIT_FIX38
inline constexpr float DiscLabelRadius=59.5f;
inline constexpr float DiscLabelInnerRadius=8.f;
#else
inline constexpr float DiscLabelRadius=59.6f;
inline constexpr float DiscLabelInnerRadius=7.5f;
#endif
inline constexpr float DiscHoleRadius=7.5f;
inline constexpr float DiscRadius=60.f;
V14CaseMesh build(const V14CaseMesh& original);
float opening_angle(float phase);
Mat4 joint_transform(V14MeshPart::Joint joint,float phase);
void deform_spine(V14CaseMesh& mesh,float phase);
}
