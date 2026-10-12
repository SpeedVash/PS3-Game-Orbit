#!/usr/bin/env python3
"""Exercise the actual native main block, including preload and error paths."""
from pathlib import Path
import hashlib
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parent.parent
source=(ROOT/'src/main.cpp').read_text()
start=source.index('    if(result==0 && !user_exit){\n        // The last calibration')
begin=source.index('{',start);end=begin+1;depth=1
while depth:
    depth+=(source[end]=='{')-(source[end]=='}');end+=1
block=source[begin+1:end-1].replace('FileManagerFix40 file_manager;', 'FileManagerFix40 file_manager(manager_root);')
prefix=(ROOT/'scripts/library-main-api-stubs-fix42.inc').read_text()
def replace(old,new):
    global prefix
    assert prefix.count(old)==1,old
    prefix=prefix.replace(old,new)

replace('static bool open_frame;', '''static bool open_frame,in_preload,preload_complete;
static unsigned preload_steps,preload_polls,preload_renders,services,worker_starts,last_service_ack;''')
replace('bool begin_frame(){assert(!open_frame);open_frame=true;return true;}',
        'bool begin_frame(){assert(!open_frame);if(scenario==26)return false;open_frame=true;return true;}')
# scenario must be declared before Stream.
replace('static bool open_frame,in_preload,preload_complete;', 'static unsigned scenario;\nstatic bool open_frame,in_preload,preload_complete;')
replace('static unsigned scenario,mount_requests,cancellations,disc_checks,preference_writes,layout_writes;',
        'static unsigned mount_requests,cancellations,disc_checks,preference_writes,layout_writes;')
replace('bool complete_frame(){assert(open_frame);open_frame=false;++acks;return true;}',
        'bool complete_frame(){assert(open_frame);open_frame=false;if(scenario==27)return false;++acks;if(in_preload && preload_complete)in_preload=false;return true;}')
replace('InputFrame input;input.connected=true;', '''InputFrame input;input.connected=true;
        if(in_preload){++preload_polls;if(scenario==22)input.circle.pressed=true;return input;}
        if(scenario>=22){++polls;input.circle.pressed=true;return input;}''')
replace('struct Renderer {', '''struct Renderer {
    bool start_streamed_artwork(){assert(!open_frame);++worker_starts;in_preload=true;return scenario!=23;}
    bool service_streamed_artwork(const CoverflowState& state){
        assert(!open_frame);++services;
        if(current_game(state))assert(last_service_ack!=acks);
        last_service_ack=acks;
        if(scenario==24)return false;
        if(in_preload)++preload_steps;
        for(const auto& pose:LibraryPairFix28::poses(state,1))
            if(pose.game_index!=2 && !state.games[std::size_t(pose.game_index)].cover_path.empty())uploaded.insert(pose.game_index);
        return true;
    }
    std::pair<unsigned,unsigned> streamed_preload_progress(const CoverflowState& state){
        assert(!open_frame);const unsigned total=current_game(state)?2:0;
        const unsigned done=std::min(preload_steps,total);
        if(done==total){if(total==0)in_preload=false;else preload_complete=true;}
        return {done,total};
    }
    bool streamed_cover_failed(int gi,const GameEntry&)const{return gi==2;}
    void stop_streamed_artwork(){assert(!open_frame);}''')
fn_start=prefix.index('    bool sync_visible_covers(')
fn_end=prefix.index('    bool has_cover_texture',fn_start)
prefix=prefix[:fn_start]+'''    bool sync_visible_covers(const CoverflowState&,CoverCache&,int radius){
        assert(!open_frame && radius==1);++uploads;return true;
    }
'''+prefix[fn_end:]
replace('assert(!open_frame && state);manager_active=false;assert(!lines[0].empty() && !lines[1].empty());',
        'assert(!open_frame && state);manager_active=false;if(scenario==25)return false;assert(!lines[0].empty() && !lines[1].empty());')
replace('if(renders==0 && scenario==15)', 'if(!in_preload && renders==preload_renders && scenario==15)')
replace('if(renders==0 && scenario==16)', 'if(!in_preload && renders==preload_renders && scenario==16)')
replace('stats.draw_calls=LibraryPairFix28::poses(state,1).size()*6;',
        'if(in_preload){assert(state.visible.empty() && state.flow.empty());++preload_renders;}stats.draw_calls=LibraryPairFix28::poses(state,1).size()*6;')
