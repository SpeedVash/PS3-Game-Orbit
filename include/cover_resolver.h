#pragma once
#include <string>
#include "game_entry.h"

enum class CoverSource {
    None,
    GlobalByTitleId,
    GlobalByGameName,
    BesideIso,
    FolderIcon0
};

struct CoverResult {
    std::string path;
    CoverSource source = CoverSource::None;
    bool is_full_cover = false;
};

class CoverResolver {
public:
    explicit CoverResolver(std::string global_dir = "/dev_hdd0/PS3COVERS");
    CoverResult resolve(const GameEntry& game) const;
    const std::string& global_dir() const { return global_dir_; }
private:
    std::string global_dir_;
    static bool file_exists(const std::string& path);
    static std::string iso_stem(const std::string& path);
};
