#pragma once
#include <cstdint>
#include <cstddef>
#include <functional>
#include <string>
#include <vector>
namespace OrbitIso48 {
using Reader=std::function<bool(std::uint64_t,void*,std::size_t)>;
struct Metadata {std::string title,id;};
Metadata sfo(const std::vector<unsigned char>& bytes);
Metadata metadata(Reader read,std::uint64_t size);
Metadata native_metadata(const std::vector<std::string>& files);
bool native_parts(const std::string& first,std::vector<std::string>& files,std::string& error);
bool iso_name(const std::string& name);
std::string stem(std::string name);
}
