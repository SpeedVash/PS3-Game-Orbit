#include "physical_disc_fix47.h"
#include "physical_disc_monitor_fix47.h"
#include "preferences_fix29.h"
#include "library_browser_fix28.h"
#include "game_tools_fix35.h"
#include "game_order_fix40.h"
#include "mount_operation_fix29.h"
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <thread>

namespace fs=std::filesystem;
namespace {
DiscAccessFix47 physical(){return DiscAccessFix47::Physical;}
DiscAccessFix47 emulated(){return DiscAccessFix47::Emulated;}
DiscAccessFix47 absent(){return DiscAccessFix47::None;}
DiscAccessFix47 unknown(){return DiscAccessFix47::Unknown;}
unsigned race_calls=0;
DiscAccessFix47 removed_during_read(){return ++race_calls==1?DiscAccessFix47::Physical:DiscAccessFix47::None;}
std::atomic<bool> blocked{false},release_read{false};
std::atomic<int> sample_mode{0},sample_calls{0};
DiscSampleFix47 sample_a,sample_b;
DiscSampleFix47 controlled_sample(const std::string&,DiscProbeFix47,const std::string&){
    ++sample_calls;
    if(sample_mode.load()==3){blocked.store(true);while(!release_read.load())std::this_thread::sleep_for(std::chrono::milliseconds(1));}
    return sample_mode.load()==1?sample_a:sample_mode.load()==2?sample_b:DiscSampleFix47{};
}
template<class Predicate>void wait_for(Predicate predicate){
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(3);
    while(!predicate()){assert(std::chrono::steady_clock::now()<deadline);std::this_thread::sleep_for(std::chrono::milliseconds(2));}
}
GameEntry disk_game(const std::string& path,const std::string& id){
    GameEntry game;game.path=path;game.title_id=id;game.title=id;
    game.source=GameSource::Disc;game.format=GameFormat::Disc;game.library_key="/dev_bdvd/"+id;game.physical_disc=true;return game;
}
}
int main(int argc,char** argv){
    assert(argc==2);const fs::path root=argv[1];const auto bd=(root/"bd").string(),covers=(root/"covers").string();
    auto a=read_disc_sample_fix47(bd,physical,covers);
    assert(a.present && a.game.title_id=="BLES00001" && a.game.title=="Jogo original A");
    assert(a.game.source==GameSource::Disc && a.game.format==GameFormat::Disc && a.game.physical_disc);
    assert(a.game.path==bd && a.game.identity_path()=="/dev_bdvd/BLES00001");
    assert(a.game.cover_kind==GameCoverKind::FullCover && a.game.cover_path==covers+"/BLES00001.jpg");
    assert(a.game.inside_cover_path==covers+"/BLES00001_INSIDE.png" && a.game.disc_art_path==covers+"/BLES00001_DISC.png");
    assert(!read_disc_sample_fix47(bd,emulated,covers).present && !read_disc_sample_fix47(bd,absent,covers).present);
    const auto fallback=read_disc_sample_fix47(bd,unknown,covers);assert(fallback.present && !fallback.game.physical_disc);
    race_calls=0;assert(!read_disc_sample_fix47(bd,removed_during_read,covers).present);
    puts("PASS: physical PS3 title/ID, ID-based cover/inside/disc, emulated/no-disc suppression, honest unsupported-probe fallback and removal during SFO read");

    auto b=read_disc_sample_fix47((root/"bd_b").string(),physical,covers);assert(b.present && b.game.title_id=="BLUS00002");
    assert(!read_disc_sample_fix47((root/"bad_id").string(),physical,covers).present);
    assert(!read_disc_sample_fix47((root/"bad_sfo").string(),physical,covers).present);
    assert(!read_disc_sample_fix47((root/"update").string(),physical,covers).present);
    assert(!read_disc_sample_fix47((root/"ps1").string(),physical,covers).present);
    const auto front=read_disc_sample_fix47((root/"front").string(),physical,covers);
    assert(front.present && front.game.cover_kind==GameCoverKind::FrontOnly && front.game.inside_cover_path.empty());
    const auto neutral=read_disc_sample_fix47((root/"neutral").string(),physical,covers);
    assert(neutral.present && neutral.game.cover_kind==GameCoverKind::None && neutral.game.disc_art_path.empty());
    puts("PASS: malformed metadata, invalid IDs and non-PS3 content rejected; ICON0 fallback is front-only; a PS3 disc without art remains available");

    GameEntry hd;hd.path="/dev_hdd0/GAMES/A";hd.title="A";GameEntry usb;usb.path="/dev_usb000/PS3ISO/B.iso";usb.title="B";usb.source=GameSource::USB;
    std::vector<GameEntry> catalog{hd,usb};
    auto change=sync_disc_catalog_fix47(catalog,a);assert(change.changed && change.index==2 && change.preserves_indices && catalog.size()==3);
    assert(!sync_disc_catalog_fix47(catalog,a).changed);
    change=sync_disc_catalog_fix47(catalog,b);assert(change.changed && change.index==2 && change.preserves_indices && catalog.size()==3);
    assert(catalog[0].path==hd.path && catalog[1].path==usb.path);
    change=sync_disc_catalog_fix47(catalog,{});assert(change.changed && change.preserves_indices && catalog.size()==2);
    catalog={a.game,hd};change=sync_disc_catalog_fix47(catalog,{});assert(change.changed && !change.preserves_indices && catalog[0].path==hd.path);

    LibraryBrowserFix28 browser("","");browser.replace_catalog({hd,usb});browser.restore_view(.7f,43.f,-9.f,false,true);
    const auto pose=browser.state();browser.replace_catalog({hd,usb,a.game},true);
    assert(current_game(browser.state())->path==hd.path && browser.state().center_yaw_deg==pose.center_yaw_deg && browser.state().center_pitch_deg==pose.center_pitch_deg);
    auto* disk=browser.entry(2);assert(disk);disk->favorite=true;disk->case_style=static_cast<GameCaseStyleFix41>(1);
    browser.restore_selection(FilterMode::All,a.game.identity_path());
    PreferencesFix29 preferences;preferences.capture(browser.state());assert(preferences.save((root/"preferences.dat").string()));
    PreferencesFix29 restored;assert(restored.load((root/"preferences.dat").string()));
    std::vector<GameEntry> others{b.game};restored.apply(others);assert(!others[0].favorite && others[0].case_style==GameCaseStyleFix41::Clear);
    others={a.game};restored.apply(others);assert(others[0].favorite && others[0].case_style==static_cast<GameCaseStyleFix41>(1));
    assert(restored.selected_path()==a.game.identity_path());
    GameToolsFix35::Names names;assert(names.set(a.game.identity_path(),"Meu original A"));
    others={a.game,b.game};names.apply(others);assert(others[0].title=="Meu original A" && others[1].title==b.game.title);
    browser.replace_catalog({hd,usb,b.game});assert(!browser.entry(2)->favorite);
    CoverflowState filters;filters.games={hd,usb,a.game};filters.filter=FilterMode::All;rebuild_visible(filters);assert(filters.visible.size()==3);
    filters.filter=FilterMode::HDD;rebuild_visible(filters);assert(filters.visible.size()==1 && filters.visible[0]==0);
    filters.filter=FilterMode::USB;rebuild_visible(filters);assert(filters.visible.size()==1 && filters.visible[0]==1);
    filters.games[2].favorite=true;filters.filter=FilterMode::Favorites;rebuild_visible(filters);assert(filters.visible.size()==1 && filters.visible[0]==2);
    std::vector<std::string> order;GameOrderFix40::reconcile(order,{a.game,b.game});assert(order.size()==2 && order[0]!=order[1]);
    puts("PASS: append/swap/remove keeps HD/USB indices; selection and camera retained; filters, favorites, case style, names and custom-order identities do not leak between different discs");

    MountOperationFix29 mount;assert(mount.start(a.game,100));
    assert(mount.state()==MountOperationFix29::State::Confirmed && mount.busy() && !mount.needs_disc_check() && mount.http_status()==0);
    assert(!mount.exit_ready(1299) && mount.exit_ready(1300));
    auto wrong=a.game;wrong.title_id="BLUS00002";MountOperationFix29 reject;assert(!reject.start(wrong,100) && reject.state()==MountOperationFix29::State::Failed);
    const auto before=fs::file_size(root/"bd/PS3_GAME/PARAM.SFO");assert(before>20);
    puts("PASS: X on the current disc confirms direct XMB return without HTTP/webMAN mounting; a changed disc ID blocks the stale selection");

    sample_a=a;sample_b=b;sample_mode.store(3);sample_calls.store(0);
    PhysicalDiscMonitorFix47 monitor(bd,physical,covers,2,controlled_sample);assert(monitor.start({}));
    wait_for([]{return blocked.load();});DiscSampleFix47 event;
    const auto begin=std::chrono::steady_clock::now();assert(!monitor.take(event));
    assert(std::chrono::steady_clock::now()-begin<std::chrono::milliseconds(30));
    release_read.store(true);sample_mode.store(1);wait_for([&]{return monitor.take(event);});assert(event.present && event.game.title_id==a.game.title_id);
    sample_mode.store(2);wait_for([&]{return monitor.take(event);});assert(event.present && event.game.title_id==b.game.title_id);
    sample_mode.store(0);wait_for([&]{return monitor.take(event);});assert(!event.present);
    monitor.stop();assert(!monitor.take(event));
    assert(monitor.start({a.game}));sample_mode.store(2);wait_for([&]{return monitor.take(event);});assert(event.game.title_id==b.game.title_id);monitor.stop();
    puts("PASS: background detection publishes insertion/swap/removal snapshots; UI polling stays nonblocking during an optical read; worker restart and shutdown are joined");
    (void)disk_game;
}
