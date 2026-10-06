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
    enum class Joint { Closed, Base, Lid, Disc
#ifdef PS3_GAME_ORBIT_FIX37
    , Spine
#endif
    };
    enum class TextureRole { Cover, DiscLabel, DiscBack, None
#ifdef PS3_GAME_ORBIT_FIX33
    , Inside
#endif
    };
    Joint joint=Joint::Closed;
    TextureRole texture_role=TextureRole::Cover;
#ifdef PS3_GAME_ORBIT_FIX37
    // Only the small spine ranges deform. Rest coordinates never change.
    std::vector<V14Vertex> flex_rest;
    std::vector<float> flex_offsets;
    bool flex_inside=false;
#ifdef PS3_GAME_ORBIT_FIX39
    // Rounded shell references and normal welds are built once. Animation
    // reuses these arrays; no maps or allocations are needed per frame.
    std::vector<std::array<float,3>> flex_shell_anchors; // curve X/Z, lid weight
    std::vector<std::uint16_t> flex_normal_groups;
    std::vector<std::array<float,3>> flex_normal_sums;
#endif
#endif
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
