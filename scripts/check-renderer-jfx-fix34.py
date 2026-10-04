#!/usr/bin/env python3
"""Exercise the actual native draw body using JFX geometry and API stubs."""
from pathlib import Path
import re,subprocess,tempfile,hashlib
ROOT=Path(__file__).resolve().parent.parent
source=(ROOT/'src/rsx_renderer_v10.cpp').read_text()
def function(signature,source=source):
    start=source.index(signature);begin=source.index('{',start);end=begin+1;depth=1
    while depth:depth+=(source[end]=='{')-(source[end]=='}');end+=1
    return source[start:end]
draw=function('bool RsxRendererV10::draw_frame_ps3(')
hud=function('bool RsxRendererV10::draw_orbit_background_ps3(')+'\n'+function('bool RsxRendererV10::draw_library_hud_ps3(')+'\n'+function('unsigned RsxRendererV10::front_green_samples(')
helpers='\n'.join(function(s) for s in ('V10FramePlan build_v10_frame_plan(','V10UvTransform compute_v10_uv_transform(','V10UvTransform compute_native_uv_transform(',
 'std::size_t native_packet_part(','std::vector<std::size_t> native_submission_order(',
 'const GpuTextureStage1* RsxRendererV10::texture_for_game(','static bool surface_uses_texture('))
