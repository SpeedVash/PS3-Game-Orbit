#include "image_decode.h"
#include <cstdlib>
#include <cstring>
#include <setjmp.h>

#ifdef __PSL1GHT__
#include <sysmodule/sysmodule.h>
#include <pngdec/pngdec.h>
#include <jpgdec/jpgdec.h>
#include "runtime_diag.h"
#else
#include <png.h>
extern "C" {
#include <jpeglib.h>
}
#endif

#ifdef __PSL1GHT__
static void argb_bytes_to_rgba(const std::uint8_t* src, int src_pitch,
                               int width, int height, DecodedImageRGBA& out) {
    out.width = width;
    out.height = height;
    out.pitch = width * 4;
    out.rgba.resize(static_cast<std::size_t>(out.pitch) * static_cast<std::size_t>(height));
    for (int y = 0; y < height; ++y) {
        const std::uint8_t* s = src + static_cast<std::size_t>(y) * src_pitch;
        std::uint8_t* d = out.rgba.data() + static_cast<std::size_t>(y) * out.pitch;
        for (int x = 0; x < width; ++x) {
            const std::uint8_t a = s[x * 4 + 0];
            const std::uint8_t r = s[x * 4 + 1];
            const std::uint8_t g = s[x * 4 + 2];
            const std::uint8_t b = s[x * 4 + 3];
            d[x * 4 + 0] = r;
            d[x * 4 + 1] = g;
            d[x * 4 + 2] = b;
            d[x * 4 + 3] = a;
        }
    }
}

#endif

#ifdef __PSL1GHT__
static bool decode_png_ps3(const CoverImage& src, DecodedImageRGBA& out, std::string& error) {
#ifdef PS3_SP_LOADER_FIX28
    RuntimeDiag::log("PNG 00: sysModuleLoad begin");
#endif
    const s32 mod = sysModuleLoad(SYSMODULE_PNGDEC);
#ifdef PS3_SP_LOADER_FIX28
    RuntimeDiag::log("PNG 01: sysModuleLoad returned; rc=%d",int(mod));
#endif
    if (mod != 0) { error = "sysModuleLoad(SYSMODULE_PNGDEC) failed"; return false; }

    pngData data{};
#ifdef PS3_SP_LOADER_FIX28
    RuntimeDiag::log("PNG 02: pngLoadFromBuffer begin; bytes=%u",unsigned(src.encoded.size()));
#endif
    const s32 ret = pngLoadFromBuffer(src.encoded.data(), static_cast<u32>(src.encoded.size()), &data);
#ifdef PS3_SP_LOADER_FIX28
    RuntimeDiag::log("PNG 03: pngLoadFromBuffer returned; rc=%d dimensions=%ux%u pitch=%u",int(ret),unsigned(data.width),unsigned(data.height),unsigned(data.pitch));
#endif
    if (ret != PNGDEC_ERROR_OK || !data.bmp_out || data.width == 0 || data.height == 0) {
        error = "pngLoadFromBuffer failed";
        if (data.bmp_out) std::free(data.bmp_out);
        sysModuleUnload(SYSMODULE_PNGDEC);
        return false;
    }

    // PSL1GHT pngdec helper returns ARGB8.
    argb_bytes_to_rgba(static_cast<const std::uint8_t*>(data.bmp_out),
                       static_cast<int>(data.pitch), static_cast<int>(data.width),
                       static_cast<int>(data.height), out);
    std::free(data.bmp_out);
    sysModuleUnload(SYSMODULE_PNGDEC);
    return out.valid();
}

