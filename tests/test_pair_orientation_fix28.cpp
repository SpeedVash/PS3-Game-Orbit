#include <array>
#include <cassert>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <string>
#include "cover_orientation_fix28.h"
#include "library_pair_fix28.h"
#include "library_browser_fix28.h"
#include "safe_boot.h"
#include "full_cover_case_fix26.h"
#define private public
#include "rsx_renderer_v10.h"
#undef private

static CoverImage exif(unsigned value,bool little,bool png){
    std::vector<unsigned char> t(26,0);t[0]=t[1]=little ? 'I' : 'M';
    auto w16=[&](unsigned p,unsigned v){t[p]=little ? v : v>>8;t[p+1]=little ? v>>8 : v;};
    auto w32=[&](unsigned p,unsigned v){for(unsigned i=0;i<4;++i)t[p+i]=v>>(little ? i*8 : (3-i)*8);};
    w16(2,42);w32(4,8);w16(8,1);w16(10,0x112);w16(12,3);w32(14,1);w16(18,value);
    CoverImage image;image.width=6;image.height=4;image.kind=GameCoverKind::FullCover;
    if(png){
        image.format=CoverFileFormat::PNG;image.encoded={137,80,78,71,13,10,26,10,0,0,0,26,'e','X','I','f'};
        image.encoded.insert(image.encoded.end(),t.begin(),t.end());image.encoded.insert(image.encoded.end(),4,0);
    }else{
        image.format=CoverFileFormat::JPEG;image.encoded={0xff,0xd8,0xff,0xe1,0,34,'E','x','i','f',0,0};
        image.encoded.insert(image.encoded.end(),t.begin(),t.end());image.encoded.insert(image.encoded.end(),{0xff,0xd9});
    }
    return image;
}
static DecodedImageRGBA pattern(){
    DecodedImageRGBA image;image.width=3;image.height=2;image.pitch=12;
    for(unsigned n=1;n<=6;++n) image.rgba.insert(image.rgba.end(),{static_cast<unsigned char>(n),17,29,255});
    return image;
}
int main(int argc,char** argv){
    assert(argc==2);const std::string root=argv[1];
    for(bool little:{false,true}) for(bool png:{false,true}) for(unsigned value=1;value<=8;++value)
        assert(read_cover_orientation_fix28(exif(value,little,png))==value);
    for(unsigned size=0;size<38;++size){auto image=exif(3,true,false);image.encoded.resize(size);const auto o=read_cover_orientation_fix28(image);assert(o==1 || o==3);}
    auto malformed=exif(3,true,false);malformed.encoded[12+4]=255;assert(read_cover_orientation_fix28(malformed)==1);
    const std::array<std::array<unsigned char,6>,8> expected={{{1,2,3,4,5,6},{3,2,1,6,5,4},{6,5,4,3,2,1},{4,5,6,1,2,3},
                                                            {1,4,2,5,3,6},{4,1,5,2,6,3},{6,3,5,2,4,1},{3,6,2,5,1,4}}};
    std::string error;
    for(unsigned value=1;value<=8;++value){
        auto image=pattern();assert(orient_cover_rgba_fix28(image,value,error));
        assert(image.width==(value<5 ? 3 : 2) && image.height==(value<5 ? 2 : 3));
        for(unsigned n=0;n<6;++n) assert(image.rgba[n*4]==expected[value-1][n]);
    }
    auto header=exif(1,true,false);header.width=1000;header.height=640;
    assert(!header.looks_like_canonical_full_cover() && uses_full_cover_layout_fix28(header));
    header.kind=GameCoverKind::FrontOnly;assert(!uses_full_cover_layout_fix28(header));
    header.kind=GameCoverKind::FullCover;header.width=640;header.height=1000;
    assert(!uses_full_cover_layout_fix28(header) && uses_full_cover_layout_fix28(header,6));
    header.encoded=exif(6,true,false).encoded;assert(uses_full_cover_layout_fix28(header));
    const auto full=root+"/pkgfiles/USRDIR/FULL_COVER_FIX26_CONTINUA.png";
    const auto second=root+"/pkgfiles/USRDIR/FULL_COVER_FIX28_SEGUNDA.png";
    auto state=make_safe_boot_state(full);auto neighbor=state.games[0];neighbor.path="SECOND";neighbor.cover_path=second;
    state.games.push_back(neighbor);state.visible.push_back(1);
    auto poses=LibraryPairFix28::poses(state,1);assert(poses.size()==2 && poses[0].game_index!=poses[1].game_index && poses[0].selected && !poses[1].selected);
    assert(poses[0].x==0 && poses[1].x==185 && poses[1].yaw_deg==-36);
    state.selected=1;poses=LibraryPairFix28::poses(state,1);assert(poses[0].game_index==1 && poses[1].game_index==0);
    state.selected=0;state.visible={0};assert(LibraryPairFix28::poses(state,1).size()==1);
    state.visible.clear();assert(LibraryPairFix28::poses(state,1).empty());state.visible={0,1};
    LibraryBrowserFix28 browser(full,second);browser.replace_catalog(state.games);
    InputFrame input;input.connected=true;input.l3.pressed=true;
    const unsigned modes[]={3,6,8,2,4,1};
    for(unsigned mode:modes){browser.update(input,0);assert(current_game(browser.state())->cover_orientation==mode);}
    browser.update(input,0);assert(current_game(browser.state())->cover_orientation==3);
    browser.replace_catalog(state.games);assert(current_game(browser.state())->cover_orientation==3);
    browser.replace_catalog({});assert(browser.render_state().visible.size()==2 && browser.state().visible.empty());
    RsxStage1 stage;assert(stage.init());RsxRendererV10 renderer;const auto mesh=build_full_cover_case_fix26();assert(renderer.init(stage,mesh,false));
    CoverCache cache(3);assert(renderer.sync_visible_covers(state,cache,1));assert(renderer.gpu_cover_count()==2);
    assert(renderer.covers_[0].texture.gpu_ptr!=renderer.covers_[1].texture.gpu_ptr);
    assert(renderer.has_full_cover_texture(0) && renderer.has_full_cover_texture(1));
    const auto image=load_cover_file(second,GameCoverKind::FullCover);assert(!image.looks_like_canonical_full_cover() && uses_full_cover_layout_fix28(image));
    GpuTextureStage1 rotated;assert(stage.prepare_cover(image,rotated,3));
    DecodedImageRGBA original;assert(decode_cover_rgba(image,original,error));
    const auto* argb=static_cast<const unsigned char*>(rotated.gpu_ptr);
    const auto* source=original.rgba.data()+std::size_t(original.height-1)*original.pitch+(original.width-1)*4;
    assert(argb[0]==source[3] && argb[1]==source[0] && argb[2]==source[1] && argb[3]==source[2]);
    stage.release_cover(rotated);
    assert(renderer.render(state,mesh,1));assert(renderer.last_stats().draw_calls==12 && renderer.last_stats().visible_cases==2);
    const auto first=renderer.covers_[0].texture.gpu_ptr;state.games[0].cover_orientation=3;
    assert(renderer.sync_visible_covers(state,cache,1) && renderer.covers_[0].orientation==3 && renderer.covers_[0].texture.gpu_ptr!=first);
    state.games[1].cover_path="/nonexistent/cover.png";
    assert(renderer.sync_visible_covers(state,cache,1) && renderer.gpu_cover_count()==1 && renderer.has_cover_texture(0) && !renderer.has_cover_texture(1));
    assert(renderer.render(state,mesh,1) && renderer.last_stats().draw_calls==12);
    renderer.shutdown();stage.shutdown();
    puts("FIX28: EXIF JPEG/PNG in both byte orders, all eight pixel orientations, wide full-cover recognition, ICON0 exclusion, per-game L3/rescan, two distinct games, independent GPU copies, orientation re-upload and missing neighbor fallback passed");
}