replace('if(scenario==14)return {};', 'if(scenario==14 || scenario==28)return {};')
replace('for(scenario=0;scenario<22;++scenario){', 'for(scenario=0;scenario<29;++scenario){')
replace('timed=false;open_frame=false;clock_value=last_manual_input=0;rotating=false;',
        'timed=false;open_frame=false;clock_value=last_manual_input=0;rotating=false;in_preload=preload_complete=false;preload_steps=preload_polls=preload_renders=services=worker_starts=0;last_service_ack=unsigned(-1);')
suffix=r'''
  renderer.stop_streamed_artwork();
  assert(!open_frame && worker_starts==1 && prefetch_calls==0 && decode_failures==0);
  if(scenario>=23 && scenario<=27){
    assert(result==(scenario<=24?33:scenario==25?32:scenario==26?29:30));
    assert(polls==0 && art_calls==0 && manager_hud_calls==0);
  }else{
    assert(result==0 && !time_limit && renders==acks && renders==hud_calls && background_updates==renders);
    assert(renders==preload_renders+art_calls+manager_hud_calls);
    if(scenario==22)assert(user_exit && preload_steps==0 && renders==0 && polls==0);
    else if(scenario==14 || scenario==28)assert(preload_renders==0);
    else assert(preload_steps==2 && preload_renders==2);
    if(scenario==18 || scenario==19 || scenario==20)assert(manager_hud_calls>0 && paused_frames>0 && restored_frames>0);
    if(scenario==20)assert(observed_copy && observed_cancel);
    if(scenario==9)assert(rescans==2);
    if(scenario==1 || scenario==4)assert(!user_exit && mount_requests==1 && disc_checks==1);
    if(scenario==21)assert(polls==181 && user_exit && services>100);
  }
 }
 puts("PASS: actual FIX45 main: preload before library; progress frames without cases; warmup circle cancel; worker/start/HUD/frame/ACK failures; empty view skips warmup");
 puts("PASS: all 29 native-loop scenarios use services between ACK and begin_frame; no legacy synchronous prefetch; normal navigation/menu/inspection/rescan/mount exits and file-manager restoration preserved");
}
'''
assert source.index('renderer.stop_streamed_artwork();')<source.index('shutdown_image_decoders_fix35();')
assert 'sysThreadCreate' in (ROOT/'src/artwork_loader_fix45.cpp').read_text()
print('MAIN_SOURCE_SHA256: '+hashlib.sha256(source.encode()).hexdigest(),flush=True)
with tempfile.TemporaryDirectory(prefix='orbit45-main-') as directory:
    path=Path(directory);cpp=path/'test.cpp';binary=path/'test'
    cpp.write_text(prefix+block+suffix)
    sources=('library_browser_fix28','app_controller','inspect_case_fix25','safe_boot','coverflow_state',
             'cover_cache','cover_image','v14_case_mesh','full_cover_case_fix26','render_plan','library_pair_fix28',
             'cover_orientation_fix28','orbit_flow_fix31','game_order_fix40','file_manager_fix40','file_manager_fix41',
             'file_manager_fix42','file_store_fix38','case_style_fix41')
    flags=['-DPS3_SP_LOADER_FIX28=1','-DPS3_SP_LOADER_FIX29=1']+['-DPS3_GAME_ORBIT_FIX%d=1'%n for n in range(30,46)]
    subprocess.run(['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror',*flags,'-I'+str(ROOT/'include'),
        str(cpp),*[str(ROOT/'src'/(s+'.cpp')) for s in sources],'-o',str(binary)],check=True,timeout=90)
    hdd=path/'hdd';(hdd/'dest').mkdir(parents=True);(hdd/'source.bin').write_bytes(b'x'*1000000)
    result=subprocess.run([str(binary),str(ROOT/'pkgfiles/USRDIR/FULL_COVER_FIX26_CONTINUA.png'),str(hdd)],capture_output=True,text=True,timeout=20)
    print(result.stdout,end='',flush=True)
    if result.returncode:print(result.stderr)
    assert result.returncode==0
