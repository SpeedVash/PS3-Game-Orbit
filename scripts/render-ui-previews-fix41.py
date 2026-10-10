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
namespace CaseStyleFix41 {const char* name(GameCaseStyleFix41){return "Greatest Hits (vermelha)";}}
namespace GameOrderFix40 {const char* name(GameSortFix40 m){return m==GameSortFix40::Custom?"Custom":m==GameSortFix40::Reverse?"Z até A":"A até Z";}}
int main(int argc,char** argv){assert(argc==2);const std::string dir=argv[1];
    assert(std::string(ProjectIdentity::Version)=="1.5.1");
    DecodedImageRGBA art;art.width=1280;art.height=720;art.pitch=5120;art.rgba.resize(5120*720);
    std::ifstream input(dir+"/splash.rgba",std::ios::binary);input.read(reinterpret_cast<char*>(art.rgba.data()),art.rgba.size());assert(input.good());
    save(dir+"/TELA_INICIAL_1_5_1.ppm",OrbitUiFix30::splash(&art));
    const std::array<std::string,6> lines{{"PS3 Game Orbit","Jogo de demonstração","1 / 57","","CLASSIC",""}};
    auto h=OrbitUiFix30::hud(lines);
    GameMenuStateFix35 menu;menu.open=true;menu.homebrew=true;menu.animated_background=true;menu.remember_last_game=true;
    OrbitUiFix30::paint_hud_fix35(h,lines,{},-1,4,&menu);save(dir+"/MENU_HOME_1_5_1.ppm",h);
    h=OrbitUiFix30::hud(lines);menu.homebrew=false;menu.sort_mode=2;OrbitUiFix30::paint_hud_fix35(h,lines,{},-1,4,&menu);save(dir+"/MENU_JOGO_1_5_1.ppm",h);
    FileManagerFix40::View manager;manager.active=true;manager.panels[0].path="/dev_hdd0";manager.panels[1].path="/dev_usb000/PS3COVERS";manager.status="O fundo animado será pausado neste modo.";
    save(dir+"/GERENCIADOR_AVISO_1_5_1.ppm",OrbitUiFix30::file_manager_hud(manager));
    manager.mode=FileManagerFix40::Mode::Browse;manager.panels[0].entries={{"GAMES",true,false,0},{"PS3COVERS",true,false,0},{"arquivo.bin",false,true,12345678}};
    manager.panels[0].entries.back().marked=true;manager.marked_count=1;manager.panels[1].entries={{"BLES00001.jpg",false,true,123000,false,true}};manager.status="Quadrado marca; Triângulo copia para o outro painel.";
    save(dir+"/GERENCIADOR_1_5_1.ppm",OrbitUiFix30::file_manager_hud(manager));
    manager.mode=FileManagerFix40::Mode::Overwrite;manager.destination="/dev_usb000/PS3COVERS/BLES00001.jpg";manager.overwrite_selected=2;manager.status="Arquivo existente: BLES00001.jpg";save(dir+"/GERENCIADOR_CONFLITO_1_5_1.ppm",OrbitUiFix30::file_manager_hud(manager));
    h=OrbitUiFix30::hud(lines);save(dir+"/BIBLIOTECA_1_5_1.ppm",h);
}
'''
with tempfile.TemporaryDirectory(prefix='orbit-ui41-') as directory:
    d=Path(directory);(d/'ui.cpp').write_text(source)
    (d/'splash.rgba').write_bytes(Image.open(root/'pkgfiles/USRDIR/ORBIT_SPLASH.png').convert('RGBA').tobytes())
    flags=['-DPS3_GAME_ORBIT_FIX'+str(i)+'=1' for i in range(30,42)]
    subprocess.run(['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror',*flags,'-I'+str(root/'include'),'-I'+str(root/'src'),str(d/'ui.cpp'),'-o',str(d/'ui')],check=True)
    subprocess.run([str(d/'ui'),str(d)],check=True)
    out=root/'assets/previews';out.mkdir(exist_ok=True)
    for p in d.glob('*.ppm'):Image.open(p).save(out/(p.stem+'.png'))
print('PASS: actual splash, library header and TRIANGLE/START UI v1.5.1 rendered; these are computer previews, not PS3 captures')
