#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include "boot_visual.h"
#include "cover_cache.h"
#include "math3d.h"
#include "render_plan.h"
#include "rsx_stage1.h"
#include "v14_case_mesh.h"
#include "library_hud_fix28.h"

struct V10DrawPacket {
    int game_index=-1;
    int relative_slot=0;
    V14Surface surface=V14Surface::ShellEdge;
    Mat4 model{};
    Mat4 mvp{};
    float alpha=1.0f;
    bool mesh_textured=false;
    bool selected=false;
    // Explicit mesh range permits two passes of one plastic mesh without
    // duplicating or simplifying its original geometry.
    std::size_t mesh_part=static_cast<std::size_t>(-1);
    unsigned plastic_pass=0; // 1: far/back faces; 2: near/front faces.
};

struct V10FramePlan {
    std::vector<V10DrawPacket> packets;
    int screen_width=1280;
    int screen_height=720;
};

struct V10UvTransform {
    float scale_u=1.0f;
    float scale_v=1.0f;
    float bias_u=0.0f;
    float bias_v=0.0f;
};

V10UvTransform compute_v10_uv_transform(V14Surface surface,bool full_cover_texture);
V10UvTransform compute_native_uv_transform(const V14CaseMesh& mesh,V14Surface surface,bool full_cover_texture);
std::vector<std::size_t> native_submission_order(const V10FramePlan& plan,const V14CaseMesh& mesh);
std::size_t native_packet_part(const V10DrawPacket& packet,std::size_t packet_index,const V14CaseMesh& mesh);

struct V10SubmissionStats {
    std::size_t draw_calls=0;
    std::size_t textured_draw_calls=0;
    std::size_t shell_draw_calls=0;
    std::size_t visible_cases=0;
    std::size_t uploaded_geometry_bytes=0;
    std::size_t hud_draw_calls=0;
    std::size_t background_draw_calls=0;
};

V10FramePlan build_v10_frame_plan(const std::vector<CasePose>& poses,
                                   const V14CaseMesh& mesh,
                                   int screen_width=1280,
                                   int screen_height=720);

V10SubmissionStats summarize_v10_submission(const V10FramePlan& plan,
                                             const V14CaseMesh& mesh);

class RsxRendererV10 {
public:
    bool init(RsxStage1& stage1,const V14CaseMesh& mesh,bool present_boot_visuals=true);
    void shutdown();
    bool ready() const { return ready_; }
    const std::string& last_error() const { return last_error_; }
    BootVisualStage boot_visual_stage() const { return boot_visual_stage_; }

    // Keeps GPU copies only for the selected case and one distinct neighbor. CPU encoded covers
    // remain owned by CoverCache. Full covers texture BACK|SPINE|FRONT; ICON0 is front-only.
    bool sync_visible_covers(const CoverflowState& state,CoverCache& cache,int radius=2);
    void clear_cover_textures();

    bool render(const CoverflowState& state,const V14CaseMesh& mesh,int radius=2,float selected_scale=0.72f);
    // Texture/geometry changes are allowed only after the stream controller has
    // acknowledged the previous frame. Main performs this before begin_frame().
    bool set_library_hud(const LibraryHudLinesFix28& lines);
    bool prepare_orbit_background();
    void show_runtime_failure();
    // FIX9: one-shot real-hardware color-clear present probe. No coverflow draw calls.
    bool run_first_present_probe();
    // FIX10: reproduce the old boot visual clear mask (color + depth) but deliberately
    // omit gcmSetWaitFlip(). This isolates depth clear from the wait-flip command.
    bool run_depth_clear_probe();
    // FIX11: executes the real boot-visual presentation path after replacing
    // gcmSetWaitFlip with bounded CPU polling. Used only by the forced diagnostic.
    bool run_boot_visual_polling_probe();
    // FIX12/FIX13: one-shot draw probe for a single untextured V14 shell part.
    // Uses the proven surface/clear/flip polling path and issues exactly one RSX indexed draw.
    bool run_single_shell_draw_probe();
    // FIX20: fill both renderer buffers magenta and reproduce the FIX19
    // initial flip + 60 alternating flips before submitting the draw probe.
    bool run_cpu_framebuffer_probe();

#if defined(__PSL1GHT__) && defined(PS3_SP_LOADER_FIX28)
    void set_diagnostic_view(const char* label,V14Surface sample_surface){
        diagnostic_view_label_=label ? label : "";
        diagnostic_sample_surface_=sample_surface;
    }
    void set_front_face_clockwise(bool value){ front_face_clockwise_=value; }
    unsigned front_green_samples(const V14CaseMesh& mesh) const;
#endif

