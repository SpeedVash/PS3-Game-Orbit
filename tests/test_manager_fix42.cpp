#include <cassert>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include "file_manager_fix42.h"
namespace fs=std::filesystem;
using Manager=FileManagerFix42;
using Mode=Manager::Mode;
static bool fail_commit=false;
extern "C" int __real_rename(const char*,const char*);
extern "C" int __wrap_rename(const char* from,const char* to){
    if(fail_commit&&std::string(from).find(".orbit152_tmp_")!=std::string::npos){fail_commit=false;errno=EIO;return -1;}
    if(fs::exists(to)){errno=EEXIST;return -1;}
    return __real_rename(from,to);
}
static void write(const fs::path& p,const std::string& data){std::ofstream f(p,std::ios::binary);f.write(data.data(),data.size());assert(f.good());}
static std::string read(const fs::path& p){std::ifstream f(p,std::ios::binary);assert(f.good());return {std::istreambuf_iterator<char>(f),{}};}
static void hit(Manager& m,ButtonEdge InputFrame::* key){InputFrame in;in.connected=true;(in.*key).pressed=true;m.update(in,.016f);}
static void select(Manager& m,const std::string& name){
    const auto& p=m.view().panels[m.view().active_panel];int target=-1;
    for(std::size_t i=0;i<p.entries.size();++i)if(p.entries[i].name==name)target=int(i);
    assert(target>=0);
    while(m.view().panels[m.view().active_panel].selected<target)hit(m,&InputFrame::down);
    while(m.view().panels[m.view().active_panel].selected>target)hit(m,&InputFrame::up);
}
static void action(Manager& m,int row){
    assert(m.view().mode==Mode::Browse);hit(m,&InputFrame::triangle);assert(m.view().mode==Mode::Actions);
    for(int i=0;i<row;++i){hit(m,&InputFrame::down);}hit(m,&InputFrame::cross);
}
static void refresh(Manager& m){action(m,3);}
static void prepare(Manager& m){
    action(m,0);unsigned guard=20000;
    while(m.view().mode==Mode::Planning&&guard--){m.step();}assert(guard>0);
}
static void begin(Manager& m){prepare(m);assert(m.view().mode==Mode::Confirm);hit(m,&InputFrame::cross);assert(m.copying());}
static void run(Manager& m){
    unsigned guard=20000;
    while(m.copying()&&guard--){assert(m.view().mode!=Mode::Overwrite);m.step();}assert(guard>0&&m.view().mode==Mode::Summary);
}
static void until_conflict(Manager& m){
    unsigned guard=20000;
    while(m.copying()&&m.view().mode!=Mode::Overwrite&&guard--){m.step();}assert(guard>0&&m.view().mode==Mode::Overwrite);
}
static void choice(Manager& m,int row){
    assert(m.view().mode==Mode::Overwrite);
    while(m.view().overwrite_selected!=row){hit(m,&InputFrame::down);}hit(m,&InputFrame::cross);
}
static bool flag(const Manager& m,int panel,const std::string& name,bool copied){
    for(const auto& e:m.view().panels[panel].entries){if(e.name==name)return copied?e.copied:e.marked;}return false;
}
static void clean(const fs::path& p){for(const auto& e:fs::recursive_directory_iterator(p))
    assert(e.path().filename().string().rfind(".orbit152_",0)!=0);}
