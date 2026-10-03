#!/usr/bin/env python3
"""Execute the real main-library block against resource-ordering API stubs.

This checks ownership and exit sequencing; it is not a PS3 filesystem/GPU test.
"""
from pathlib import Path
import hashlib
import subprocess
import tempfile

root=Path(__file__).resolve().parent.parent
source=(root/'src/main.cpp').read_text()
start=source.index('    if(result==0 && !user_exit){\n        // The last calibration')
begin=source.index('{',start);end=begin+1;depth=1
while depth:
    depth+=(source[end]=='{')-(source[end]=='}');end+=1
block=source[begin+1:end-1]
prefix=r'''
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>
#include <unordered_set>
#include "library_pair_fix28.h"
#include "cover_orientation_fix28.h"
#include "library_browser_fix28.h"
#include "cover_cache.h"
#include "full_cover_case_fix26.h"
using s64=long long;
static bool open_frame;
static unsigned polls,renders,acks,hud_calls,uploads,releases,rescans,decode_failures;
static bool timed;
static s64 clock_value;
static std::string fixture;
static s64 sysGetSystemTime(){clock_value+=timed ? 60000000 : 16667;return clock_value;}
namespace RuntimeDiag { template<class... A> void log(const char*,A...) {} }
struct Stream {
    bool begin_frame(){assert(!open_frame);open_frame=true;return true;}
    bool complete_frame(){assert(open_frame);open_frame=false;++acks;return true;}
    unsigned completed_frames() const {return acks;}
    unsigned switches() const {return acks;}
};
struct Lifecycle {void pump(){} bool exit_requested() const {return false;}};
struct Pad {
    InputFrame poll(){
        InputFrame input;input.connected=true;
        if(timed){++polls;return input;}
        switch(polls++){
            case 1:case 2:input.right.pressed=true;break;
            case 3:input.l1.pressed=true;break;
            case 4:input.r1.pressed=true;break;
            case 5:input.start.pressed=true;break;
            case 6:input.select.pressed=true;break;
            case 7:input.cross.pressed=true;break;
            case 8:input.right_x=1;break;
            case 9:input.circle.pressed=true;break;
        }
        assert(polls<12);return input;
    }
};
struct Renderer {
    std::string error;
    unsigned covers=1;std::unordered_set<int> uploaded;
    struct Stats {unsigned draw_calls=6,hud_draw_calls=1;} stats;
    bool sync_visible_covers(const CoverflowState& state,CoverCache&,int radius){
        assert(!open_frame && radius==1);++uploads;uploaded.clear();
        for(const auto& pose:LibraryPairFix28::poses(state,radius)){
            const auto& game=state.games[pose.game_index];
            if(game.path=="GAME2" && !game.cover_path.empty()) ++decode_failures;
            else if(!game.cover_path.empty()) uploaded.insert(pose.game_index);
        }
        covers=uploaded.size();return true;
    }
    bool has_cover_texture(int index) const {return uploaded.count(index);}
    bool has_full_cover_texture(int index) const {return uploaded.count(index);}
    void clear_cover_textures(){assert(!open_frame);covers=0;++releases;}
    unsigned gpu_cover_count() const {return covers;}
    bool set_library_hud(const LibraryHudLinesFix28& lines){
        assert(!open_frame);assert(!lines[0].empty() && !lines[1].empty());++hud_calls;return true;
    }
    void set_diagnostic_view(const char*,V14Surface){}
    bool render(const CoverflowState& state,const V14CaseMesh&,int radius,float scale){
        assert(open_frame && radius==1 && scale>=0.55f && scale<=0.80f);
        assert(current_game(state));stats.draw_calls=LibraryPairFix28::poses(state,1).size()*6;++renders;return true;
    }
    const Stats& last_stats() const {return stats;}
    const std::string& last_error() const {return error;}
};
static std::vector<GameEntry> scan_catalog(){
    assert(!open_frame);++rescans;
    std::vector<GameEntry> games(3);
    for(unsigned i=0;i<games.size();++i){
        games[i].title="Test "+std::to_string(i);games[i].path="GAME"+std::to_string(i);
        if(i!=1){games[i].cover_path=fixture;games[i].cover_kind=GameCoverKind::FullCover;}
    }
    return games;
}
int main(int argc,char** argv){
    assert(argc==2);fixture=argv[1];
    for(unsigned scenario=0;scenario<2;++scenario){
        timed=scenario==1;open_frame=false;clock_value=0;
        polls=renders=acks=hud_calls=uploads=releases=rescans=decode_failures=0;
        int result=0;bool user_exit=false,time_limit=false;
        Stream stream;Lifecycle lifecycle;Pad controller;Renderer renderer;CoverCache cache(3);
        LibraryBrowserFix28 browser(fixture);const auto mesh=build_full_cover_case_fix26();
'''
suffix=r'''
        assert(result==0 && !open_frame && renders==acks && renders==hud_calls);
        if(!timed){
            assert(user_exit && !time_limit && renders==9 && rescans==2 && releases==1 && decode_failures==1);
            puts("PASS: actual library main block scans, changes selected covers, recovers a decode failure, handles empty favorites/rescan, and changes all resources only between acknowledged frames");
            puts("PASS: circle exits before another upload/HUD/draw submission");
        }else{
            assert(!user_exit && time_limit && renders==4 && polls==4);
            puts("PASS: five-minute library test limit exits before polling/submitting the next frame");
        }
    }
    puts("HOST CONTROL-FLOW MODEL: API stubs do not prove PS3 I/O latency, RSX texture visibility or hardware coherence");
}
'''
print('MAIN_SOURCE_SHA256: '+hashlib.sha256((root/'src/main.cpp').read_bytes()).hexdigest(),flush=True)
with tempfile.TemporaryDirectory(prefix='fix28-library-loop-') as directory:
    folder=Path(directory)
    upload='if(!renderer.set_library_hud(browser.hud_lines(cover_status))){result=32;break;}\n            if(!stream.begin_frame()){result=29;break;}'
    assert upload in block
    variants={'corrected':block,'hud-upload-during-frame':block.replace(upload,'if(!stream.begin_frame()){result=29;break;}\n            if(!renderer.set_library_hud(browser.hud_lines(cover_status))){result=32;break;}')}
    for name,body in variants.items():
        cpp=folder/(name+'.cpp');binary=folder/name;cpp.write_text(prefix+body+suffix)
        subprocess.run(['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror','-I'+str(root/'include'),
                        str(cpp),*[str(root/'src'/file) for file in ('library_browser_fix28.cpp','app_controller.cpp','inspect_case_fix25.cpp','safe_boot.cpp','coverflow_state.cpp','cover_cache.cpp','cover_image.cpp','v14_case_mesh.cpp','full_cover_case_fix26.cpp','render_plan.cpp','library_pair_fix28.cpp','cover_orientation_fix28.cpp')],'-o',str(binary)],check=True,timeout=30)
        run=subprocess.run([str(binary),str(root/'pkgfiles/USRDIR/FULL_COVER_FIX26_CONTINUA.png')],capture_output=True,text=True,timeout=15)
        if name=='corrected':
            print(run.stdout,end='',flush=True)
            if run.returncode:print(run.stderr)
            assert run.returncode==0
        else:
            assert run.returncode!=0,'Resource ownership regression escaped the model'
            print('PASS: HUD upload during an open frame regression rejected',flush=True)
