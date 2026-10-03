#include "cover_image.h"
#include <cstdio>
#include <cmath>

static std::uint32_t be32(const std::uint8_t* p) {
    return ((std::uint32_t)p[0] << 24) | ((std::uint32_t)p[1] << 16) |
           ((std::uint32_t)p[2] << 8) | (std::uint32_t)p[3];
}
static std::uint16_t be16(const std::uint8_t* p) {
    return (std::uint16_t)(((std::uint16_t)p[0] << 8) | p[1]);
}

static bool parse_png(const std::vector<std::uint8_t>& b, int& w, int& h) {
    static const std::uint8_t sig[8] = {137,80,78,71,13,10,26,10};
    if (b.size() < 24) return false;
    for (int i = 0; i < 8; ++i) if (b[(size_t)i] != sig[i]) return false;
    if (b[12] != 'I' || b[13] != 'H' || b[14] != 'D' || b[15] != 'R') return false;
    w = (int)be32(&b[16]); h = (int)be32(&b[20]);
    return w > 0 && h > 0;
}

static bool parse_jpeg(const std::vector<std::uint8_t>& b, int& w, int& h) {
    if (b.size() < 4 || b[0] != 0xFF || b[1] != 0xD8) return false;
    size_t p = 2;
    while (p + 4 <= b.size()) {
        while (p < b.size() && b[p] != 0xFF) ++p;
        while (p < b.size() && b[p] == 0xFF) ++p;
        if (p >= b.size()) break;
        const std::uint8_t marker = b[p++];
        if (marker == 0xD9 || marker == 0xDA) break;
        if (p + 2 > b.size()) break;
        const std::uint16_t len = be16(&b[p]);
        if (len < 2 || p + len > b.size()) break;
        const bool sof = (marker >= 0xC0 && marker <= 0xC3) ||
                         (marker >= 0xC5 && marker <= 0xC7) ||
                         (marker >= 0xC9 && marker <= 0xCB) ||
                         (marker >= 0xCD && marker <= 0xCF);
        if (sof && len >= 7) {
            h = (int)be16(&b[p + 3]);
            w = (int)be16(&b[p + 5]);
            return w > 0 && h > 0;
        }
        p += len;
    }
    return false;
}

bool CoverImage::looks_like_canonical_full_cover(float tolerance) const {
    if (kind != GameCoverKind::FullCover || height <= 0) return false;
    const float expected = FullCoverLayout::TotalWidthMm / FullCoverLayout::HeightMm;
    return std::fabs(aspect() - expected) / expected <= tolerance;
}

CoverImage load_cover_file(const std::string& path, GameCoverKind kind) {
    CoverImage img{};
    img.path = path;
    img.kind = kind;
    if (path.empty()) return img;

    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return img;
    std::fseek(f, 0, SEEK_END);
    long n = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    if (n <= 0 || n > 32 * 1024 * 1024) { std::fclose(f); return img; }
    img.encoded.resize((size_t)n);
    if (std::fread(img.encoded.data(), 1, img.encoded.size(), f) != img.encoded.size()) {
        std::fclose(f); img.encoded.clear(); return img;
    }
    std::fclose(f);

    if (parse_png(img.encoded, img.width, img.height)) img.format = CoverFileFormat::PNG;
    else if (parse_jpeg(img.encoded, img.width, img.height)) img.format = CoverFileFormat::JPEG;
    else { img.encoded.clear(); img.width = img.height = 0; }
    return img;
}
