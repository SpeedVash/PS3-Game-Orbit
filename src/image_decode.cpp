#include "image_decode.h"
#include <cstdlib>
#include <cstring>
#include <setjmp.h>
#include "performance_fix35.h"

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

#if defined(__PSL1GHT__) && defined(PS3_GAME_ORBIT_FIX35)
namespace {
bool png_module_loaded=false,jpg_module_loaded=false;
s32 ensure_image_module(sysModuleId module,bool& loaded){
    if(loaded)return 0;
    const s32 rc=sysModuleLoad(module);if(rc==0)loaded=true;return rc;
}
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
    #ifdef PS3_GAME_ORBIT_FIX35
    const s32 mod = ensure_image_module(SYSMODULE_PNGDEC,png_module_loaded);
#else
    const s32 mod = sysModuleLoad(SYSMODULE_PNGDEC);
#endif
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
#ifndef PS3_GAME_ORBIT_FIX35
        sysModuleUnload(SYSMODULE_PNGDEC);
#endif
        return false;
    }

    // PSL1GHT pngdec helper returns ARGB8.
    argb_bytes_to_rgba(static_cast<const std::uint8_t*>(data.bmp_out),
                       static_cast<int>(data.pitch), static_cast<int>(data.width),
                       static_cast<int>(data.height), out);
    std::free(data.bmp_out);
#ifndef PS3_GAME_ORBIT_FIX35
    sysModuleUnload(SYSMODULE_PNGDEC);
#endif
    return out.valid();
}

static bool decode_jpeg_ps3(const CoverImage& src, DecodedImageRGBA& out, std::string& error) {
    #ifdef PS3_GAME_ORBIT_FIX35
    const s32 mod = ensure_image_module(SYSMODULE_JPGDEC,jpg_module_loaded);
#else
    const s32 mod = sysModuleLoad(SYSMODULE_JPGDEC);
#endif
    if (mod != 0) { error = "sysModuleLoad(SYSMODULE_JPGDEC) failed"; return false; }

    jpgData data{};
    const s32 ret = jpgLoadFromBuffer(src.encoded.data(), static_cast<u32>(src.encoded.size()), &data);
    if (ret != JPGDEC_ERROR_OK || !data.bmp_out || data.width == 0 || data.height == 0) {
        error = "jpgLoadFromBuffer failed";
        if (data.bmp_out) std::free(data.bmp_out);
#ifndef PS3_GAME_ORBIT_FIX35
        sysModuleUnload(SYSMODULE_JPGDEC);
#endif
        return false;
    }

    // PSL1GHT jpgdec helper returns ARGB8.
    argb_bytes_to_rgba(static_cast<const std::uint8_t*>(data.bmp_out),
                       static_cast<int>(data.pitch), static_cast<int>(data.width),
                       static_cast<int>(data.height), out);
    std::free(data.bmp_out);
#ifndef PS3_GAME_ORBIT_FIX35
    sysModuleUnload(SYSMODULE_JPGDEC);
#endif
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
#ifdef PS3_GAME_ORBIT_FIX35
    OrbitPerformanceFix35::Scope timer(OrbitPerformanceFix35::Kind::Decode);
#endif
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

#ifdef PS3_GAME_ORBIT_FIX35
bool init_image_decoders_fix35(){
#ifdef __PSL1GHT__
    const bool png=ensure_image_module(SYSMODULE_PNGDEC,png_module_loaded)==0;
    const bool jpg=ensure_image_module(SYSMODULE_JPGDEC,jpg_module_loaded)==0;
    return png && jpg;
#else
    return true;
#endif
}
void shutdown_image_decoders_fix35(){
#ifdef __PSL1GHT__
    if(png_module_loaded){sysModuleUnload(SYSMODULE_PNGDEC);png_module_loaded=false;}
    if(jpg_module_loaded){sysModuleUnload(SYSMODULE_JPGDEC);jpg_module_loaded=false;}
#endif
}
bool decode_cover_argb_fix35(const CoverImage& src,DecodedImageARGB& out,std::string& error){
    out={};error.clear();
#ifdef __PSL1GHT__
    OrbitPerformanceFix35::Scope timer(OrbitPerformanceFix35::Kind::Decode);
    if(!src.valid()){error="Invalid image";return false;}
    void* bitmap=nullptr;u32 width=0,height=0,pitch=0;
    if(src.format==CoverFileFormat::PNG){
        if(ensure_image_module(SYSMODULE_PNGDEC,png_module_loaded)!=0){error="PNG module unavailable";return false;}
        pngData data{};const auto rc=pngLoadFromBuffer(src.encoded.data(),u32(src.encoded.size()),&data);
        bitmap=data.bmp_out;width=data.width;height=data.height;pitch=data.pitch;
        if(rc!=PNGDEC_ERROR_OK){if(bitmap)std::free(bitmap);error="PNG decode failed";return false;}
    }else if(src.format==CoverFileFormat::JPEG){
        if(ensure_image_module(SYSMODULE_JPGDEC,jpg_module_loaded)!=0){error="JPEG module unavailable";return false;}
        jpgData data{};const auto rc=jpgLoadFromBuffer(src.encoded.data(),u32(src.encoded.size()),&data);
        bitmap=data.bmp_out;width=data.width;height=data.height;pitch=data.pitch;
        if(rc!=JPGDEC_ERROR_OK){if(bitmap)std::free(bitmap);error="JPEG decode failed";return false;}
    }else {error="Unsupported image";return false;}
    if(!bitmap || width==0 || height==0 || width>8192 || height>8192 || pitch<width*4 || std::size_t(width)*height>16000000){
        if(bitmap)std::free(bitmap);
        error="Invalid decoded dimensions";return false;
    }
    out.width=int(width);out.height=int(height);out.pitch=int(pitch);
    const auto* first=static_cast<const std::uint8_t*>(bitmap);
    out.argb.assign(first,first+std::size_t(pitch)*height);std::free(bitmap);return out.valid();
#else
    DecodedImageRGBA rgba;if(!decode_cover_rgba(src,rgba,error))return false;
    out.width=rgba.width;out.height=rgba.height;out.pitch=rgba.width*4;out.argb.resize(std::size_t(out.pitch)*out.height);
    for(int y=0;y<out.height;++y)for(int x=0;x<out.width;++x){
        const auto* a=rgba.rgba.data()+std::size_t(y)*rgba.pitch+x*4;
        auto* b=out.argb.data()+std::size_t(y)*out.pitch+x*4;b[0]=a[3];b[1]=a[0];b[2]=a[1];b[3]=a[2];
    }
    return out.valid();
#endif
}
#endif
