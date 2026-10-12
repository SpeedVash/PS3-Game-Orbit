#!/usr/bin/env python3
"""Run the native main block and verify that preload never replaces the splash."""
from pathlib import Path
import hashlib
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parent.parent
source = (ROOT / 'src/main.cpp').read_text()
start = source.index('    if(result==0 && !user_exit){\n        // The last calibration')
begin = source.index('{', start)
end, depth = begin + 1, 1
while depth:
    depth += (source[end] == '{') - (source[end] == '}')
    end += 1
block = source[begin + 1:end - 1].replace('FileManagerFix40 file_manager;', 'FileManagerFix40 file_manager(manager_root);')
prefix = (ROOT / 'scripts/library-main-api-stubs-fix42.inc').read_text()

def replace(old, new):
    global prefix
    assert prefix.count(old) == 1, old
    prefix = prefix.replace(old, new)

replace('static bool open_frame;', '''static unsigned scenario;
static bool open_frame,in_preload,preload_complete,startup_ready;
static unsigned preload_steps,preload_polls,preload_acks,services,worker_starts,last_service_ack,worker_yields;
static void usleep(unsigned delay){assert(!open_frame && in_preload && delay==1000);++worker_yields;}''')
replace('static unsigned scenario,mount_requests,cancellations,disc_checks,preference_writes,layout_writes;',
        'static unsigned mount_requests,cancellations,disc_checks,preference_writes,layout_writes;')
replace('bool begin_frame(){assert(!open_frame);open_frame=true;return true;}',
        'bool begin_frame(){assert(!open_frame);if(scenario==26)return false;open_frame=true;return true;}')
replace('bool complete_frame(){assert(open_frame);open_frame=false;++acks;return true;}',
        '''bool complete_frame(){assert(open_frame);open_frame=false;if(scenario==27)return false;++acks;
        if(in_preload){++preload_acks;if(preload_complete)in_preload=false;}return true;}''')
replace('InputFrame input;input.connected=true;', '''InputFrame input;input.connected=true;
        if(in_preload){++preload_polls;if(scenario==22 || (scenario==31 && preload_polls==3))input.circle.pressed=true;return input;}
        if(scenario>=22){++polls;if(scenario!=32 || polls>1)input.circle.pressed=true;return input;}''')
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
        assert(!open_frame);
        const unsigned total=!current_game(state) || scenario==30?0:(scenario==29 || scenario==31?80:2);
        const unsigned done=std::min(preload_steps,total);
        if(done==total){startup_ready=true;if(total==0)in_preload=false;else preload_complete=true;}
        return {done,total};
    }
    bool streamed_cover_failed(int gi,const GameEntry&)const{return gi==2;}
    void stop_streamed_artwork(){assert(!open_frame);}''')
fn_start = prefix.index('    bool sync_visible_covers(')
fn_end = prefix.index('    bool has_cover_texture', fn_start)
prefix = prefix[:fn_start] + '''    bool sync_visible_covers(const CoverflowState&,CoverCache&,int radius){
        assert(!open_frame && radius==1);++uploads;return true;
    }
