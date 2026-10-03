#pragma once
#include <cstdint>
#include <vector>
#include "full_cover_layout.h"

struct V14Vertex {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
};

enum class V14Surface : std::uint8_t {
    ShellFront,
    ShellBack,
    ShellEdge,
    CoverFront,
    CoverBack,
    CoverSpine
};

struct V14MeshPart {
    V14Surface surface = V14Surface::ShellEdge;
    std::vector<V14Vertex> vertices;
    std::vector<std::uint16_t> indices;
    bool textured = false;
    UVRect uv{0,0,1,1};
};

struct V14CaseMesh {
    std::vector<V14MeshPart> parts;
};

// CPU-side geometry for the approved V14 case. Dimensions/camera are not changed here.
V14CaseMesh build_v14_case_mesh(int corner_segments = 5);
