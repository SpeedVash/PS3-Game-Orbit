#include <cassert>
#include <cstdio>
#include <fstream>
#include <vector>
#include "coverflow_state.h"
#include "app_controller.h"
#include "render_plan.h"
#include "metadata_utils.h"
#include "cover_image.h"

static GameEntry game(const char* title, GameSource s) {
    GameEntry g; g.title = title; g.source = s; return g;
}

static void write_png_header(const char* path, int w, int h) {
    std::vector<unsigned char> b(24, 0);
    const unsigned char sig[8] = {137,80,78,71,13,10,26,10};
    for (int i=0;i<8;++i) b[(size_t)i]=sig[i];
    b[12]='I'; b[13]='H'; b[14]='D'; b[15]='R';
    b[16]=(w>>24)&255; b[17]=(w>>16)&255; b[18]=(w>>8)&255; b[19]=w&255;
    b[20]=(h>>24)&255; b[21]=(h>>16)&255; b[22]=(h>>8)&255; b[23]=h&255;
    std::ofstream(path, std::ios::binary).write((const char*)b.data(), (std::streamsize)b.size());
}

static void write_jpeg_header(const char* path, int w, int h) {
    // Minimal marker stream sufficient for our dimension parser: SOI + SOF0.
    unsigned char b[] = {
        0xFF,0xD8,
        0xFF,0xC0, 0x00,0x11, 0x08,
        (unsigned char)((h>>8)&255),(unsigned char)(h&255),
        (unsigned char)((w>>8)&255),(unsigned char)(w&255),
        0x03,0x01,0x11,0x00,0x02,0x11,0x00,0x03,0x11,0x00,
        0xFF,0xD9
    };
    std::ofstream(path, std::ios::binary).write((const char*)b, sizeof(b));
}

int main() {
    CoverflowState s;
    s.games = {game("A",GameSource::HDD),game("B",GameSource::USB),game("C",GameSource::HDD)};
    rebuild_visible(s);
    assert(s.visible.size() == 3);

    move_selection(s, 1);
    assert(current_game(s) && current_game(s)->title == "B");

    AppController app;
    InputFrame in{}; in.connected = true; in.square.pressed = true;
    app.update(s, in, 1.0f/60.0f);
    assert(s.inspect_mode);

    in = {}; in.connected = true; in.right_x = 1.0f;
    app.update(s, in, 1.0f);
    assert(s.center_yaw_deg > 100.0f);

    in = {}; in.connected = true; in.square.pressed = true;
    app.update(s, in, 1.0f/60.0f);
    assert(!s.inspect_mode);

    in = {}; in.connected = true; in.r1.pressed = true;
    app.update(s, in, 1.0f/60.0f);
    assert(s.filter == FilterMode::HDD && s.visible.size() == 2);

    auto plan = build_coverflow_render_plan(s, 2);
    assert(!plan.empty());
    bool center = false;
    for (auto& p : plan) if (p.selected) center = true;
    assert(center);

    assert(extract_title_id_from_text("Race Pro [BLES00680].iso") == "BLES00680");

    const char* png = "/tmp/v03_full_cover.png";
    const char* jpg = "/tmp/v03_front.jpg";
    write_png_header(png, 2750, 1470);
    write_jpeg_header(jpg, 1000, 1400);
    auto full = load_cover_file(png, GameCoverKind::FullCover);
    auto front = load_cover_file(jpg, GameCoverKind::FrontOnly);
    assert(full.valid() && full.format == CoverFileFormat::PNG && full.width == 2750 && full.height == 1470);
    assert(full.looks_like_canonical_full_cover());
    assert(front.valid() && front.format == CoverFileFormat::JPEG && front.width == 1000 && front.height == 1400);
    assert(!front.looks_like_canonical_full_cover());
    std::remove(png); std::remove(jpg);

    std::puts("V04 inherited runtime + cover tests: OK");
    return 0;
}
