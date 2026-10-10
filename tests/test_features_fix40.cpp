#include "game_order_fix40.h"
#include "file_manager_fix40.h"
#include "library_browser_fix28.h"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <unistd.h>
#include <cstdio>
namespace fs=std::filesystem;
static std::string failed_target;
extern "C" int __real_rename(const char*,const char*);
extern "C" int __wrap_rename(const char* a,const char* b){if(!failed_target.empty()&&b==failed_target&&std::string(a).find(".orbit15_tmp_")!=std::string::npos)return -1;return __real_rename(a,b);}
static void write(const fs::path& p,const std::string& s){std::ofstream f(p,std::ios::binary);f<<s;assert(f.good());}
static std::string read(const fs::path& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
static InputFrame press(ButtonEdge InputFrame::* key){InputFrame i;i.connected=true;(i.*key).pressed=true;return i;}
static void hit(FileManagerFix40& f,ButtonEdge InputFrame::* k){f.update(press(k),.016f);}
static void select(FileManagerFix40& f,const std::string& name){int n=-1;for(std::size_t i=0;i<f.view().entries.size();++i)if(f.view().entries[i].name==name)n=int(i);assert(n>=0);while(f.view().selected<n)hit(f,&InputFrame::down);while(f.view().selected>n)hit(f,&InputFrame::up);}
static void done(FileManagerFix40& f){int guard=100;while(f.copying()&&guard--)f.step();assert(guard>0);}
static void no_temps(const fs::path& root){for(auto& p:fs::recursive_directory_iterator(root))assert(p.path().filename().string().find(".orbit15_")==std::string::npos);}
int main(){
    std::vector<GameEntry> games(4);const char* titles[]={"Zebra","Água","beta","agua"};
    for(int i=0;i<4;++i){games[i].path="/g/"+std::to_string(i);games[i].title=titles[i];games[i].source=i==2?GameSource::USB:GameSource::HDD;}
    std::vector<int> v{0,1,2,3};GameOrderFix40::sort_visible(v,games,GameSortFix40::Alphabetical,{});assert((v==std::vector<int>{1,3,2,0}));
    GameOrderFix40::sort_visible(v,games,GameSortFix40::Reverse,{});assert((v==std::vector<int>{0,2,1,3}));
    assert(games[0].title=="Zebra");GameOrderFix40::Saved saved;saved.mode=GameSortFix40::Custom;GameOrderFix40::reconcile(saved.paths,games);
    assert((saved.paths==std::vector<std::string>{"/g/1","/g/3","/g/2","/g/0"}));
    GameOrderFix40::sort_visible(v,games,saved.mode,saved.paths);assert(GameOrderFix40::move_visible(saved.paths,games,v,3,0));
    GameOrderFix40::sort_visible(v,games,saved.mode,saved.paths);assert(v.front()==0);
    const auto hidden=std::find(saved.paths.begin(),saved.paths.end(),"/g/2")-saved.paths.begin();
    assert(GameOrderFix40::move_visible(saved.paths,games,{0,1,3},0,2));assert(saved.paths[std::size_t(hidden)]=="/g/2");
    auto present=games;present.erase(present.begin()+2);GameOrderFix40::reconcile(saved.paths,present);assert(std::find(saved.paths.begin(),saved.paths.end(),"/g/2")!=saved.paths.end());
    char temp[]="/tmp/orbit15-tests-XXXXXX";const fs::path root=mkdtemp(temp);assert(!root.empty());
    assert(saved.save((root/"order").string()));GameOrderFix40::Saved loaded;assert(loaded.load((root/"order").string())&&loaded.paths==saved.paths&&loaded.mode==saved.mode);
    auto corrupt=read(root/"order");corrupt[17]^=1;write(root/"order",corrupt);assert(!loaded.load((root/"order").string())&&loaded.paths==saved.paths);
    auto invalid=saved;invalid.paths.push_back("relative");assert(!invalid.save((root/"bad").string()));invalid=saved;invalid.paths.push_back(saved.paths.front());assert(!invalid.save((root/"bad").string()));
    LibraryBrowserFix28 b("");b.replace_catalog(games);b.update(InputFrame{},.1f);assert(current_game(b.state())->path=="/g/1");
    auto menu=[&](int index,bool home){b.update(press(home?&InputFrame::start:&InputFrame::triangle),0);for(int i=0;i<index;++i)b.update(press(&InputFrame::down),0);return b.update(press(&InputFrame::cross),0);};
    menu(4,true);assert(b.state().sort_mode==GameSortFix40::Reverse&&current_game(b.state())->path=="/g/1");b.update(press(&InputFrame::circle),0);
    menu(4,true);assert(b.state().sort_mode==GameSortFix40::Custom);b.update(press(&InputFrame::circle),0);
    menu(4,false);assert(b.state().menu.reorder);b.update(press(&InputFrame::down),0);b.update(press(&InputFrame::cross),0);assert(!b.state().menu.reorder&&b.state().selected==1&&current_game(b.state())->path=="/g/1");
    b.update(press(&InputFrame::circle),0);assert(menu(5,true).open_file_manager&&!b.state().menu.open);
    b.restore_selection(FilterMode::All,"");assert(b.state().selected==0);
    puts("PASS: A-Z/Z-A accent folding, stable catalog indices, filtered Custom move, missing USB, checksummed persistence and actual START/TRIANGLE commands");
    fs::create_directories(root/"dest");std::string data(2*1024*1024+173,'x');for(std::size_t i=0;i<data.size();++i)data[i]=char(i*37);write(root/"source.bin",data);
    FileManagerFix40 f(root.string());f.open();assert(f.active()&&f.view().mode==FileManagerFix40::Mode::Notice);hit(f,&InputFrame::cross);select(f,"source.bin");hit(f,&InputFrame::square);hit(f,&InputFrame::triangle);assert(!f.copying());
    select(f,"dest");hit(f,&InputFrame::cross);hit(f,&InputFrame::triangle);assert(f.copying()&&f.view().total==data.size());f.step();assert(f.view().done==256*1024&&!fs::exists(root/"dest/source.bin"));done(f);assert(read(root/"dest/source.bin")==data&&read(root/"source.bin")==data);no_temps(root);
    write(root/"dest/source.bin","old");hit(f,&InputFrame::triangle);assert(f.view().mode==FileManagerFix40::Mode::Overwrite);hit(f,&InputFrame::circle);assert(read(root/"dest/source.bin")=="old");
    hit(f,&InputFrame::triangle);hit(f,&InputFrame::cross);f.step();hit(f,&InputFrame::circle);assert(read(root/"dest/source.bin")=="old");no_temps(root);
    hit(f,&InputFrame::triangle);hit(f,&InputFrame::cross);failed_target=(root/"dest/source.bin").string();done(f);failed_target.clear();assert(read(root/"dest/source.bin")=="old");no_temps(root);
    hit(f,&InputFrame::triangle);hit(f,&InputFrame::cross);done(f);assert(read(root/"dest/source.bin")==data);no_temps(root);
    hit(f,&InputFrame::triangle);hit(f,&InputFrame::cross);f.step();write(root/"source.bin",data+"changed");done(f);assert(read(root/"dest/source.bin")==data);no_temps(root);
    hit(f,&InputFrame::triangle);write(root/"dest/source.bin","changed dest");hit(f,&InputFrame::cross);assert(!f.copying()&&read(root/"dest/source.bin")=="changed dest");no_temps(root);
    hit(f,&InputFrame::circle);write(root/"empty.bin","");f.close();f.open();hit(f,&InputFrame::cross);select(f,"empty.bin");hit(f,&InputFrame::square);select(f,"dest");hit(f,&InputFrame::cross);hit(f,&InputFrame::triangle);done(f);assert(fs::exists(root/"dest/empty.bin")&&fs::file_size(root/"dest/empty.bin")==0);
    f.close();fs::create_symlink(root/"source.bin",root/"link");f.open();hit(f,&InputFrame::cross);select(f,"link");const auto clipboard=f.view().clipboard;hit(f,&InputFrame::square);assert(f.view().clipboard==clipboard);
    write(root/"large.bin","");fs::resize_file(root/"large.bin",(5ull<<30)+123);f.close();f.open();hit(f,&InputFrame::cross);select(f,"large.bin");hit(f,&InputFrame::square);select(f,"dest");hit(f,&InputFrame::cross);hit(f,&InputFrame::triangle);assert(f.copying()&&f.view().total==(5ull<<30)+123);f.step();hit(f,&InputFrame::start);assert(!f.active()&&!fs::exists(root/"dest/large.bin"));no_temps(root);
    fs::remove_all(root);puts("PASS: chunked real file copy, byte identity, empty and >4GB files, cancellation, overwrite confirmation, rollback, source/destination changes and symlink handling");
}
