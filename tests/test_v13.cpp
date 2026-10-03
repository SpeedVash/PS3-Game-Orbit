#include <cassert>
#include <cmath>
#include <cstdio>
#include "coverflow_state.h"
#include "cover_cache.h"
#include "render_plan.h"
#include "v14_case_mesh.h"
#include "rsx_renderer_v10.h"
#include "boot_visual.h"

static bool near(float a,float b,float e=1e-4f){return std::fabs(a-b)<e;}

int main(){
    CoverflowState s;
    for(int i=0;i<7;++i){
        GameEntry g;
        g.title="Game"+std::to_string(i);
        g.title_id="TEST"+std::to_string(1000+i);
        s.games.push_back(g);
        s.visible.push_back(i);
    }
    s.selected=3;
    s.center_yaw_deg=37.0f;
    s.center_pitch_deg=-8.0f;

    const auto poses=build_coverflow_render_plan(s,2);
    const auto mesh=build_v14_case_mesh(5);
    const auto fp=build_v10_frame_plan(poses,mesh,1280,720);
    const auto stats=summarize_v10_submission(fp,mesh);

    assert(poses.size()==5);
    assert(mesh.parts.size()==6);
    assert(fp.packets.size()==30);
    assert(stats.draw_calls==30);
    assert(stats.visible_cases==5);
    assert(stats.textured_draw_calls==15);
    assert(stats.shell_draw_calls==15);
    assert(stats.uploaded_geometry_bytes>0);

    // ICON0/front-only fallback must expand the canonical front strip to the full image.
    const auto full_uv=compute_v10_uv_transform(V14Surface::CoverFront,true);
    assert(near(full_uv.scale_u,1.0f) && near(full_uv.bias_u,0.0f));
    const auto icon_uv=compute_v10_uv_transform(V14Surface::CoverFront,false);
    assert(icon_uv.scale_u>2.0f && icon_uv.bias_u<0.0f);
    const float remapped_u0=FullCoverLayout::Front.u0*icon_uv.scale_u+icon_uv.bias_u;
    const float remapped_u1=FullCoverLayout::Front.u1*icon_uv.scale_u+icon_uv.bias_u;
    assert(near(remapped_u0,0.0f) && near(remapped_u1,1.0f));

    int selected_packets=0;
    for(const auto&p:fp.packets) if(p.selected) ++selected_packets;
    assert(selected_packets==6);
    const auto& center=poses[2];
    assert(center.selected && near(center.yaw_deg,37.0f) && near(center.pitch_deg,-8.0f));
    for(float v:fp.packets.front().mvp.m) assert(std::isfinite(v));

    // Host renderer path verifies initialization, draw plan and visible-cover ownership.
    RsxStage1 rsx;
    assert(rsx.init());
    RsxRendererV10 renderer;
    assert(renderer.init(rsx,mesh));
    assert(renderer.boot_visual_stage()==BootVisualStage::Ready);
    assert(boot_visual_color(BootVisualStage::FatalShader)!=boot_visual_color(BootVisualStage::FatalGeometry));
    assert(std::string(boot_visual_name(BootVisualStage::Ready))=="READY");
    CoverCache cache(7);
    assert(renderer.sync_visible_covers(s,cache,2)); // no paths => no GPU cover allocations
    assert(renderer.gpu_cover_count()==0);
    assert(renderer.render(s,mesh));
    assert(renderer.last_frame_plan().packets.size()==30);
    assert(renderer.last_stats().draw_calls==30);
    renderer.show_runtime_failure();
    assert(renderer.boot_visual_stage()==BootVisualStage::FatalRuntime);
    renderer.shutdown();
    rsx.shutdown();

    std::puts("V13 RSX draw-submission + boot-visual tests: OK");
    return 0;
}
