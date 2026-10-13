#!/usr/bin/env python3
"""Run the new file-manager tests through the actual PSL1GHT filesystem branch."""
from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parent.parent
with tempfile.TemporaryDirectory(prefix='orbit-fs42-') as directory:
    d=Path(directory);(d/'sys').mkdir();(d/'lv2').mkdir()
    for p in (d/'sys/file.h',d/'lv2/sysfs.h'):p.write_text('#include "native_fs_fix42.h"\n')
    (d/'test.cpp').write_text('#include "native_fs_fix42.h"\n'+(root/'tests/test_manager_fix42.cpp').read_text())
    subprocess.run(['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror','-D__PSL1GHT__=1','-DPS3_GAME_ORBIT_FIX42=1',
        '-I'+str(d),'-I'+str(root/'tests'),'-I'+str(root/'include'),str(d/'test.cpp'),str(root/'src/file_manager_fix42.cpp'),
        str(root/'src/runtime_diag.cpp'),'-Wl,--wrap=rename,--wrap=statvfs','-o',str(d/'test')],check=True)
    subprocess.run([str(d/'test')],check=True)
