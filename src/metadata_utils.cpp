#include "metadata_utils.h"
#include <cstdio>
#include <cstdint>
#include <vector>
#include <cctype>
#include <algorithm>

static uint16_t rd16(const uint8_t* p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}
static uint32_t rd32(const uint8_t* p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

std::string extract_title_id_from_text(const std::string& text) {
    // Most retail PS3 IDs used here are 4 ASCII letters + 5 digits.
    for (size_t i = 0; i + 9 <= text.size(); ++i) {
        bool letters = true, digits = true;
        for (size_t j = 0; j < 4; ++j)
            letters &= std::isalpha((unsigned char)text[i+j]) != 0;
        for (size_t j = 4; j < 9; ++j)
            digits &= std::isdigit((unsigned char)text[i+j]) != 0;
        if (!letters || !digits) continue;
        std::string id = text.substr(i, 9);
        std::transform(id.begin(), id.end(), id.begin(), [](unsigned char c){ return (char)std::toupper(c); });
        return id;
    }
    return {};
}

std::string read_param_sfo_string(const std::string& sfo_path, const std::string& wanted_key) {
    FILE* f = std::fopen(sfo_path.c_str(), "rb");
    if (!f) return {};
    std::fseek(f, 0, SEEK_END);
    long n = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    if (n < 20 || n > 4 * 1024 * 1024) { std::fclose(f); return {}; }

    std::vector<uint8_t> data((size_t)n);
    if (std::fread(data.data(), 1, data.size(), f) != data.size()) { std::fclose(f); return {}; }
    std::fclose(f);

    // PSF magic is 00 50 53 46.
    if (data[0] != 0x00 || data[1] != 0x50 || data[2] != 0x53 || data[3] != 0x46) return {};
    const uint32_t key_table = rd32(&data[8]);
    const uint32_t data_table = rd32(&data[12]);
    const uint32_t count = rd32(&data[16]);
    if (key_table >= data.size() || data_table >= data.size() || count > 4096) return {};
    if (20ull + (uint64_t)count * 16ull > data.size()) return {};

    for (uint32_t i = 0; i < count; ++i) {
        const uint8_t* e = &data[20 + i * 16];
        const uint16_t key_off = rd16(e + 0);
        const uint16_t fmt = rd16(e + 2);
        const uint32_t len = rd32(e + 4);
        const uint32_t value_off = rd32(e + 12);
        const size_t kp = (size_t)key_table + key_off;
        const size_t vp = (size_t)data_table + value_off;
        if (kp >= data.size() || vp >= data.size()) continue;

        size_t ke = kp;
        while (ke < data.size() && data[ke] != 0) ++ke;
        if (ke == data.size()) continue;
        std::string key((const char*)&data[kp], ke-kp);
        if (key != wanted_key) continue;

        // String-ish SFO formats are 0x0204 / 0x0004 variants in common PARAM.SFOs.
        // For robustness, return a NUL-terminated byte sequence for non-integer fields.
        if ((fmt & 0xFF) == 0x04 || fmt == 0x0204 || fmt == 0x0004) {
            size_t maxlen = std::min<size_t>(len ? len : 1024, data.size() - vp);
            size_t ve = vp;
            while (ve < vp + maxlen && data[ve] != 0) ++ve;
            return std::string((const char*)&data[vp], ve-vp);
        }
        return {};
    }
    return {};
}
