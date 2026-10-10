#!/usr/bin/env python3
from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parent.parent
with tempfile.TemporaryDirectory(prefix='orbit-fs41-') as directory:
    d=Path(directory);(d/'sys').mkdir();(d/'lv2').mkdir()
    for p in (d/'sys/file.h',d/'lv2/sysfs.h'):p.write_text('#include "native_fs_fix41.h"\n')
    cpp=(root/'tests/test_features_fix41.cpp').read_text()
    cpp=cpp.replace('    f.close();fs::remove_all(root);','''
    write(hd/"blockopen.bin","blocked");hit(f,&InputFrame::r3);select(f,"blockopen.bin");hit(f,&InputFrame::triangle);
    assert(!f.copying() && f.view().status.find("80010009")!=std::string::npos);
    assert(native_opens>0 && native_reads>0 && native_fstats>0 && !native_free_device.empty());
    puts("PASS: actual native sysFs branch uses matching descriptors, independent path/fd timestamp baselines, destination-volume free space and exact open error codes");
    f.close();fs::remove_all(root);''')
    (d/'test.cpp').write_text('#include "native_fs_fix41.h"\n'+cpp)
    flags=['-std=c++17','-O2','-Wall','-Wextra','-Werror','-D__PSL1GHT__=1','-DPS3_SP_LOADER_FIX28=1','-DPS3_SP_LOADER_FIX29=1']+['-DPS3_GAME_ORBIT_FIX'+str(i)+'=1' for i in range(30,42)]
    files=('file_manager_fix41','runtime_diag','preferences_fix29','file_store_fix38','library_browser_fix28','game_order_fix40','app_controller','inspect_case_fix25','safe_boot','coverflow_state','orbit_flow_fix31','render_plan','library_pair_fix28','cover_orientation_fix28','case_style_fix41')
    subprocess.run(['g++',*flags,'-I'+str(d),'-I'+str(root/'tests'),'-I'+str(root/'include'),str(d/'test.cpp'),*[str(root/'src'/f'{s}.cpp') for s in files],'-Wl,--wrap=rename','-o',str(d/'test')],check=True)
    subprocess.run([str(d/'test')],check=True)