''' + prefix[fn_end:]
replace('assert(!open_frame && state);manager_active=false;assert(!lines[0].empty() && !lines[1].empty());',
        'assert(!open_frame && state && !in_preload && startup_ready);manager_active=false;if(scenario==32)return false;assert(!lines[0].empty() && !lines[1].empty());')
replace('assert(open_frame && radius==1 && scale>=0.55f && scale<=0.80f);',
        'assert(open_frame && !in_preload && startup_ready && radius==1 && scale>=0.55f && scale<=0.80f);')
replace('assert(!open_frame && dt>=0 && dt<=.1f);',
        'assert(!open_frame && !in_preload && startup_ready && dt>=0 && dt<=.1f);')
replace('if(scenario==14)return {};', 'if(scenario==14 || scenario==28)return {};')
replace('for(scenario=0;scenario<22;++scenario){', 'for(scenario=0;scenario<33;++scenario){')
replace('timed=false;open_frame=false;clock_value=last_manual_input=0;rotating=false;',
        'timed=false;open_frame=false;clock_value=last_manual_input=0;rotating=false;in_preload=preload_complete=startup_ready=false;preload_steps=preload_polls=preload_acks=services=worker_starts=worker_yields=0;last_service_ack=unsigned(-1);')
suffix = r'''
  renderer.stop_streamed_artwork();
  assert(!open_frame && worker_starts==1 && prefetch_calls==0 && decode_failures==0);
  if(scenario==23 || scenario==24 || scenario==26 || scenario==27 || scenario==32){
    assert(result==(scenario<=24?33:scenario==26?29:scenario==27?30:32));
    assert(renders==0 && hud_calls==0 && background_updates==0 && manager_hud_calls==0);
    if(scenario==32)assert(startup_ready && preload_acks==2 && polls==1);
    else assert(preload_acks==0 && polls==0);
  }else{
    assert(result==0 && !time_limit && renders+preload_acks==acks);
    assert(renders==hud_calls && background_updates==renders && renders==art_calls+manager_hud_calls);
    if(scenario==22)assert(user_exit && preload_steps==0 && preload_acks==0 && renders==0 && polls==0);
    else if(scenario==31)assert(user_exit && preload_steps==2 && preload_acks==2 && worker_yields==2 && renders==0 && polls==0);
    else if(scenario==14 || scenario==28 || scenario==30)assert(startup_ready && preload_acks==0 && worker_yields==0);
    else if(scenario==29)assert(startup_ready && preload_steps==80 && preload_acks==80 && worker_yields==79);
    else assert(startup_ready && preload_steps==2 && preload_acks==2 && worker_yields==1);
    if(scenario==18 || scenario==19 || scenario==20)assert(manager_hud_calls>0 && paused_frames>0 && restored_frames>0);
    if(scenario==20)assert(observed_copy && observed_cancel);
    if(scenario==9)assert(rescans==2);
    if(scenario==1 || scenario==4)assert(!user_exit && mount_requests==1 && disc_checks==1);
    if(scenario==21)assert(polls==181 && user_exit && services>100);
  }
 }
 puts("PASS: actual FIX46 main: no library/HUD/background presentation until initial artwork completion; upload cycles acknowledged without flips; worker receives CPU time");
 puts("PASS: 33 scenarios: slow startup, empty library, no artwork, early/mid-preload cancellation, worker/queue/FIFO failures, first-library HUD failure and previous layout/menu/manager/mount flows");
}
'''
assert source.index('renderer.stop_streamed_artwork();') < source.index('shutdown_image_decoders_fix35();')
print('MAIN_SOURCE_SHA256: ' + hashlib.sha256(source.encode()).hexdigest(), flush=True)
with tempfile.TemporaryDirectory(prefix='orbit46-main-') as directory:
    path = Path(directory)
    cpp, binary = path / 'test.cpp', path / 'test'
    cpp.write_text(prefix + block + suffix)
    sources = ('library_browser_fix28','app_controller','inspect_case_fix25','safe_boot','coverflow_state',
               'cover_cache','cover_image','v14_case_mesh','full_cover_case_fix26','render_plan','library_pair_fix28',
               'cover_orientation_fix28','orbit_flow_fix31','game_order_fix40','file_manager_fix40','file_manager_fix41',
               'file_manager_fix42','file_store_fix38','case_style_fix41')
    flags = ['-DPS3_SP_LOADER_FIX28=1','-DPS3_SP_LOADER_FIX29=1'] + ['-DPS3_GAME_ORBIT_FIX%d=1' % n for n in range(30,47)]
    subprocess.run(['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror',*flags,'-I'+str(ROOT/'include'),
                    str(cpp),*[str(ROOT/'src'/(s+'.cpp')) for s in sources],'-o',str(binary)],check=True,timeout=90)
    hdd = path / 'hdd'
    (hdd / 'dest').mkdir(parents=True)
    (hdd / 'source.bin').write_bytes(b'x' * 1000000)
    result = subprocess.run([str(binary),str(ROOT/'pkgfiles/USRDIR/FULL_COVER_FIX26_CONTINUA.png'),str(hdd)],capture_output=True,text=True,timeout=20)
    print(result.stdout,end='',flush=True)
    if result.returncode:
        print(result.stderr)
    assert result.returncode == 0
