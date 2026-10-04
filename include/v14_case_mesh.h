#pragma once
#include <cstdint>
#include <array>
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
    // Native JFX materials use the archived shader's baseColor/alpha interface.
    // Legacy mesh builders retain their previous defaults and behavior.
    enum class Material { Legacy, ClearPlastic, Paper, Logo
#ifdef PS3_GAME_ORBIT_FIX32
    , Disc, DiscGlass
#endif
    };
#ifdef PS3_GAME_ORBIT_FIX32
    enum class Joint { Closed, Base, Lid, Disc };
    enum class TextureRole { Cover, DiscLabel, DiscBack, None
#ifdef PS3_GAME_ORBIT_FIX33
    , Inside
#endif
    };
    Joint joint=Joint::Closed;
    TextureRole texture_role=TextureRole::Cover;
#endif
    Material material = Material::Legacy;
    std::array<float,4> color{{0.773f,0.788f,0.804f,0.72f}};
};

struct V14CaseMesh {
    std::vector<V14MeshPart> parts;
    bool jfx = false;
};

// CPU-side geometry for the approved V14 case. Dimensions/camera are not changed here.
V14CaseMesh build_v14_case_mesh(int corner_segments = 5);
