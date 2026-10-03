#pragma once
#include <cstdint>
#include <string>

namespace RsxPresentFix20 {
#ifdef __PSL1GHT__
bool acknowledge(void* native_context, std::string& error);
bool warm_up(void* native_context, void* const* buffers,
             const std::uint32_t* offsets, int width, int height,
             std::uint32_t pitch, std::string& error);
bool wait_flip(const char* tag, std::string& error);
void log_display(const char* tag);
void log_buffers(const char* tag, const std::uint32_t* offsets,
                 int width, int height, std::uint32_t pitch);
#endif
}
