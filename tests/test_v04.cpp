#include <cassert>
#include <cstdio>
#include <cmath>
#include "cover_image.h"
#include "image_decode.h"
#include "rsx_stage1.h"
#include "v14_case_mesh.h"
#include "full_cover_layout.h"

static bool near(float a,float b,float e=0.0001f){ return std::fabs(a-b)<e; }

int main(int argc, char** argv) {
    assert(argc >= 2);
    const std::string root=argv[1];
    const auto png=load_cover_file(root+"/tests/assets/full_cover_275x147.png",GameCoverKind::FullCover);
    const auto jpg=load_cover_file(root+"/tests/assets/front_64x96.jpg",GameCoverKind::FrontOnly);
    assert(png.valid() && png.looks_like_canonical_full_cover());
    assert(jpg.valid());

    DecodedImageRGBA rgba; std::string err;
    assert(decode_cover_rgba(png,rgba,err));
    assert(rgba.width==275 && rgba.height==147 && rgba.rgba.size()==275u*147u*4u);
    // Representative pixels from [back|spine|front]. PNG is lossless.
    auto px=[&](int x,int y,int c){ return rgba.rgba[(y*rgba.width+x)*4+c]; };
    assert(px(10,10,0)==220 && px(10,10,1)==30);
    assert(px(135,10,1)==200);
    assert(px(200,10,2)==220);

    DecodedImageRGBA jrgba;
    assert(decode_cover_rgba(jpg,jrgba,err));
    assert(jrgba.width==64 && jrgba.height==96);

    RsxStage1 rsx; assert(rsx.init());
    GpuTextureStage1 tex{};
    assert(rsx.prepare_cover(png,tex));
    assert(tex.uploaded && tex.full_cover && tex.width==275 && tex.height==147);
    assert(tex.pitch % 64 == 0);
    assert(near(tex.back_uv.u1,130.0f/275.0f));
    assert(near(tex.spine_uv.u0,130.0f/275.0f));
    assert(near(tex.spine_uv.u1,145.0f/275.0f));
    assert(near(tex.front_uv.u0,145.0f/275.0f));
    const unsigned char* argb=static_cast<const unsigned char*>(tex.gpu_ptr);
    assert(argb[0]==255 && argb[1]==220 && argb[2]==30 && argb[3]==40);
    rsx.release_cover(tex); rsx.shutdown();

    const auto mesh=build_v14_case_mesh(5);
    assert(mesh.parts.size()==6);
    bool front=false,back=false,spine=false;
    for(const auto& p:mesh.parts){
        if(p.surface==V14Surface::CoverFront){ front=true; assert(near(p.uv.u0,145.0f/275.0f)); }
        if(p.surface==V14Surface::CoverBack){ back=true; assert(near(p.uv.u1,130.0f/275.0f)); }
        if(p.surface==V14Surface::CoverSpine){ spine=true; assert(near(p.uv.u0,130.0f/275.0f)); assert(near(p.uv.u1,145.0f/275.0f)); }
    }
    assert(front&&back&&spine);
    std::puts("V04 decode + RSX upload + V14 mesh tests: OK");
    return 0;
}