    const V10FramePlan& last_frame_plan() const { return last_plan_; }
    const V10SubmissionStats& last_stats() const { return last_stats_; }
    std::size_t gpu_cover_count() const { return covers_.size(); }
    bool has_cover_texture(int index) const { const auto* t=texture_for_game(index);return t && t->uploaded; }
    bool has_full_cover_texture(int index) const { const auto* t=texture_for_game(index);return t && t->uploaded && t->full_cover; }

private:
    struct GpuCoverRecord {
        std::string path;
        GameCoverKind kind=GameCoverKind::None;
        unsigned orientation=1;
        GpuTextureStage1 texture{};
    };

    RsxStage1* stage1_=nullptr;
    std::unordered_map<int,GpuCoverRecord> covers_;
    V10FramePlan last_plan_{};
    V10SubmissionStats last_stats_{};
    bool ready_=false;
    BootVisualStage boot_visual_stage_=BootVisualStage::None;
    std::string last_error_;
    GpuTextureStage1 hud_texture_{};
    GpuTextureStage1 background_texture_{};
    LibraryHudLinesFix28 hud_lines_{};

    void release_cover_record(GpuCoverRecord& rec);
    const GpuTextureStage1* texture_for_game(int game_index) const;

#ifdef __PSL1GHT__
    struct GpuMeshPart {
        V14Surface surface=V14Surface::ShellEdge;
        bool textured=false;
        void* vertices=nullptr;
        void* indices=nullptr;
        std::uint32_t vertex_offset=0;
        std::uint32_t index_offset=0;
        std::uint32_t vertex_count=0;
        std::uint32_t index_count=0;
    };

    bool init_display_ps3();
    bool init_shaders_ps3();
    bool upload_geometry_ps3(const V14CaseMesh& mesh);
    bool draw_frame_ps3(const V10FramePlan& plan,const V14CaseMesh& mesh);
    bool draw_library_hud_ps3();
    bool draw_orbit_background_ps3();
    void release_geometry_ps3();
    void setup_texture_ps3(const GpuTextureStage1& tex);
    bool present_boot_visual_ps3(BootVisualStage stage);
    bool wait_for_flip_polling_ps3(const char* tag,unsigned max_polls=10000u);

    void* color_buffer_[2]{nullptr,nullptr};
    std::uint32_t color_offset_[2]{0,0};
    void* depth_buffer_=nullptr;
    std::uint32_t depth_offset_=0;
    std::uint32_t color_pitch_=0;
    std::uint32_t depth_pitch_=0;
    int current_buffer_=0;
    int width_=1280;
    int height_=720;

    const void* vertex_program_=nullptr;
    const void* fragment_program_=nullptr;
    const void* vertex_ucode_=nullptr;
    void* fragment_ucode_=nullptr;
    std::uint32_t fragment_offset_=0;

    const void* vp_mvp_=nullptr;
    const void* vp_model_=nullptr;
    const void* vp_uv_transform_=nullptr;
    const void* fp_base_color_=nullptr;
    const void* fp_use_texture_=nullptr;
    const void* fp_alpha_=nullptr;
    int attr_position_=-1;
    int attr_normal_=-1;
    int attr_uv_=-1;

    std::vector<GpuMeshPart> gpu_mesh_;
    GpuMeshPart hud_mesh_{};
    GpuMeshPart background_mesh_{};
#ifdef PS3_SP_LOADER_FIX28
    unsigned diagnostic_frame_number_=0;
    bool front_face_clockwise_=false;
    std::string diagnostic_view_label_;
    V14Surface diagnostic_sample_surface_=V14Surface::CoverFront;
#endif
#endif
};