art_source=(ROOT/'src/inspection_art_fix33.cpp').read_text()
cache_source=(ROOT/'src/inspection_art_cache_fix34.cpp').read_text()
helpers+='\n'+'\n'.join(function(s,cache_source) for s in ('bool RsxRendererV10::has_inside_texture(', 'bool RsxRendererV10::has_disc_art_texture('))+'\n'+function('const GpuTextureStage1* RsxRendererV10::texture_for_art_role(',art_source)
prefix=(ROOT/'scripts/jfx-native-api-stubs-fix34.inc').read_text()
constants=sorted(set(re.findall(r'\bGCM_[A-Z0-9_]+\b',draw+hud)))
defines=''.join(f'constexpr u32 {n}={0 if n=="GCM_FALSE" else 1 if n=="GCM_TRUE" else i+2};\n' for i,n in enumerate(constants))
prefix=prefix.replace('struct gcmContextData {',defines+'struct gcmContextData {')
suffix=r'''
static void reset(){
 ctx={commands,commands,commands+16383};draws=loads=version=loaded_version=uploads=0;
 flip_calls=padding_on_clear=plastic_draws=paper_draws=logo_draws=0;
 texture_bound=false;wait_ok=true;flip_rc=0;cull=false;far_seen=near_seen=false;
 transparent_started=false;previous_depth=100;far_faces.clear();
}
static V10FramePlan plan_for(RsxRendererV10& r,const V14CaseMesh& mesh,const std::vector<CasePose>& poses){
 r.covers_.clear();for(const auto& p:poses){RsxRendererV10::GpuCoverRecord record;record.texture.uploaded=record.texture.full_cover=true;record.texture.gpu_offset=10+p.game_index;r.covers_[p.game_index]=record;}
 const auto plan=build_v10_frame_plan(poses,mesh);expected_draws.clear();
 for(auto i:native_submission_order(plan,mesh)){const auto& p=plan.packets[i];expected_draws.push_back({p,mesh.parts[p.mesh_part]});}
 case_draws=plan.packets.size();return plan;
}
int main(){
 RsxStage1 stage;stage.context_=&ctx;RsxRendererV10 r;r.stage1_=&stage;
 r.color_buffer_[0]=r.color_buffer_[1]=pixels.data();r.depth_buffer_=pixels.data();r.color_pitch_=5120;
 r.vertex_program_=&vp;r.fragment_program_=&fp;r.vp_mvp_=&mvp_param;r.vp_model_=&model_param;r.vp_uv_transform_=&uv_param;
 r.fp_base_color_=&color_param;r.fp_use_texture_=&texture_param;r.fp_alpha_=&alpha_param;
 const auto mesh=CaseAnimationFix32::build(JfxCaseFix29::build());
 for(const auto& part:mesh.parts){RsxRendererV10::GpuMeshPart gp;gp.surface=part.surface;gp.vertices=gp.indices=pixels.data();gp.vertex_count=part.vertices.size();gp.index_count=part.indices.size();r.gpu_mesh_.push_back(gp);}
 r.hud_texture_.uploaded=true;r.hud_texture_.width=1280;r.hud_texture_.height=512;
 r.disc_label_texture_.uploaded=r.disc_back_texture_.uploaded=true;r.disc_label_texture_.gpu_offset=80;r.disc_back_texture_.gpu_offset=81;
 r.hud_mesh_.vertices=r.hud_mesh_.indices=pixels.data();r.hud_mesh_.index_count=18;
 r.background_texture_.uploaded=true;r.background_texture_.width=1280;r.background_texture_.height=360;r.background_texture_.gpu_offset=30;
 r.background_mesh_.vertices=r.background_mesh_.indices=pixels.data();r.background_mesh_.index_count=6;
 V10FramePlan last;
 for(int cases:{1,2,3,11,14}) for(int yaw:{0,28,90,152,180,270,330}){
  std::vector<CasePose> poses;
  for(int i=0;i<cases;++i){CasePose p;p.game_index=i;p.selected=i==0;p.x=(i-3)*20;p.z=55-i*25;p.yaw_deg=i ? 90 : yaw;p.pitch_deg=i ? 0 : -5;p.scale=.55f;p.alpha=i ? .76f : 1;p.visibility=cases==3 ? (i==0 ? .35f : i==1 ? .5f : 1.0f) : 1.0f;poses.push_back(p);}
  last=plan_for(r,mesh,poses);
  for(int mode:{1,2,0}){
   texture_mode=mode;for(auto& kv:r.covers_) {kv.second.texture.full_cover=mode!=2;kv.second.texture.uploaded=mode!=0;}
   reset();if(!r.draw_frame_ps3(last,mesh)){fprintf(stderr,"closed cases=%d yaw=%d mode=%d: %s\n",cases,yaw,mode,r.last_error().c_str());return 3;}
   assert(draws==unsigned(cases*6+2) && loads==draws && flip_calls==1 && paper_draws==unsigned(cases*3) && logo_draws==unsigned(cases) && plastic_draws==unsigned(cases*2) && far_seen && near_seen);
   assert(r.last_stats_.textured_draw_calls==unsigned(cases*(mode==1 ? 3 : mode==2 ? 1 : 0)));
  }
 }

 for(float phase:{.25f,.5f,.65f,.76f,1.f}) for(int yaw:{0,28,90,152,180,270,342}) {
  CasePose pose;pose.game_index=0;pose.selected=true;pose.x=35;pose.z=72;pose.scale=.55f;pose.yaw_deg=yaw;pose.pitch_deg=-5;pose.inspection_phase=phase;
  last=plan_for(r,mesh,{pose});
  for(int mode:{1,2,0}) for(art_mode=0;art_mode<5;++art_mode) {
   r.inside_cache_.entries.clear();r.disc_cache_.entries.clear();const int art_index=art_mode==2 ? 99 : 0;
   RsxRendererV10::InspectionArt inside,disc;inside.game_index=disc.game_index=art_index;
   inside.texture.uploaded=art_mode!=0 && art_mode!=4;disc.texture.uploaded=art_mode!=0 && art_mode!=3;
   inside.texture.gpu_offset=82;disc.texture.gpu_offset=83;
   r.inside_cache_.entries[art_index]=inside;r.disc_cache_.entries[art_index]=disc;
   texture_mode=mode;for(auto& kv:r.covers_){kv.second.texture.full_cover=mode!=2;kv.second.texture.uploaded=mode!=0;}
   reset();if(!r.draw_frame_ps3(last,mesh)){fprintf(stderr,"open phase=%g yaw=%d mode=%d: %s\n",phase,yaw,mode,r.last_error().c_str());return 3;}assert(draws==last.packets.size()+2 && flip_calls==1 && plastic_draws==4);
   unsigned expected=0;for(const auto& packet:last.packets){const auto& part=mesh.parts[packet.mesh_part];if(part.textured && (part.texture_role==V14MeshPart::TextureRole::DiscLabel || part.texture_role==V14MeshPart::TextureRole::DiscBack || (part.texture_role==V14MeshPart::TextureRole::Inside && (art_mode==1 || art_mode==3)) || (part.texture_role==V14MeshPart::TextureRole::Cover && mode && (mode==1 || part.surface==V14Surface::CoverFront))))++expected;}
   assert(r.last_stats_.textured_draw_calls==expected);
  }
 }
 puts("PASS: native articulated draw at five phases and seven angles, custom/missing/mismatched interior and disc labels, full-spread identity UVs and per-game bindings, opaque disc center, complete case-plastic passes and actual joint matrices");
 // Restore the maximum closed transition pool for FIFO and malformed-range probes.
 std::vector<CasePose> maxposes;for(int i=0;i<14;++i){CasePose pose;pose.game_index=i;pose.scale=.55f;pose.z=55;maxposes.push_back(pose);}last=plan_for(r,mesh,maxposes);
 puts("PASS: actual native body at seven angles and 1/2/3/11/14 cases: per-game textures/matrices, unreflected original back UVs, front-only/missing artwork, fading paper/logo, both neutral plastic passes, opaque-first/depth order, background and compact HUD");
 texture_mode=1;for(auto& kv:r.covers_){kv.second.texture.uploaded=true;kv.second.texture.full_cover=true;}
 reset();ctx.end=commands+100;assert(!r.draw_frame_ps3(last,mesh) && draws==0);
 reset();auto malformed=last;malformed.packets[0].mesh_part=99;assert(!r.draw_frame_ps3(malformed,mesh) && draws==0);
 reset();malformed=last;malformed.packets[1].plastic_pass=0;assert(!r.draw_frame_ps3(malformed,mesh) && draws==0);
 reset();malformed.packets=last.packets;malformed.packets.insert(malformed.packets.end(),last.packets.begin(),last.packets.begin()+6);assert(!r.draw_frame_ps3(malformed,mesh) && draws==0);
 reset();padding_on_clear=11000;assert(!r.draw_frame_ps3(last,mesh) && flip_calls==0);
 puts("PASS: insufficient FIFO space, invalid range, missing plastic pass, excessive case count and command budget prevent flip");
 CasePose only;only.game_index=0;only.scale=.55f;only.z=55;only.yaw_deg=28;only.pitch_deg=-5;
 last=plan_for(r,mesh,{only});reset();r.current_buffer_=0;r.diagnostic_view_label_="CULL_FRONT";assert(r.draw_frame_ps3(last,mesh) && flip_calls==0 && r.current_buffer_==0);
 static std::vector<u32> shown_pixels(1280*720,0xffff00ffu);
 std::fill(pixels.begin(),pixels.end(),0xff00c030u);r.color_buffer_[1]=shown_pixels.data();r.last_plan_=last;assert(r.front_green_samples(mesh)==6);
 r.color_buffer_[1]=pixels.data();r.diagnostic_view_label_="";
 puts("PASS: startup calibration stays offscreen without flipping or advancing the displayed framebuffer");
 puts("LIMIT: API simulation checks states/order/guards, not real RSX sampling, performance, native command word counts or television output");
}
'''

