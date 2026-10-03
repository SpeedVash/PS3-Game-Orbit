#pragma once
#include <cstdint>

enum class BootVisualStage {
    None = 0,
    DisplayReady,
    ShaderReady,
    GeometryReady,
    Ready,
    FatalShader,
    FatalGeometry,
    FatalRuntime
};

std::uint32_t boot_visual_color(BootVisualStage stage);
const char* boot_visual_name(BootVisualStage stage);
