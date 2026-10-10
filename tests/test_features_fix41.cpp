#include <cassert>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include "file_manager_fix41.h"
#include "interaction_policy_fix41.h"
#include "preferences_fix29.h"
#include "library_browser_fix28.h"
#include "case_style_fix41.h"
namespace fs=std::filesystem;
static bool fail_commit=false;
extern "C" int __real_rename(const char*,const char*);
extern "C" int __wrap_rename(const char* from,const char* to){
    if(fail_commit&&std::string(from).find(".orbit151_tmp_")!=std::string::npos){fail_commit=false;errno=EIO;return -1;}
    if(fs::exists(to)){errno=EEXIST;return -1;}
    return __real_rename(from,to);
}
static void write(const fs::path& p,const std::string& data){std::ofstream f(p,std::ios::binary);f.write(data.data(),data.size());assert(f.good());}
static std::string read(const fs::path& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
static InputFrame press(ButtonEdge InputFrame::* key){InputFrame in;in.connected=true;(in.*key).pressed=true;return in;}
static void hit(FileManagerFix41& f,ButtonEdge InputFrame::* key){f.update(press(key),.016f);}
static void select(FileManagerFix41& f,const std::string& name){
    const auto& p=f.view().panels[f.view().active_panel];int target=-1;
    for(std::size_t i=0;i<p.entries.size();++i)if(p.entries[i].name==name)target=int(i);
    assert(target>=0);while(f.view().panels[f.view().active_panel].selected<target)hit(f,&InputFrame::down);
    while(f.view().panels[f.view().active_panel].selected>target)hit(f,&InputFrame::up);
}
static void run(FileManagerFix41& f){unsigned guard=500;while(f.copying()&&guard--){assert(f.view().mode!=FileManagerFix41::Mode::Overwrite);f.step();}assert(guard>0);}
static void conflict(FileManagerFix41& f,int choice){assert(f.view().mode==FileManagerFix41::Mode::Overwrite);for(int i=0;i<choice;++i)hit(f,&InputFrame::down);hit(f,&InputFrame::cross);}
static bool marked(FileManagerFix41& f,const std::string& name,bool copied){for(const auto& e:f.view().panels[f.view().active_panel].entries)if(e.name==name)return copied?e.copied:e.marked;return false;}
static void clean(const fs::path& p){for(const auto& e:fs::recursive_directory_iterator(p))assert(e.path().filename().string().rfind(".orbit151_",0)!=0);}
static std::uint32_t digest(const std::string& s){std::uint32_t h=2166136261u;for(unsigned char c:s)h=(h^c)*16777619u;return h;}
static std::string hex(const std::string& s){std::string out;const char* d="0123456789abcdef";for(unsigned char c:s){out+=d[c>>4];out+=d[c&15];}return out;}
int main(){
    char tmp[]="/tmp/orbit151-tests-XXXXXX";const fs::path root=mkdtemp(tmp),hd=root/"dev_hdd0",usb=root/"dev_usb000";
    fs::create_directories(hd/"dest");fs::create_directories(usb);
    const std::string data(2*1024*1024+173,'q');write(hd/"a.bin",data);write(hd/"b.bin","second");write(hd/"empty.bin","");
    FileManagerFix41 f(std::vector<std::string>{hd.string(),usb.string()});f.open();assert(f.active()&&f.devices().size()==2);
    hit(f,&InputFrame::cross);assert(f.view().panels[0].path==hd&&f.view().panels[1].path==usb);
    select(f,"a.bin");hit(f,&InputFrame::square);assert(marked(f,"a.bin",false));select(f,"b.bin");hit(f,&InputFrame::square);
    select(f,"empty.bin");hit(f,&InputFrame::square);assert(f.view().marked_count==3);hit(f,&InputFrame::triangle);run(f);
    assert(f.view().copied_count==3&&f.view().marked_count==0&&read(usb/"a.bin")==data&&read(usb/"b.bin")=="second"&&fs::file_size(usb/"empty.bin")==0);
    assert(marked(f,"a.bin",true));hit(f,&InputFrame::right);assert(marked(f,"a.bin",true));clean(root);
    puts("PASS: two independent HD/USB panels, multi-file copy with exact byte identity, zero-byte files and source/destination highlights");
    // Reverse direction; other panel is an independently navigated directory.
    hit(f,&InputFrame::left);select(f,"dest");hit(f,&InputFrame::cross);hit(f,&InputFrame::right);select(f,"b.bin");hit(f,&InputFrame::triangle);run(f);
    assert(read(hd/"dest/b.bin")=="second");hit(f,&InputFrame::left);assert(f.view().panels[0].path==hd/"dest");hit(f,&InputFrame::circle);
    hit(f,&InputFrame::circle);assert(f.view().panels[0].path.empty());select(f,"dev_hdd0");hit(f,&InputFrame::cross);assert(f.view().panels[0].path==hd);
    select(f,"a.bin");hit(f,&InputFrame::square);select(f,"b.bin");hit(f,&InputFrame::square);hit(f,&InputFrame::triangle);
    assert(read(usb/"a.bin")==data);conflict(f,3);run(f);assert(f.view().skipped_count==2&&f.view().copied_count==0);clean(root);
    // Reset marked set through SELECT twice; then overwrite-all.
    hit(f,&InputFrame::select);hit(f,&InputFrame::select);write(hd/"a.bin","new-a");write(hd/"b.bin","new-b");hit(f,&InputFrame::r3);
    select(f,"a.bin");hit(f,&InputFrame::square);select(f,"b.bin");hit(f,&InputFrame::square);hit(f,&InputFrame::triangle);conflict(f,2);run(f);
    assert(read(usb/"a.bin")=="new-a"&&read(usb/"b.bin")=="new-b"&&f.view().copied_count==2);clean(root);
    puts("PASS: USB-to-HD copy, device chooser, independent directory positions and overwrite/skip-all decisions");
    // Single overwrite cancel, single skip and retry without sticky apply-all.
    select(f,"a.bin");hit(f,&InputFrame::triangle);assert(f.view().mode==FileManagerFix41::Mode::Overwrite);hit(f,&InputFrame::circle);
    assert(!f.copying()&&read(usb/"a.bin")=="new-a");hit(f,&InputFrame::triangle);conflict(f,1);run(f);assert(f.view().skipped_count==1);
    write(hd/"a.bin","retry");hit(f,&InputFrame::r3);select(f,"a.bin");hit(f,&InputFrame::triangle);conflict(f,0);run(f);assert(read(usb/"a.bin")=="retry");
    // Transaction failure restores old target; source mutation never commits.
    write(hd/"a.bin","changed");hit(f,&InputFrame::r3);select(f,"a.bin");hit(f,&InputFrame::triangle);conflict(f,0);fail_commit=true;run(f);
    assert(read(usb/"a.bin")=="retry");clean(root);
    hit(f,&InputFrame::triangle);conflict(f,0);write(hd/"a.bin","source changed while copying");run(f);assert(read(usb/"a.bin")=="retry");clean(root);
    puts("PASS: per-file overwrite/pause/cancel, non-sticky batch policy, rollback on final rename failure and source mutation detection");
    // Cancel after first file completes; finished copies remain, pending ones do not.
    write(hd/"a.bin","done");write(hd/"b.bin",data);hit(f,&InputFrame::r3);select(f,"a.bin");hit(f,&InputFrame::square);select(f,"b.bin");hit(f,&InputFrame::square);
    hit(f,&InputFrame::triangle);conflict(f,2);while(f.view().copied_count<1)f.step();f.step();hit(f,&InputFrame::circle);
    assert(!f.copying()&&read(usb/"a.bin")=="done"&&read(usb/"b.bin")=="new-b");clean(root);
    hit(f,&InputFrame::select);hit(f,&InputFrame::select);select(f,"dest");hit(f,&InputFrame::square);assert(f.view().marked_count==0);
    // 64-bit file length from USB to HDD; cancel after one reused buffer slice.
    write(usb/"large.bin","");fs::resize_file(usb/"large.bin",5ULL*1024*1024*1024+123);hit(f,&InputFrame::r3);hit(f,&InputFrame::right);select(f,"large.bin");hit(f,&InputFrame::triangle);
    assert(f.copying()&&f.view().total==5ULL*1024*1024*1024+123);f.step();assert(f.view().done==256*1024);hit(f,&InputFrame::circle);assert(!fs::exists(hd/"large.bin"));clean(root);
    // A >4 GiB file cannot be copied as one native FAT32 file.
    write(hd/"too-large.bin","");fs::resize_file(hd/"too-large.bin",5ULL*1024*1024*1024+123);hit(f,&InputFrame::left);hit(f,&InputFrame::r3);select(f,"too-large.bin");hit(f,&InputFrame::triangle);
    assert(!f.copying()&&f.view().status.find("4 GB")!=std::string::npos);clean(root);
    puts("PASS: cancellation preserves completed files, directories are not copied, 64-bit sizes and bounded buffer, native FAT32 limit");
    // Existing preferences migrate to V2, including missing USB game records.
    const std::string gamepath="/dev_hdd0/GAMES/Test",usbpath="/dev_usb000/GAMES/Other";
    const std::string payload="filter 0\nselected "+hex(gamepath)+"\ngame 1 1 "+hex(gamepath)+"\n";
    write(root/"prefs","PS3_SP_PREFS29_V1 "+std::to_string(digest(payload))+"\n"+payload);
    PreferencesFix29 pref;assert(pref.load((root/"prefs").string()));CoverflowState s;s.games.resize(2);s.games[0].path=gamepath;s.games[1].path=usbpath;s.visible={0,1};
    pref.apply(s.games);assert(s.games[0].favorite&&s.games[0].case_style==GameCaseStyleFix41::Clear);
    s.games[0].case_style=GameCaseStyleFix41::Platinum;s.games[1].case_style=GameCaseStyleFix41::GreatestHits;pref.capture(s);assert(pref.save((root/"prefs").string()));
    s.games.pop_back();s.visible={0};pref.capture(s);assert(pref.save((root/"prefs").string()));PreferencesFix29 restored;assert(restored.load((root/"prefs").string()));
    s.games.resize(2);s.games[0].case_style=GameCaseStyleFix41::Clear;s.games[1].path=usbpath;restored.apply(s.games);
    assert(s.games[0].case_style==GameCaseStyleFix41::Platinum&&s.games[1].case_style==GameCaseStyleFix41::GreatestHits);
    LibraryBrowserFix28 browser("");browser.replace_catalog(s.games);browser.update(press(&InputFrame::triangle),0);
    for(int i=0;i<4;++i)browser.update(press(&InputFrame::down),0);
    browser.update(press(&InputFrame::cross),0);
    assert(current_game(browser.state())->case_style==GameCaseStyleFix41::GreatestHits&&browser.take_preferences_changed());
    puts("PASS: saved per-game colors, V1-to-V2 preference migration, missing USB record retention and TRIANGLE case-style action");
    // A rotation or browse input defers work, including its settling interval.
    InteractionPolicyFix41 activity;InputFrame in;in.connected=true;in.right_x=1;activity.note(in,1000000);assert(!activity.quiet(1000001));assert(activity.quiet(1650000));
    in.right_x=0;in.left.pressed=true;activity.note(in,1700000);assert(!activity.idle(2300000,s));assert(activity.idle(2350000,s));
    CasePose fading;fading.visibility=.5f;s.flow={fading};assert(!activity.idle(3000000,s));s.flow[0].visibility=1;assert(activity.idle(3000000,s));
    V14MeshPart outer;outer.material=V14MeshPart::Material::ClearPlastic;outer.external_plastic=true;
    assert(CaseStyleFix41::applies(outer,GameCaseStyleFix41::Platinum,2)&&!CaseStyleFix41::applies(outer,GameCaseStyleFix41::Platinum,1));
    outer.external_plastic=false;assert(!CaseStyleFix41::applies(outer,GameCaseStyleFix41::Platinum,2));outer.material=V14MeshPart::Material::DiscGlass;outer.external_plastic=true;
    assert(!CaseStyleFix41::applies(outer,GameCaseStyleFix41::GreatestHits,2));
    puts("PASS: manual navigation/rotation and settling defer prefetch; colors apply exclusively to exterior plastic faces");
    f.close();fs::remove_all(root);
}
