#!/usr/bin/env python3
"""Exercise the actual native draw body using JFX geometry and API stubs."""
from pathlib import Path
import re,subprocess,tempfile,hashlib
ROOT=Path(__file__).resolve().parent.parent
source=(ROOT/'src/rsx_renderer_v10.cpp').read_text()
def function(signature):
    start=source.index(signature);begin=source.index('{',start);end=begin+1;depth=1
    while depth:depth+=(source[end]=='{')-(source[end]=='}');end+=1
    return source[start:end]
draw=function('bool RsxRendererV10::draw_frame_ps3(')
hud=function('bool RsxRendererV10::draw_library_hud_ps3(')
helpers='\n'.join(function(s) for s in ('V10UvTransform compute_v10_uv_transform(','V10UvTransform compute_native_uv_transform(',
 'std::size_t native_packet_part(','std::vector<std::size_t> native_submission_order(',
 'const GpuTextureStage1* RsxRendererV10::texture_for_game(','static bool surface_uses_texture('))
prefix=(ROOT/'scripts/jfx-native-api-stubs.inc').read_text()
constants=sorted(set(re.findall(r'\bGCM_[A-Z0-9_]+\b',draw+hud)))
defines=''.join(f'constexpr u32 {n}={0 if n=="GCM_FALSE" else 1 if n=="GCM_TRUE" else i+2};\n' for i,n in enumerate(constants))
prefix=prefix.replace('struct gcmContextData {',defines+'struct gcmContextData {')
suffix=r'''
static void reset(){
 ctx={commands,commands,commands+10000};draws=loads=version=loaded_version=uploads=0;
 flip_calls=padding_on_clear=plastic_draws=paper_draws=logo_draws=0;
 texture_bound=false;wait_ok=true;flip_rc=0;cull=false;far_seen=near_seen=false;
}
int main(){
 RsxStage1 stage;stage.context_=&ctx;RsxRendererV10 r;r.stage1_=&stage;
 r.color_buffer_[0]=r.color_buffer_[1]=pixels.data();r.depth_buffer_=pixels.data();r.color_pitch_=5120;
 r.vertex_program_=&vp;r.fragment_program_=&fp;r.vp_mvp_=&mvp_param;r.vp_model_=&model_param;r.vp_uv_transform_=&uv_param;
 r.fp_base_color_=&color_param;r.fp_use_texture_=&texture_param;r.fp_alpha_=&alpha_param;
 const auto mesh=JfxCaseFix29::build();
 for(const auto& part:mesh.parts){RsxRendererV10::GpuMeshPart gp;gp.surface=part.surface;gp.vertices=gp.indices=pixels.data();gp.vertex_count=part.vertices.size();gp.index_count=part.indices.size();r.gpu_mesh_.push_back(gp);}
 r.hud_texture_.uploaded=true;r.hud_texture_.width=1024;r.hud_texture_.height=192;
 r.hud_mesh_.vertices=r.hud_mesh_.indices=pixels.data();r.hud_mesh_.index_count=12;
 RsxRendererV10::GpuCoverRecord record;record.texture.uploaded=record.texture.full_cover=true;record.texture.gpu_offset=10;r.covers_[0]=record;
 record.texture.gpu_offset=20;r.covers_[1]=record;
 const auto vp_matrix=mat4_mul(mat4_perspective(29,1280.0f/720,1,2000),mat4_look_at(0,2,425,0,0,0,0,1,0));
 const auto side_model=mat4_mul(mat4_translate(185,0,-70),mat4_mul(mat4_rotate_x_deg(-3),mat4_mul(mat4_rotate_y_deg(-36),mat4_scale(.8f))));
 const auto side_mvp=mat4_mul(vp_matrix,side_model);
 for(int row=0;row<4;++row) for(int col=0;col<4;++col){neighbor_model[row*4+col]=side_model.m[col*4+row];neighbor_mvp[row*4+col]=side_mvp.m[col*4+row];}
 case_draws=12;V10FramePlan last;
 for(int yaw:{0,28,90,152,180,270,330}){
  const auto model=mat4_mul(mat4_translate(0,0,55),mat4_mul(mat4_rotate_x_deg(-5),mat4_mul(mat4_rotate_y_deg(float(yaw)),mat4_scale(.72f))));
  const auto mvp=mat4_mul(vp_matrix,model);
  for(int row=0;row<4;++row) for(int col=0;col<4;++col){expected_model[row*4+col]=model.m[col*4+row];expected_mvp[row*4+col]=mvp.m[col*4+row];}
  last.packets.clear();
  for(int game=0;game<2;++game) for(std::size_t i=0;i<mesh.parts.size();++i){
   const auto& part=mesh.parts[i];V10DrawPacket p;p.game_index=game;p.surface=part.surface;p.mesh_textured=part.textured;p.mesh_part=i;
   p.model=game ? side_model : model;p.mvp=game ? side_mvp : mvp;p.alpha=game ? .76f : 1;
   if(i==0){p.plastic_pass=1;last.packets.push_back(p);p.plastic_pass=2;}last.packets.push_back(p);
  }
  for(int mode:{1,2,0}){
   texture_mode=mode;for(auto& kv:r.covers_) kv.second.texture.full_cover=mode!=2;
   if(mode==0){for(auto& kv:r.covers_) kv.second.texture.uploaded=false;}
   reset();assert(r.draw_frame_ps3(last,mesh));
   assert(draws==13 && loads==13 && flip_calls==1 && paper_draws==6 && logo_draws==2 && plastic_draws==4 && far_seen && near_seen);
   assert(r.last_stats_.textured_draw_calls==unsigned(mode==1 ? 6 : mode==2 ? 2 : 0));
   for(auto& kv:r.covers_) kv.second.texture.uploaded=true;
  }
 }
 puts("PASS: actual JFX native body at seven poses: two independent covers/matrices, original back UVs, front-only and absent artwork, opaque paper/logo, both neutral transparent plastic passes and one HUD");
 texture_mode=1;reset();ctx.end=commands+100;assert(!r.draw_frame_ps3(last,mesh) && draws==0);
 reset();auto malformed=last;malformed.packets[0].mesh_part=99;assert(!r.draw_frame_ps3(malformed,mesh) && draws==0);
 reset();malformed=last;malformed.packets[1].plastic_pass=0;assert(!r.draw_frame_ps3(malformed,mesh) && draws==0);
 reset();padding_on_clear=2800;assert(!r.draw_frame_ps3(last,mesh) && flip_calls==0);
 puts("PASS: insufficient command space, invalid mesh range, missing plastic pass and frame over budget stop before flip");
 puts("LIMIT: API simulation does not emulate RSX sampling/cache, word counts or physical video output");
}
'''
variants={'corrected':draw,'blue-opaque-plastic':draw.replace('mesh.jfx ? part.color.data() : shell_color','shell_color'),
 'missing-far-face-pass':draw.replace('packet.plastic_pass==1 ? GCM_CULL_FRONT : GCM_CULL_BACK','GCM_CULL_BACK'),
 'wrong-neighbor-texture':draw.replace('texture_for_game(packet.game_index)','texture_for_game(0)'),
 'missing-fp-reload':draw.replace('rsxLoadFragmentProgramLocation(ctx,fp,fragment_offset_,GCM_LOCATION_RSX);','')}
print('NATIVE_DRAW_SOURCE_SHA256: '+hashlib.sha256(source.encode()).hexdigest(),flush=True)
with tempfile.TemporaryDirectory(prefix='fix29-jfx-draw-') as directory:
 for name,body in variants.items():
    path=Path(directory);cpp=path/(name+'.cpp');binary=path/name;cpp.write_text(prefix+'\n'+helpers+'\n'+hud+'\n'+body+'\n'+suffix)
    subprocess.run(['g++','-std=c++17','-O2','-Wall','-Wextra','-I'+str(ROOT/'include'),str(cpp),*[str(ROOT/'src'/s) for s in ('math3d.cpp','jfx_case_fix29.cpp','v14_case_mesh.cpp','case_render_fix25.cpp','full_cover_case_fix26.cpp')],'-o',str(binary)],check=True,timeout=40,capture_output=True)
    result=subprocess.run([str(binary)],capture_output=True,text=True,timeout=20)
    if name=='corrected':
        print(result.stdout,end='',flush=True)
        if result.returncode:print(result.stderr)
        assert result.returncode==0
    else:assert result.returncode!=0,name;print('PASS: rejected regression '+name,flush=True)
