#!/usr/bin/env python3
"""Actual FIX32 main block: GPU/I/O ordering, error, cancel and mount exit paths."""
from pathlib import Path
import subprocess,tempfile,hashlib
ROOT=Path(__file__).resolve().parent.parent
source=(ROOT/'src/main.cpp').read_text()
start=source.index('    if(result==0 && !user_exit){\n        // The last calibration')
begin=source.index('{',start);end=begin+1;depth=1
while depth:depth+=(source[end]=='{')-(source[end]=='}');end+=1
block=source[begin+1:end-1]
prefix=(ROOT/'scripts/library-main-api-stubs-fix32.inc').read_text()
suffix=r'''
  assert(result==0 && !time_limit && !open_frame && renders==acks && renders==hud_calls);
  if(scenario==3) {
   assert(mount_requests==0 && user_exit && renders==140 && prefetch_calls>0 && preference_writes>0 && layout_writes>0);
  } else {
  assert(mount_requests==1 && cancellations==1 && preference_writes>=2 && layout_writes>=2);
  if(scenario==1) assert(!user_exit && renders==10 && disc_checks==1);
  else assert(user_exit && renders==9);
  }
 }
 puts("PASS: actual FIX32 main with network failure, confirmed mounted-disc exit and circle cancellation; rescan/decode/HUD/mount/preference/disc checks occur only between acknowledged frames");
 puts("PASS: actual idle prefetch runs between acknowledged frames; inspection X does not mount and closed Circle exits after return");
 puts("PASS: unavailable network keeps the library running; XMB exit follows disc confirmation, and no next-frame upload/draw occurs after exit");
}
'''
print('MAIN_SOURCE_SHA256: '+hashlib.sha256(source.encode()).hexdigest(),flush=True)
with tempfile.TemporaryDirectory(prefix='fix29-main-loop-') as directory:
    path=Path(directory);cpp=path/'test.cpp';binary=path/'test';cpp.write_text(prefix+block+suffix)
    sources=('library_browser_fix28.cpp','app_controller.cpp','inspect_case_fix25.cpp','safe_boot.cpp','coverflow_state.cpp',
             'cover_cache.cpp','cover_image.cpp','v14_case_mesh.cpp','full_cover_case_fix26.cpp','render_plan.cpp','library_pair_fix28.cpp','cover_orientation_fix28.cpp','orbit_flow_fix31.cpp')
    subprocess.run(['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror','-DPS3_SP_LOADER_FIX28=1','-DPS3_SP_LOADER_FIX29=1','-DPS3_GAME_ORBIT_FIX30=1','-DPS3_GAME_ORBIT_FIX31=1','-DPS3_GAME_ORBIT_FIX32=1',
                    '-I'+str(ROOT/'include'),str(cpp),*[str(ROOT/'src'/s) for s in sources],'-o',str(binary)],check=True,timeout=40)
    result=subprocess.run([str(binary),str(ROOT/'pkgfiles/USRDIR/FULL_COVER_FIX26_CONTINUA.png')],capture_output=True,text=True,timeout=15)
    print(result.stdout,end='',flush=True)
    if result.returncode:print(result.stderr)
    assert result.returncode==0