struct Fixture {
    fs::path root,hd,usb;Manager manager;
    static fs::path temporary(){char p[]="/tmp/orbit152-manager-XXXXXX";const char* r=mkdtemp(p);assert(r);return r;}
    Fixture():root(temporary()),hd(root/"dev_hdd0"),usb(root/"dev_usb000"),manager(std::vector<std::string>{hd.string(),usb.string()}){
        fs::create_directories(hd);fs::create_directories(usb);
    }
    void open(){manager.open();assert(manager.active()&&manager.devices().size()==2);hit(manager,&InputFrame::cross);assert(manager.view().mode==Mode::Browse);}
    ~Fixture(){manager.close();fs::remove_all(root);}
};
int main(){
    const std::string data(2*1024*1024+173,'q');
    {
        Fixture t;write(t.hd/"a.bin",data);write(t.hd/"b.bin","second");write(t.hd/"empty.bin","");fs::create_directories(t.usb/"target");t.open();auto& m=t.manager;
        action(m,1);assert(m.view().marked_count==3&&m.view().selection_panel==0);
        hit(m,&InputFrame::r1);select(m,"target");hit(m,&InputFrame::cross);
        assert(m.view().marked_count==3&&m.view().selection_panel==0);
        prepare(m);assert(m.view().mode==Mode::Confirm&&m.view().source_panel==0&&m.view().file_count==3);
        assert(m.view().batch_total==data.size()+6&&fs::is_empty(t.usb/"target"));
        assert(m.view().source_directory==t.hd&&m.view().destination_directory==t.usb/"target");
        hit(m,&InputFrame::circle);assert(m.view().mode==Mode::Browse&&fs::is_empty(t.usb/"target"));
        begin(m);run(m);assert(m.view().copied_count==3&&m.view().marked_count==0&&m.view().error_count==0);
        assert(read(t.usb/"target/a.bin")==data&&read(t.usb/"target/b.bin")=="second"&&fs::file_size(t.usb/"target/empty.bin")==0);
        assert(flag(m,0,"a.bin",true)&&flag(m,1,"a.bin",true));clean(t.root);
        hit(m,&InputFrame::cross);select(m,"b.bin");hit(m,&InputFrame::square);
        hit(m,&InputFrame::l1);select(m,"a.bin");hit(m,&InputFrame::square);
        assert(m.view().selection_panel==1&&m.view().marked_count==1&&!flag(m,0,"a.bin",false));
        action(m,2);assert(m.view().marked_count==0&&m.view().selection_panel==-1);
        select(m,"a.bin");hit(m,&InputFrame::square);hit(m,&InputFrame::circle);assert(m.view().marked_count==0);
        hit(m,&InputFrame::r3);assert(m.view().mode==Mode::Devices);hit(m,&InputFrame::cross);assert(m.view().panels[0].path==t.hd);
        hit(m,&InputFrame::select);assert(m.view().mode==Mode::Details);hit(m,&InputFrame::r2);assert(m.view().details_page==1);hit(m,&InputFrame::circle);
        hit(m,&InputFrame::r1);select(m,"b.bin");begin(m);until_conflict(m);choice(m,0);run(m);assert(read(t.hd/"b.bin")=="second");
        puts("PASS: source stays fixed on destination panel, preview writes nothing, HD/USB both directions, exact bytes, zero-byte file, selection ownership, device and path controls");
    }
    {
        Fixture t;fs::create_directories(t.hd/"tree/empty");fs::create_directories(t.hd/"tree/sub");
        write(t.hd/"tree/a.bin",data);write(t.hd/"tree/sub/b.bin","nested");fs::create_directories(t.usb/"tree");write(t.usb/"tree/keep.bin","keep");t.open();auto& m=t.manager;
        select(m,"tree");hit(m,&InputFrame::square);prepare(m);assert(m.view().mode==Mode::Confirm&&m.view().folder_count==3&&m.view().file_count==2);
        assert(m.view().batch_total==data.size()+6&&!fs::exists(t.usb/"tree/empty"));hit(m,&InputFrame::cross);run(m);
        assert(read(t.usb/"tree/a.bin")==data&&read(t.usb/"tree/sub/b.bin")=="nested"&&read(t.usb/"tree/keep.bin")=="keep");
        assert(fs::is_empty(t.usb/"tree/empty")&&m.view().directories_done==3&&flag(m,0,"tree",true)&&flag(m,1,"tree",true));clean(t.root);
        hit(m,&InputFrame::cross);select(m,"tree");hit(m,&InputFrame::square);begin(m);until_conflict(m);assert(m.view().overwrite_selected==1);
        choice(m,3);run(m);assert(m.view().skipped_count==2&&m.view().copied_count==0&&m.view().marked_count==1);
        hit(m,&InputFrame::cross);write(t.hd/"tree/a.bin","new");begin(m);until_conflict(m);
        assert(m.view().overwrite_selected==1);choice(m,2);run(m);assert(m.view().copied_count==2&&read(t.usb/"tree/a.bin")=="new");clean(t.root);
        puts("PASS: recursive folder and empty-folder copy, merge keeps unrelated files, accurate count and size, skip-all and overwrite-all reset per batch");
    }
    {
        Fixture t;fs::create_directories(t.hd/"tree/sub");write(t.hd/"tree/a","safe");t.open();auto& m=t.manager;
        select(m,"tree");hit(m,&InputFrame::square);hit(m,&InputFrame::r1);hit(m,&InputFrame::r3);hit(m,&InputFrame::up);hit(m,&InputFrame::cross);
        select(m,"tree");hit(m,&InputFrame::cross);prepare(m);assert(m.view().mode==Mode::Summary&&m.view().error_count==1&&!fs::exists(t.hd/"tree/tree"));
        hit(m,&InputFrame::cross);hit(m,&InputFrame::circle);hit(m,&InputFrame::l1);prepare(m);assert(m.view().mode==Mode::Summary);clean(t.root);
        puts("PASS: copying a folder into itself or its descendants is refused before any destination is created");
    }
    {
        Fixture t;fs::create_directories(t.hd/"dir");write(t.usb/"dir","existing file");write(t.hd/"other","valid");t.open();auto& m=t.manager;
        select(m,"dir");prepare(m);assert(m.view().mode==Mode::Summary&&m.view().error_count==1&&read(t.usb/"dir")=="existing file");
        hit(m,&InputFrame::cross);fs::create_symlink(t.hd/"other",t.hd/"dir/link");fs::remove(t.usb/"dir");refresh(m);select(m,"dir");prepare(m);
        assert(m.view().mode==Mode::Summary&&m.view().error_count==1&&!fs::exists(t.usb/"dir"));clean(t.root);
        puts("PASS: incompatible target type and unsupported nested symlink stop preparation without writing");
    }
    {
        Fixture t;fs::create_directories(t.hd/"many");for(int i=0;i<80;++i)write(t.hd/"many"/std::to_string(i),"x");t.open();auto& m=t.manager;
        select(m,"many");action(m,0);assert(m.view().mode==Mode::Planning);m.step();assert(m.view().mode==Mode::Planning);
        hit(m,&InputFrame::start);assert(m.view().mode==Mode::ExitConfirm);const auto revision=m.view().revision;m.step();assert(m.view().revision==revision);
        hit(m,&InputFrame::circle);assert(m.view().mode==Mode::Planning);hit(m,&InputFrame::circle);assert(m.view().cancelled&&!fs::exists(t.usb/"many"));
        hit(m,&InputFrame::cross);begin(m);hit(m,&InputFrame::start);hit(m,&InputFrame::down);hit(m,&InputFrame::cross);
        assert(!m.active()&&!m.copying()&&!fs::exists(t.usb/"many"));clean(t.root);
        puts("PASS: bounded preparation, cancel without writes, START pauses work and asks before discarding an active batch");
    }
    {
        Fixture t;write(t.hd/"a.bin","new");write(t.usb/"a.bin","old");t.open();auto& m=t.manager;
        select(m,"a.bin");begin(m);until_conflict(m);choice(m,0);fail_commit=true;run(m);
        assert(m.view().error_count==1&&m.view().copied_count==0&&read(t.usb/"a.bin")=="old");clean(t.root);
        hit(m,&InputFrame::cross);begin(m);until_conflict(m);choice(m,0);write(t.hd/"a.bin","changed during copy");run(m);
        assert(m.view().error_count==1&&read(t.usb/"a.bin")=="old");clean(t.root);
        hit(m,&InputFrame::cross);begin(m);until_conflict(m);choice(m,0);write(t.usb/"a.bin","target changed during copy");run(m);
        assert(m.view().error_count==1&&read(t.usb/"a.bin")=="target changed during copy");clean(t.root);
        puts("PASS: failed final rename restores existing file; changing source or destination blocks commit and removes temporary data");
    }
    {
        Fixture t;write(t.hd/"a.bin","done");write(t.hd/"b.bin",data);write(t.usb/"b.bin","old");t.open();auto& m=t.manager;
        action(m,1);begin(m);while(m.view().copied_count<1)m.step();until_conflict(m);choice(m,0);m.step();assert(m.view().done==256*1024);
        hit(m,&InputFrame::circle);assert(m.view().cancelled&&m.view().copied_count==1&&read(t.usb/"a.bin")=="done"&&read(t.usb/"b.bin")=="old");
        assert(flag(m,0,"a.bin",true)&&m.view().marked_count==1);clean(t.root);
        puts("PASS: cancelling mid-file preserves completed copies, pending selection and previous destination");
    }
    {
        Fixture t;write(t.usb/"large.bin","");const auto size=5ULL*1024*1024*1024+123;fs::resize_file(t.usb/"large.bin",size);t.open();auto& m=t.manager;
        hit(m,&InputFrame::r1);select(m,"large.bin");begin(m);assert(m.view().batch_total==size);m.step();m.step();assert(m.view().total==size&&m.view().done==256*1024);
        hit(m,&InputFrame::circle);assert(!fs::exists(t.hd/"large.bin"));clean(t.root);
        write(t.hd/"large.bin","");fs::resize_file(t.hd/"large.bin",size);hit(m,&InputFrame::cross);hit(m,&InputFrame::l1);refresh(m);select(m,"large.bin");prepare(m);
        assert(m.view().mode==Mode::Summary&&m.view().status.find("4 GB")!=std::string::npos);clean(t.root);
        puts("PASS: 64-bit length, one reused 256 KiB buffer and FAT32 single-file limit enforced during preview");
    }
    {
        Fixture t;fs::create_directories(t.hd/"limit");for(unsigned i=0;i<16385;++i)write(t.hd/"limit"/std::to_string(i),"");t.open();auto& m=t.manager;
        select(m,"limit");prepare(m);assert(m.view().mode==Mode::Summary&&m.view().status.find("limite de preparação")!=std::string::npos&&!fs::exists(t.usb/"limit"));
        clean(t.root);puts("PASS: recursive job limit stops oversized batches before any destination write");
    }
#ifdef __PSL1GHT__
    {
        Fixture t;write(t.hd/"blockopen.bin","blocked");t.open();auto& m=t.manager;select(m,"blockopen.bin");begin(m);run(m);
        assert(m.view().error_count==1&&m.view().status.find("80010009")!=std::string::npos&&!fs::exists(t.usb/"blockopen.bin"));
        assert(native_opens>0&&native_reads>0&&native_fstats>0&&native_mkdirs>0&&!native_free_device.empty()&&native_dirs.empty());clean(t.root);
        puts("PASS: real sysFs branch, independent descriptor timestamps, native mkdir, destination-volume free space, descriptor cleanup and exact open error code");
    }
#endif
}
