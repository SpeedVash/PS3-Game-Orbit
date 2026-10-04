#pragma once
#include "v14_case_mesh.h"
#include "math3d.h"
namespace CaseAnimationFix32 {
inline constexpr float HingeX=-68.0f;
float smooth(float phase,float begin,float end);
float lid_angle(float phase);
float disc_slide(float phase);
Mat4 joint_transform(V14MeshPart::Joint joint,float phase);
V14CaseMesh build(const V14CaseMesh& original);
}