variants={'corrected':draw,'blue-opaque-plastic':draw.replace('mesh.jfx ? part.color.data() : shell_color','shell_color'),
 'missing-far-face-pass':draw.replace('packet.plastic_pass==1 ? GCM_CULL_FRONT : GCM_CULL_BACK','GCM_CULL_BACK'),
 'wrong-neighbor-texture':draw.replace('texture_for_art_role(part.texture_role,packet.game_index)','texture_for_art_role(part.texture_role,0)'),
 'opaque-fading-paper':draw.replace('shell || packet.visibility<.99999f','shell'),
 'missing-visibility':draw.replace('(shell ? packet.alpha : 1.0f)*packet.visibility','(shell ? packet.alpha : 1.0f)'),
 'missing-background-depth-restore':draw.replace('rsxSetDepthTestEnable(ctx,GCM_TRUE);\n        rsxSetDepthWriteEnable','rsxSetDepthTestEnable(ctx,GCM_FALSE);\n        rsxSetDepthWriteEnable'),
 'inside-bound-to-exterior':draw.replace('texture_for_art_role(part.texture_role,packet.game_index)','texture_for_art_role(part.texture_role==V14MeshPart::TextureRole::Inside ? V14MeshPart::TextureRole::Cover : part.texture_role,packet.game_index)'),
 'missing-fp-reload':draw.replace('rsxLoadFragmentProgramLocation(ctx,fp,fragment_offset_,GCM_LOCATION_RSX);','')}
print('NATIVE_DRAW_SOURCE_SHA256: '+hashlib.sha256(source.encode()).hexdigest(),flush=True)
with tempfile.TemporaryDirectory(prefix='fix29-jfx-draw-') as directory:
 for name,body in variants.items():
    path=Path(directory);cpp=path/(name+'.cpp');binary=path/name;cpp.write_text(prefix+'\n'+helpers+'\n'+hud+'\n'+body+'\n'+suffix)
    subprocess.run(['g++','-std=c++17','-O2','-Wall','-Wextra','-DPS3_GAME_ORBIT_FIX31=1','-DPS3_GAME_ORBIT_FIX32=1','-DPS3_GAME_ORBIT_FIX33=1','-DPS3_GAME_ORBIT_FIX34=1','-I'+str(ROOT/'include'),str(cpp),*[str(ROOT/'src'/s) for s in ('math3d.cpp','jfx_case_fix29.cpp','v14_case_mesh.cpp','case_render_fix25.cpp','full_cover_case_fix26.cpp','case_animation_fix32.cpp','frame_validation_fix32.cpp')],'-o',str(binary)],check=True,timeout=40)
    result=subprocess.run([str(binary)],capture_output=True,text=True,timeout=20)
    if name=='corrected':
        print(result.stdout,end='',flush=True)
        if result.returncode:print(result.stderr)
        assert result.returncode==0
    else:assert result.returncode!=0,name;print('PASS: rejected regression '+name,flush=True)
