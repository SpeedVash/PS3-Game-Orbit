#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include "project_identity.h"
#include "safe_boot.h"
#include "rsx_renderer_v10.h"
#include "diagnostic_views_fix24.h"
#include "case_render_fix25.h"
#include "inspect_case_fix25.h"
#include "full_cover_case_fix26.h"
#include "image_decode.h"

static std::array<float,4> shader_multiply(const std::array<float,16>& rows,
                                         const std::array<float,4>& point){
    std::array<float,4> result{};
    for(int r=0;r<4;++r) for(int c=0;c<4;++c) result[r]+=rows[r*4+c]*point[c];
    return result;
}

static void check_continuous_cover(const std::string& fixture){
    const auto original=build_v14_case_mesh(5);
    const auto mesh=build_full_cover_case_fix26(5);
    assert(mesh.parts.size()==6);
    // Preserve the physically validated plastic and front; only fix the wrap.
    for(unsigned i=0;i<4;++i){
        assert(mesh.parts[i].indices==original.parts[i].indices);
        assert(mesh.parts[i].vertices.size()==original.parts[i].vertices.size());
        assert(std::memcmp(mesh.parts[i].vertices.data(),original.parts[i].vertices.data(),
                           mesh.parts[i].vertices.size()*sizeof(V14Vertex))==0);
    }
    const auto& front=mesh.parts[3];const auto& back=mesh.parts[4];const auto& spine=mesh.parts[5];
    // This is the rejected old gap: back's spine-side edge was 5 mm away.
    assert(original.parts[4].vertices[0].x-original.parts[3].vertices[0].x==5);
    assert(back.vertices[0].x==front.vertices[0].x);
    assert(back.vertices[1].x==front.vertices[1].x);
    assert(back.vertices[1].x-back.vertices[0].x==130);
    assert(spine.vertices.size()==24 && spine.indices.size()==66);
    auto same_join=[](const V14Vertex& a,const V14Vertex& b,float effective_u){
        assert(a.x==b.x && a.y==b.y && a.z==b.z);
        assert(a.nx==b.nx && a.ny==b.ny && a.nz==b.nz);
        assert(std::fabs(a.u-effective_u)<1e-6f && a.v==b.v);
    };
    const auto back_uv=compute_v10_uv_transform(V14Surface::CoverBack,true);
    same_join(spine.vertices[0],back.vertices[0],back.vertices[0].u*back_uv.scale_u+back_uv.bias_u);
    same_join(spine.vertices[1],back.vertices[3],back.vertices[3].u*back_uv.scale_u+back_uv.bias_u);
    same_join(spine.vertices[22],front.vertices[0],front.vertices[0].u);
    same_join(spine.vertices[23],front.vertices[3],front.vertices[3].u);
    // Ordered atlas coordinates across the real folded surface, no reversed strip.
    for(std::size_t i=0;i<spine.vertices.size();i+=2){
        const auto& a=spine.vertices[i];const auto& b=spine.vertices[i+1];
        assert(a.x==b.x && a.z==b.z && a.u==b.u && a.y-b.y==147);
        assert(a.v==0 && b.v==1 && a.u>=130.0f/275 && a.u<=145.0f/275);
        assert(std::fabs(a.nx*a.nx+a.nz*a.nz-1)<1e-5f);
        if(i){assert(a.z>spine.vertices[i-2].z);assert(a.u>spine.vertices[i-2].u);}
    }
    const auto center=full_cover_surface_point_fix26(spine,0.5f,0.5f);
    assert(std::fabs(center[0]+67.62f)<1e-4f && std::fabs(center[1]+6.5f)<1e-4f);
    assert(std::fabs(center[2])<1e-4f);
    assert(std::fabs((spine.vertices[0].u+spine.vertices[22].u)*0.5f-0.5f)<1e-6f);
    // Every folded triangle already faces outwards; the old spine needed reversal.
    assert(CaseRenderFix25::outward_indices(spine)==spine.indices);
    for(std::size_t i=0;i<spine.indices.size();i+=3){
        const auto& a=spine.vertices[spine.indices[i]];
        const auto& b=spine.vertices[spine.indices[i+1]];
        const auto& c=spine.vertices[spine.indices[i+2]];
        const float bx=b.x-a.x,by=b.y-a.y,bz=b.z-a.z;
        const float cx=c.x-a.x,cy=c.y-a.y,cz=c.z-a.z;
        const float dot=(by*cz-bz*cy)*(a.nx+b.nx+c.nx)+(bz*cx-bx*cz)*(a.ny+b.ny+c.ny)+(bx*cy-by*cx)*(a.nz+b.nz+c.nz);
        assert(dot>0.01f);
    }
    // The same two image ribbons must cross every column, including both joins.
    auto image=load_cover_file(fixture,GameCoverKind::FullCover);
    DecodedImageRGBA rgba;std::string error;
    assert(decode_cover_rgba(image,rgba,error) && rgba.valid());
    for(int y:{97,489}) for(int x=0;x<1100;++x){
        const auto p=std::size_t(y)*rgba.pitch+std::size_t(x)*4;
        assert(rgba.rgba[p]==235 && rgba.rgba[p+1]==241 && rgba.rgba[p+2]==246 && rgba.rgba[p+3]==255);
    }
    unsigned poses=0;
    auto state=make_safe_boot_state("");
    for(float scale:{0.55f,0.72f,0.80f}) for(float pitch:{-28.0f,-5.0f,22.0f}) for(int yaw=0;yaw<360;yaw+=5){
        state.center_yaw_deg=float(yaw);state.center_pitch_deg=pitch;
        auto placement=build_coverflow_render_plan(state,0);placement[0].scale=scale;
        const auto plan=build_v10_frame_plan(placement,mesh);
        assert(plan.packets.size()==6);
        for(const auto& part:mesh.parts) for(const auto& v:part.vertices){
            const auto clip=shader_multiply(mat4_shader_rows(plan.packets[0].mvp),{v.x,v.y,v.z,1});
            assert(clip[3]>250 && clip[2]>0 && clip[2]<clip[3]);
            const float x=(1+clip[0]/clip[3])*640,y=(1-clip[1]/clip[3])*360;
            assert(x>0 && x<1280 && y>0 && y<720);
        }
        ++poses;
    }
    assert(poses==648);
    puts("FIX26 wrap: old 5 mm gap removed; both position/UV/normal joins exact; center U=0.5; 22 outward fold triangles; 648 poses fit; continuous image ribbons decoded");
}

