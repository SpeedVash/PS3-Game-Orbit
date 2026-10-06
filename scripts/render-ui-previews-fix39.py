#!/usr/bin/env python3
"""Render the actual CPU splash/HUD/font code, without a PS3 or image decoder."""
from pathlib import Path
import subprocess,tempfile
from PIL import Image
root=Path(__file__).resolve().parent.parent
source=r'''
#include <fstream>
#include <cassert>
#include "orbit_ui_fix30.cpp"
static void save(const std::string& name,const DecodedImageRGBA& p){
    std::ofstream f(name,std::ios::binary);f<<"P6\n"<<p.width<<" "<<p.height<<"\n255\n";
    for(int y=0;y<p.height;++y)for(int x=0;x<p.width;++x){const auto i=std::size_t(y)*p.pitch+x*4;
        unsigned char c[3];for(unsigned k=0;k<3;++k)c[k]=(unsigned(p.rgba[i+k])*p.rgba[i+3]+unsigned(k==0?16:k==1?19:23)*(255-p.rgba[i+3])+127)/255;
        f.write(reinterpret_cast<char*>(c),3);}
}
int main(int argc,char** argv){assert(argc==2);const std::string dir=argv[1];
    assert(std::string(ProjectIdentity::Version)=="1.4.2");
    DecodedImageRGBA art;art.width=1280;art.height=720;art.pitch=5120;art.rgba.resize(5120*720);
    std::ifstream input(dir+"/splash.rgba",std::ios::binary);input.read(reinterpret_cast<char*>(art.rgba.data()),art.rgba.size());assert(input.good());
    save(dir+"/TELA_INICIAL_1_4_2.ppm",OrbitUiFix30::splash(&art));
    const std::array<std::string,6> lines{{"PS3 Game Orbit","Jogo de demonstração","1 / 57","","CLASSIC",""}};
    auto h=OrbitUiFix30::hud(lines);
    GameMenuStateFix35 menu;menu.open=true;menu.homebrew=true;menu.animated_background=true;menu.remember_last_game=true;
    OrbitUiFix30::paint_hud_fix35(h,lines,{},-1,4,&menu);save(dir+"/MENU_HOME_1_4_2.ppm",h);
    h=OrbitUiFix30::hud(lines);menu.homebrew=false;OrbitUiFix30::paint_hud_fix35(h,lines,{},-1,4,&menu);save(dir+"/MENU_JOGO_1_4_2.ppm",h);
    h=OrbitUiFix30::hud(lines);save(dir+"/BIBLIOTECA_1_4_2.ppm",h);
}
'''
with tempfile.TemporaryDirectory(prefix='orbit-ui39-') as directory:
    d=Path(directory);(d/'ui.cpp').write_text(source)
    (d/'splash.rgba').write_bytes(Image.open(root/'pkgfiles/USRDIR/ORBIT_SPLASH.png').convert('RGBA').tobytes())
    flags=['-DPS3_GAME_ORBIT_FIX'+str(i)+'=1' for i in range(30,40)]
    subprocess.run(['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror',*flags,'-I'+str(root/'include'),'-I'+str(root/'src'),str(d/'ui.cpp'),'-o',str(d/'ui')],check=True)
    subprocess.run([str(d/'ui'),str(d)],check=True)
    out=root/'assets/previews';out.mkdir(exist_ok=True)
    for p in d.glob('*.ppm'):Image.open(p).save(out/(p.stem+'.png'))
print('PASS: actual splash, library header and TRIANGLE/START UI v1.4.2 rendered; these are computer previews, not PS3 captures')
