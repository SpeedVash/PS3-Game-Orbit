#pragma once
#include <array>
#include "v14_case_mesh.h"

// Adapt the frozen native mesh to the approved full-cover layout. The plastic
// and front remain unchanged. Back, two paper folds and spine form a connected
// sheet; all three parts sample the same BACK|SPINE|FRONT image.
V14CaseMesh build_full_cover_case_fix26(int corner_segments=5);
std::array<float,3> full_cover_surface_point_fix26(const V14MeshPart& part,float u,float v);