int main(int argc,char** argv) {
    assert(argc==2);
    const std::string fixture=std::string(argv[1])+"/pkgfiles/USRDIR/FULL_COVER_FIX26_CONTINUA.png";
    check_continuous_cover(fixture);
    assert(std::string(ProjectIdentity::AppId)=="PSSPF2801");
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
    printf("FIX26 projected V14 bounds: x=%.2f..%.2f y=%.2f..%.2f; all vertices inside viewport\n",
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
    unsigned repaired=0;
    for(const auto& part:mesh.parts){
        const auto fixed=CaseRenderFix25::outward_indices(part);
        assert(fixed.size()==part.indices.size());
        for(std::size_t i=0;i<fixed.size();i+=3){
            repaired+=fixed[i+1]!=part.indices[i+1];
            const auto& a=part.vertices[fixed[i]];
            const auto& b=part.vertices[fixed[i+1]];
            const auto& c=part.vertices[fixed[i+2]];
            const float bx=b.x-a.x,by=b.y-a.y,bz=b.z-a.z;
            const float cx=c.x-a.x,cy=c.y-a.y,cz=c.z-a.z;
            const float dot=(by*cz-bz*cy)*(a.nx+b.nx+c.nx)+(bz*cx-bx*cz)*(a.ny+b.ny+c.ny)+(bx*cy-by*cx)*(a.nz+b.nz+c.nz);
            assert(dot>=-0.0001f);
        }
    }
    assert(repaired==48);
    for(float scale:{0.55f,0.72f,0.80f}) for(float pitch:{-28.0f,-5.0f,22.0f}) for(int yaw=0;yaw<360;yaw+=5){
        state.center_yaw_deg=float(yaw);state.center_pitch_deg=pitch;
        assert(renderer.render(state,mesh,0,scale));
        const auto& frame=renderer.last_frame_plan();
        const auto order=CaseRenderFix25::submission_order(frame,mesh);
        assert(order.size()==6);
        for(int i=0;i<6;++i) assert(frame.packets[order[i]].mesh_textured==(i<3));
        for(const auto& part:mesh.parts) for(const auto& vertex:part.vertices){
            const auto clip=shader_multiply(mat4_shader_rows(frame.packets[0].mvp),{vertex.x,vertex.y,vertex.z,1});
            assert(clip[3]>250 && clip[2]>0 && clip[2]<clip[3]);
            const float x=(1+clip[0]/clip[3])*640,y=(1-clip[1]/clip[3])*360;
            assert(x>0 && x<1280 && y>0 && y<720);
        }
    }
    for(float yaw:{0.0f,180.0f}){
        state.center_yaw_deg=yaw;state.center_pitch_deg=0;
        assert(renderer.render(state,mesh,0));
        const auto order=CaseRenderFix25::submission_order(renderer.last_frame_plan(),mesh);
        assert(order[3]==(yaw==0 ? 1u : 0u));
        assert(order[5]==(yaw==0 ? 0u : 1u));
    }
    InspectCaseFix25 inspect;
    state.center_yaw_deg=28;state.center_pitch_deg=-5;
    InputFrame input;
    for(int i=0;i<721;++i) assert(inspect.update(state,input,1.0f/60));
    assert(!inspect.automatic() && std::fabs(state.center_yaw_deg-28)<0.01f);
    input.connected=true;input.cross.pressed=true;
    assert(inspect.update(state,input,0) && inspect.automatic());
    input.cross.pressed=false;input.right.held=true;
    assert(inspect.update(state,input,0.1f) && !inspect.automatic());
    assert(std::fabs(state.center_yaw_deg-40)<0.01f);
    input.right.held=false;input.triangle.pressed=true;
    assert(inspect.update(state,input,0));
    assert(state.center_yaw_deg==152 && state.center_pitch_deg==-5);
    input.triangle.pressed=false;input.down.held=true;input.r2.held=true;
    for(int i=0;i<50;++i) assert(inspect.update(state,input,0.1f));
    assert(state.center_pitch_deg==22 && inspect.scale()==0.80f);
    input.down.held=input.r2.held=false;input.up.held=input.l2.held=true;
    for(int i=0;i<50;++i) assert(inspect.update(state,input,0.1f));
    assert(state.center_pitch_deg==-28 && inspect.scale()==0.55f);
    input.up.held=input.l2.held=false;input.r3.pressed=true;
    assert(inspect.update(state,input,0));
    assert(state.center_yaw_deg==28 && state.center_pitch_deg==-5 && inspect.scale()==0.72f);
    input.r3.pressed=false;input.circle.pressed=true;
    assert(!inspect.update(state,input,0));
    puts("FIX26 host: 648 rotation/tilt/zoom poses fit the viewport; 48 triangle orientations repaired; opaque/transparent order and controls passed");
    const auto back=compute_v10_uv_transform(V14Surface::CoverBack,true);
    assert(back.scale_u==-1 && back.scale_v==1 && back.bias_v==0);
    const auto back_without_full=compute_v10_uv_transform(V14Surface::CoverBack,false);
    assert(back_without_full.scale_u==1 && back_without_full.bias_u==0);
    state.games[0].cover_path.clear();
    assert(renderer.sync_visible_covers(state,cache,0));
    assert(renderer.gpu_cover_count()==0);
    renderer.shutdown();
    rsx.shutdown();
    puts("FIX26 host: shader row layout, projection, canonical PNG decode/upload/ownership and six-packet plan OK");
    puts("HOST ONLY: native GPU draws, texture sampling and TV visibility remain untested");
}