static bool decode_jpeg_ps3(const CoverImage& src, DecodedImageRGBA& out, std::string& error) {
    const s32 mod = sysModuleLoad(SYSMODULE_JPGDEC);
    if (mod != 0) { error = "sysModuleLoad(SYSMODULE_JPGDEC) failed"; return false; }

    jpgData data{};
    const s32 ret = jpgLoadFromBuffer(src.encoded.data(), static_cast<u32>(src.encoded.size()), &data);
    if (ret != JPGDEC_ERROR_OK || !data.bmp_out || data.width == 0 || data.height == 0) {
        error = "jpgLoadFromBuffer failed";
        if (data.bmp_out) std::free(data.bmp_out);
        sysModuleUnload(SYSMODULE_JPGDEC);
        return false;
    }

    // PSL1GHT jpgdec helper returns ARGB8.
    argb_bytes_to_rgba(static_cast<const std::uint8_t*>(data.bmp_out),
                       static_cast<int>(data.pitch), static_cast<int>(data.width),
                       static_cast<int>(data.height), out);
    std::free(data.bmp_out);
    sysModuleUnload(SYSMODULE_JPGDEC);
    return out.valid();
}
#else
static bool decode_png_host(const CoverImage& src, DecodedImageRGBA& out, std::string& error) {
    png_image image{};
    image.version = PNG_IMAGE_VERSION;
    if (!png_image_begin_read_from_memory(&image, src.encoded.data(), src.encoded.size())) {
        error = "png_image_begin_read_from_memory failed";
        return false;
    }
    image.format = PNG_FORMAT_RGBA;
    out.width = static_cast<int>(image.width);
    out.height = static_cast<int>(image.height);
    out.pitch = out.width * 4;
    out.rgba.resize(PNG_IMAGE_SIZE(image));
    if (!png_image_finish_read(&image, nullptr, out.rgba.data(), out.pitch, nullptr)) {
        error = image.message[0] ? image.message : "png_image_finish_read failed";
        png_image_free(&image);
        out = {};
        return false;
    }
    png_image_free(&image);
    return out.valid();
}

struct JpegErrorState {
    jpeg_error_mgr pub;
    jmp_buf jump;
};
static void jpeg_error_exit(j_common_ptr cinfo) {
    auto* state = reinterpret_cast<JpegErrorState*>(cinfo->err);
    longjmp(state->jump, 1);
}

static bool decode_jpeg_host(const CoverImage& src, DecodedImageRGBA& out, std::string& error) {
    jpeg_decompress_struct cinfo{};
    JpegErrorState jerr{};
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = jpeg_error_exit;
    if (setjmp(jerr.jump)) {
        jpeg_destroy_decompress(&cinfo);
        error = "libjpeg decode failed";
        out = {};
        return false;
    }
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, src.encoded.data(), static_cast<unsigned long>(src.encoded.size()));
    jpeg_read_header(&cinfo, TRUE);
    cinfo.out_color_space = JCS_RGB;
    jpeg_start_decompress(&cinfo);

    out.width = static_cast<int>(cinfo.output_width);
    out.height = static_cast<int>(cinfo.output_height);
    out.pitch = out.width * 4;
    out.rgba.resize(static_cast<std::size_t>(out.pitch) * out.height);
    std::vector<std::uint8_t> row(static_cast<std::size_t>(out.width) * 3u);

    while (cinfo.output_scanline < cinfo.output_height) {
        JSAMPROW row_ptr = row.data();
        jpeg_read_scanlines(&cinfo, &row_ptr, 1);
        const int y = static_cast<int>(cinfo.output_scanline) - 1;
        std::uint8_t* d = out.rgba.data() + static_cast<std::size_t>(y) * out.pitch;
        for (int x = 0; x < out.width; ++x) {
            d[x*4+0] = row[x*3+0];
            d[x*4+1] = row[x*3+1];
            d[x*4+2] = row[x*3+2];
            d[x*4+3] = 255;
        }
    }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    return out.valid();
}
#endif

bool decode_cover_rgba(const CoverImage& src, DecodedImageRGBA& out, std::string& error) {
    out = {};
    error.clear();
    if (!src.valid()) { error = "invalid CoverImage"; return false; }
#ifdef __PSL1GHT__
    if (src.format == CoverFileFormat::PNG) return decode_png_ps3(src, out, error);
    if (src.format == CoverFileFormat::JPEG) return decode_jpeg_ps3(src, out, error);
#else
    if (src.format == CoverFileFormat::PNG) return decode_png_host(src, out, error);
    if (src.format == CoverFileFormat::JPEG) return decode_jpeg_host(src, out, error);
#endif
    error = "unsupported cover format";
    return false;
}
