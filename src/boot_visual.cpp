#include "boot_visual.h"

std::uint32_t boot_visual_color(BootVisualStage stage) {
    // Packed the same way the renderer already supplies rsxSetClearColor: 0xRRGGBBAA.
    switch (stage) {
        case BootVisualStage::DisplayReady:  return 0x38424cff; // steel gray
        case BootVisualStage::ShaderReady:   return 0x735f32ff; // amber/brown
        case BootVisualStage::GeometryReady: return 0x2f6c4cff; // muted green
        case BootVisualStage::Ready:         return 0x324a62ff; // dark slate blue-gray
        case BootVisualStage::FatalShader:   return 0x8a3030ff; // red
        case BootVisualStage::FatalGeometry: return 0x8a572cff; // orange
        case BootVisualStage::FatalRuntime:  return 0x6b2f6bff; // purple
        case BootVisualStage::None:          return 0x101418ff;
    }
    return 0x101418ff;
}

const char* boot_visual_name(BootVisualStage stage) {
    switch (stage) {
        case BootVisualStage::DisplayReady:  return "DISPLAY_READY";
        case BootVisualStage::ShaderReady:   return "SHADER_READY";
        case BootVisualStage::GeometryReady: return "GEOMETRY_READY";
        case BootVisualStage::Ready:         return "READY";
        case BootVisualStage::FatalShader:   return "FATAL_SHADER";
        case BootVisualStage::FatalGeometry: return "FATAL_GEOMETRY";
        case BootVisualStage::FatalRuntime:  return "FATAL_RUNTIME";
        case BootVisualStage::None:          return "NONE";
    }
    return "UNKNOWN";
}
