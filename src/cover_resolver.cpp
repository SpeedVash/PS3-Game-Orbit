#include "cover_resolver.h"
#include <sys/stat.h>

CoverResolver::CoverResolver(std::string global_dir) : global_dir_(std::move(global_dir)) {}

bool CoverResolver::file_exists(const std::string& path) {
    struct stat st{};
    return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

std::string CoverResolver::iso_stem(const std::string& path) {
    size_t slash = path.find_last_of('/');
    std::string name = slash == std::string::npos ? path : path.substr(slash + 1);
    if (name.size() >= 4) {
        std::string ext = name.substr(name.size()-4);
        for (char& c : ext) if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
        if (ext == ".iso") name.resize(name.size()-4);
    }
    return name;
}

CoverResult CoverResolver::resolve(const GameEntry& game) const {
    auto try_full = [&](const std::string& base, CoverSource src) -> CoverResult {
        for (const char* ext : {".jpg", ".jpeg", ".png", ".JPG", ".JPEG", ".PNG"}) {
            std::string p = base + ext;
            if (file_exists(p)) return {p, src, true};
        }
        return {};
    };

    // 1) Canonical cover: Title ID under /dev_hdd0/PS3COVERS.
    if (!game.title_id.empty()) {
        auto r = try_full(global_dir_ + "/" + game.title_id, CoverSource::GlobalByTitleId);
        if (!r.path.empty()) return r;
    }

    // 2) Global full cover using the ISO/folder name.
    std::string stem = iso_stem(game.path);
    if (!stem.empty()) {
        auto r = try_full(global_dir_ + "/" + stem, CoverSource::GlobalByGameName);
        if (!r.path.empty()) return r;
    }

    // 3) ISO-local full cover: Game.iso + Game.jpg/png beside it.
    if (game.format == GameFormat::ISO) {
        size_t slash = game.path.find_last_of('/');
        std::string dir = slash == std::string::npos ? std::string() : game.path.substr(0, slash);
        std::string base = (dir.empty() ? std::string() : dir + "/") + stem;
        auto r = try_full(base, CoverSource::BesideIso);
        if (!r.path.empty()) return r;
    }

    // 4) Folder game fallback. ICON0 is front artwork only, never treated as a full cover.
    if (game.format == GameFormat::Folder) {
        const std::string icon0 = game.path + "/PS3_GAME/ICON0.PNG";
        if (file_exists(icon0)) return {icon0, CoverSource::FolderIcon0, false};
    }

    return {};
}

#ifdef PS3_GAME_ORBIT_FIX33
void CoverResolver::resolve_inspection_art(GameEntry& game) const {
    const auto find=[&](const std::string& suffix) {
        const auto try_base=[&](const std::string& base) {
            for(const char* ext:{".png",".jpg",".jpeg",".PNG",".JPG",".JPEG"}) {
                const auto path=base+ext;if(file_exists(path)) return path;
            }
            return std::string{};
        };
        if(!game.title_id.empty()) {
            auto p=try_base(global_dir_+"/"+game.title_id+suffix);if(!p.empty()) return p;
        }
        const auto stem=iso_stem(game.path);
        if(!stem.empty()) {
            auto p=try_base(global_dir_+"/"+stem+suffix);if(!p.empty()) return p;
            const auto slash=game.path.find_last_of('/');
            const auto directory=game.format==GameFormat::Folder ? game.path :
                slash==std::string::npos ? "" : game.path.substr(0,slash);
            p=try_base((directory.empty() ? "" : directory+"/")+stem+suffix);if(!p.empty()) return p;
        }
        return std::string{};
    };
    game.inside_cover_path=find("_INSIDE");game.disc_art_path=find("_DISC");
}
#endif
