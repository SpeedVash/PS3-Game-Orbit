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
    assert(std::string(ProjectIdentity::Version)=="1.5.2");
    DecodedImageRGBA art;art.width=1280;art.height=720;art.pitch=5120;art.rgba.resize(5120*720);
    std::ifstream input(dir+"/splash.rgba",std::ios::binary);input.read(reinterpret_cast<char*>(art.rgba.data()),art.rgba.size());assert(input.good());
    save(dir+"/TELA_INICIAL_1_5_2.ppm",OrbitUiFix30::splash(&art));
    const std::array<std::string,6> lines{{"PS3 Game Orbit","Jogo de demonstração","1 / 57","","CLASSIC",""}};
    auto h=OrbitUiFix30::hud(lines);
    GameMenuStateFix35 menu;menu.open=true;menu.homebrew=true;menu.animated_background=true;menu.remember_last_game=true;
    OrbitUiFix30::paint_hud_fix35(h,lines,{},-1,4,&menu);save(dir+"/MENU_HOME_1_5_2.ppm",h);
    h=OrbitUiFix30::hud(lines);menu.homebrew=false;menu.sort_mode=2;OrbitUiFix30::paint_hud_fix35(h,lines,{},-1,4,&menu);save(dir+"/MENU_JOGO_1_5_2.ppm",h);

    using M=FileManagerFix40;
    M::View m;m.active=true;m.panels[0].path="/dev_hdd0/GAMES";m.panels[1].path="/dev_usb000/Backup";
    m.status="O fundo animado será pausado e voltará ao sair.";
    auto shot=[&](const char* name){save(dir+"/"+name+"_1_5_2.ppm",OrbitUiFix30::file_manager_hud(m));};
    shot("GERENCIADOR_AVISO");
    m.mode=M::Mode::Browse;m.selection_panel=0;m.selection_path=m.panels[0].path;m.marked_count=2;m.selected_items=2;
    m.panels[0].entries={{"God of War Collection",true,false,0,true,false},{"Gran Turismo 6",true,false,0,false,true},{"arquivo de exemplo com um nome muito comprido para conferir a leitura.bin",false,true,123456789,true,false},{"EmptyFolder",true,false,0,false,false}};
    m.panels[0].selected=2;
    m.panels[1].entries={{"God of War Collection",true,false,0,false,false},{"Gran Turismo 6",true,false,0,false,true},{"arquivo.bin",false,true,12345678,false,false}};
    m.status="Origem fixada. Abra o destino no outro painel e use Triângulo.";shot("GERENCIADOR");
    m.active_panel=1;m.mode=M::Mode::Actions;shot("GERENCIADOR_ACOES");
    m.mode=M::Mode::Devices;m.devices={{"/dev_hdd0","HD interno",false},{"/dev_usb000","USB 000",true},{"/dev_usb001","USB 001",true}};m.device_selected=1;shot("GERENCIADOR_DISPOSITIVOS");
    m.active_panel=0;m.mode=M::Mode::Details;
    m.panels[0].path="/dev_hdd0/GAMES/uma pasta com caminho longo/para conferir o caminho inteiro/e nomes que precisam ocupar várias linhas/sem cortar a informação/mais uma subpasta com acentos e espaços";
    shot("GERENCIADOR_CAMINHO");
    m.panels[0].path="/dev_hdd0/GAMES";m.source_panel=0;m.source_directory=m.panels[0].path;m.destination_directory=m.panels[1].path;
    m.file_count=12;m.folder_count=4;m.batch_total=12345678900ULL;m.source="/dev_hdd0/GAMES/God of War Collection/USRDIR/exemplo.bin";m.destination="/dev_usb000/Backup/God of War Collection/USRDIR/exemplo.bin";
    m.mode=M::Mode::Planning;shot("GERENCIADOR_PREPARACAO");
    m.mode=M::Mode::Confirm;shot("GERENCIADOR_CONFIRMACAO");
    m.mode=M::Mode::Overwrite;m.overwrite_selected=1;shot("GERENCIADOR_CONFLITO");
    m.mode=M::Mode::Copy;m.file_index=5;m.done=25600000;m.total=100000000;m.batch_done=3300000000ULL;m.bytes_copied=3200000000ULL;m.copied_count=3;m.skipped_count=1;m.directories_done=4;shot("GERENCIADOR_COPIA");
    m.mode=M::Mode::Summary;m.copied_count=10;m.skipped_count=2;m.status="Operação concluída.";shot("GERENCIADOR_RESUMO");
    m.error_count=1;m.copied_count=3;m.status="Falha ao concluir. Arquivo anterior preservado.";shot("GERENCIADOR_ERRO");
    m.mode=M::Mode::ExitConfirm;m.exit_selected=0;shot("GERENCIADOR_SAIDA");
    h=OrbitUiFix30::hud(lines);save(dir+"/BIBLIOTECA_1_5_2.ppm",h);
}
'''
with tempfile.TemporaryDirectory(prefix='orbit-ui42-') as directory:
    d=Path(directory);(d/'ui.cpp').write_text(source)
    (d/'splash.rgba').write_bytes(Image.open(root/'pkgfiles/USRDIR/ORBIT_SPLASH.png').convert('RGBA').tobytes())
    flags=['-DPS3_GAME_ORBIT_FIX'+str(i)+'=1' for i in range(30,43)]
    subprocess.run(['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror',*flags,'-I'+str(root/'include'),'-I'+str(root/'src'),str(d/'ui.cpp'),'-o',str(d/'ui')],check=True)
    subprocess.run([str(d/'ui'),str(d)],check=True)
    out=root/'assets/previews';out.mkdir(exist_ok=True)
    for p in d.glob('*.ppm'):Image.open(p).save(out/(p.stem+'.png'))
print('PASS: actual splash, library header and TRIANGLE/START UI v1.5.2 rendered; these are computer previews, not PS3 captures')
