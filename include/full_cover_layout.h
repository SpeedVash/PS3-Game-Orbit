#pragma once

// Canonical full-cover layout for the approved V14 case:
// [ BACK 130 mm ][ SPINE 15 mm ][ FRONT 130 mm ]
// Height: 147 mm
// Total useful cover aspect: 275 : 147
struct UVRect {
    float u0, v0, u1, v1;
};

namespace FullCoverLayout {
    constexpr float BackWidthMm  = 130.0f;
    constexpr float SpineWidthMm = 15.0f;
    constexpr float FrontWidthMm = 130.0f;
    constexpr float HeightMm     = 147.0f;
    constexpr float TotalWidthMm = BackWidthMm + SpineWidthMm + FrontWidthMm;

    constexpr float BackEndU   = BackWidthMm / TotalWidthMm;                  // 130/275
    constexpr float SpineEndU  = (BackWidthMm + SpineWidthMm) / TotalWidthMm; // 145/275

    constexpr UVRect Back  {0.0f,      0.0f, BackEndU,  1.0f};
    constexpr UVRect Spine {BackEndU,  0.0f, SpineEndU, 1.0f};
    constexpr UVRect Front {SpineEndU, 0.0f, 1.0f,       1.0f};
}
