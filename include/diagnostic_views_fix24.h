#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include "v14_case_mesh.h"

namespace DiagnosticViewsFix24 {
struct View {
    const char* tag;
    float yaw_deg;
    float pitch_deg;
    std::int64_t hold_us;
    V14Surface sample_surface;
};
inline constexpr std::array<View,3> Views{{
    {"FRONT",28.0f,-5.0f,4000000,V14Surface::CoverFront},
    {"SPINE",90.0f,-5.0f,4000000,V14Surface::CoverSpine},
    {"BACK",180.0f,-5.0f,6000000,V14Surface::CoverBack}
}};
// FIX23 measured 3664 bytes per textured frame, including its 116-byte flip.
// Require room for three bounded frames without command-segment rollover.
inline constexpr std::size_t FrameCommandBudgetBytes=4096;
inline constexpr std::size_t InitialFrameGuardBytes=8192;
inline constexpr std::size_t RequiredInitialBytes=
    (Views.size()-1)*FrameCommandBudgetBytes+InitialFrameGuardBytes;
}
