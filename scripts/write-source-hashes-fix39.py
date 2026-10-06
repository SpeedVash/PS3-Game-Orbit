#!/usr/bin/env python3
"""Record the current build inputs; private JFX data is represented by hashes only."""
import hashlib
from pathlib import Path
root=Path(__file__).resolve().parent.parent
files=[root/name for name in ('Makefile','README.md','VERSION','CHANGELOG.md','CREDITS.md')]
for folder in ('.github','include','src','shaders','scripts','tests','pkgfiles','assets','docs'):
 files += [p for p in (root/folder).rglob('*') if p.is_file() and p.suffix not in ('.o','.d','.pyc','.log') and '__pycache__' not in p.parts]
out=root/'dist/B/SOURCE_SHA256.txt';out.parent.mkdir(parents=True,exist_ok=True)
out.write_text(''.join(hashlib.sha256(p.read_bytes()).hexdigest()+'  '+p.relative_to(root).as_posix()+'\n' for p in sorted(set(files))))
