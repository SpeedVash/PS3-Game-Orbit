#include <cassert>
#include <cmath>
#include <cstdio>
#include <string>
#include "project_identity.h"
#include "safe_boot.h"
#include "rsx_renderer_v10.h"
#include "diagnostic_views_fix24.h"

static std::array<float,4> shader_multiply(const std::array<float,16>& rows,
                                         const std::array<float,4>& point){
    std::array<float,4> result{};
    for(int r=0;r<4;++r) for(int c=0;c<4;++c) result[r]+=rows[r*4+c]*point[c];
    return result;
}

int main(int argc,char** argv) {
    assert(argc==2);
    const std::string fixture=std::string(argv[1])+"/pkgfiles/USRDIR/FULL_COVER_FIX22_LABELS.png";
    assert(std::string(ProjectIdentity::AppId)=="PSSPF2401");
    const auto translated=shader_multiply(mat4_shader_rows(mat4_translate(5,-7,11)),{1,2,3,1});
    assert(translated[0]==6 && translated[1]==-5 && translated[2]==14 && translated[3]==1);
    const V14CaseMesh mesh=build_v14_case_mesh(5);
    assert(mesh.parts.size()==6);
    CoverflowState state=make_safe_boot_state("");
    assert(state.visible.size()==1 && state.games.size()==1);
    const auto poses=build_coverflow_render_plan(state,0);
    assert(poses.size()==1 && poses[0].selected);
    const V10FramePlan plan=build_v10_frame_plan(poses,mesh);
    assert(plan.packets.size()==6);
    // Independent perspective expectation for the original FIX20 straight pose.
    const Mat4 vp=mat4_mul(mat4_perspective(29,1280.0f/720,1,2000),
                          mat4_look_at(0,0,425,0,0,0,0,1,0));
    const auto center=shader_multiply(mat4_shader_rows(vp),{0,0,7.5f,1});
    assert(std::fabs(center[3]-417.5f)<0.001f);
    assert(center[0]==0 && center[1]==0 && center[2]>0 && center[2]<center[3]);
    float min_x=1280,min_y=720,max_x=0,max_y=0;
    for(const auto& part:mesh.parts) for(const auto& vertex:part.vertices){
        const auto clip=shader_multiply(mat4_shader_rows(plan.packets[0].mvp),{vertex.x,vertex.y,vertex.z,1});
        assert(clip[3]>300 && clip[3]<450 && clip[2]>0 && clip[2]<clip[3]);
        const float x=(1+clip[0]/clip[3])*640;
        const float y=(1-clip[1]/clip[3])*360;
        assert(x>0 && x<1280 && y>0 && y<720);
        min_x=std::fmin(min_x,x);max_x=std::fmax(max_x,x);
        min_y=std::fmin(min_y,y);max_y=std::fmax(max_y,y);
    }
    assert(min_x>430 && max_x<840 && max_x-min_x>320 && max_x-min_x<370);
    assert(min_y>95 && max_y<620 && max_y-min_y>480 && max_y-min_y<530);
    printf("FIX24 projected V14 bounds: x=%.2f..%.2f y=%.2f..%.2f; all vertices inside viewport\n",
           double(min_x),double(max_x),double(min_y),double(max_y));
    // The previous upload applies the translation column to homogeneous w.
    // This regression check distinguishes it from the intended camera depth.
    const auto old_clip=shader_multiply(plan.packets[0].mvp.m,{0,0,7.62f,1});
    const auto fixed_clip=shader_multiply(mat4_shader_rows(plan.packets[0].mvp),{0,0,7.62f,1});
    assert(std::fabs(old_clip[3]-fixed_clip[3])>2000);
    int textured_surfaces=0;
    for(std::size_t i=0;i<6;++i) {
        assert(plan.packets[i].surface==mesh.parts[i].surface);
        assert(plan.packets[i].game_index==0);
        assert(plan.packets[i].selected);
        for(float value:plan.packets[i].mvp.m) assert(std::isfinite(value));
        if(plan.packets[i].mesh_textured) ++textured_surfaces;
    }
    assert(textured_surfaces==3);
    RsxStage1 rsx;
    assert(rsx.init());
    RsxRendererV10 renderer;
    assert(renderer.init(rsx,mesh,false));
    CoverCache cache(1);
    assert(renderer.sync_visible_covers(state,cache,0));
    assert(renderer.gpu_cover_count()==0 && cache.size()==0);
    assert(renderer.render(state,mesh,0));
    assert(renderer.last_frame_plan().packets.size()==6);
    state.games[0].cover_path=fixture;
    const CoverImage* image=cache.get_or_load(state.games[0]);
    assert(image && image->valid() && image->format==CoverFileFormat::PNG);
    assert(image->width==1100 && image->height==588 && image->looks_like_canonical_full_cover());
    assert(renderer.sync_visible_covers(state,cache,0));
    assert(renderer.gpu_cover_count()==1 && cache.size()==1);
    assert(renderer.render(state,mesh,0));
    assert(renderer.last_stats().draw_calls==6 && renderer.last_stats().visible_cases==1);
    assert(renderer.last_frame_plan().packets.size()==6);
    const auto uv=compute_v10_uv_transform(V14Surface::CoverFront,true);
    assert(uv.scale_u==1.0f && uv.scale_v==1.0f && uv.bias_u==0.0f && uv.bias_v==0.0f);
    for(const auto& view:DiagnosticViewsFix24::Views){
        state.center_yaw_deg=view.yaw_deg;
        state.center_pitch_deg=view.pitch_deg;
        assert(renderer.render(state,mesh,0));
        const auto& frame=renderer.last_frame_plan();
        assert(frame.packets.size()==6 && renderer.gpu_cover_count()==1 && cache.size()==1);
        float xmin=1280,ymin=720,xmax=0,ymax=0;
        for(const auto& part:mesh.parts) for(const auto& vertex:part.vertices){
            const auto clip=shader_multiply(mat4_shader_rows(frame.packets[0].mvp),{vertex.x,vertex.y,vertex.z,1});
            assert(clip[3]>300 && clip[3]<450 && clip[2]>0 && clip[2]<clip[3]);
            const float x=(1+clip[0]/clip[3])*640, y=(1-clip[1]/clip[3])*360;
            assert(x>0 && x<1280 && y>40 && y<680);
            xmin=std::fmin(xmin,x);xmax=std::fmax(xmax,x);
            ymin=std::fmin(ymin,y);ymax=std::fmax(ymax,y);
        }
        const auto index=static_cast<std::size_t>(view.sample_surface);
        const auto& part=mesh.parts[index];
        assert(part.surface==view.sample_surface && part.vertices.size()==4);
        const auto& left=part.vertices[0];const auto& right=part.vertices[1];
        const auto a=shader_multiply(mat4_shader_rows(frame.packets[index].mvp),{left.x,left.y,left.z,1});
        const auto b=shader_multiply(mat4_shader_rows(frame.packets[index].mvp),{right.x,right.y,right.z,1});
        const auto transform=compute_v10_uv_transform(view.sample_surface,true);
        const float ua=left.u*transform.scale_u+transform.bias_u;
        const float ub=right.u*transform.scale_u+transform.bias_u;
        // U must increase from the viewer's left to right on every visible face.
        assert((b[0]/b[3]-a[0]/a[3])*(ub-ua)>0);
        for(const auto& vertex:part.vertices){
            const float u=vertex.u*transform.scale_u+transform.bias_u;
            assert(u>=part.uv.u0-0.0001f && u<=part.uv.u1+0.0001f);
        }
        const auto normal=shader_multiply(mat4_shader_rows(frame.packets[index].model),{left.nx,left.ny,left.nz,0});
        assert(normal[2]>0.5f); // Selected test face points towards the camera.
        printf("VIEW %s host bounds: x=%.2f..%.2f y=%.2f..%.2f; face towards camera, readable U direction\n",
               view.tag,double(xmin),double(xmax),double(ymin),double(ymax));
    }
    const auto back=compute_v10_uv_transform(V14Surface::CoverBack,true);
    assert(back.scale_u==-1 && back.scale_v==1 && back.bias_v==0);
    const auto back_without_full=compute_v10_uv_transform(V14Surface::CoverBack,false);
    assert(back_without_full.scale_u==1 && back_without_full.bias_u==0);
    state.games[0].cover_path.clear();
    assert(renderer.sync_visible_covers(state,cache,0));
    assert(renderer.gpu_cover_count()==0);
    renderer.shutdown();
    rsx.shutdown();
    puts("FIX24 host: shader row layout, projection, canonical PNG decode/upload/ownership and six-packet plan OK");
    puts("HOST ONLY: native GPU draws, texture sampling and TV visibility remain untested");
}
