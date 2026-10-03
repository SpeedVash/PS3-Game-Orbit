#include "library_scanner.h"
#include "metadata_utils.h"
#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>
#include <cctype>
#include <cstdio>
#ifdef __PSL1GHT__
#include "runtime_diag.h"
#endif

static bool ends_with_ci(const std::string& s, const std::string& ext) {
    if (s.size() < ext.size()) return false;
    for (size_t i = 0; i < ext.size(); ++i) {
        char a = (char)std::tolower((unsigned char)s[s.size()-ext.size()+i]);
        char b = (char)std::tolower((unsigned char)ext[i]);
        if (a != b) return false;
    }
    return true;
}

static std::string stem_title(std::string name) {
    if (ends_with_ci(name, ".iso")) name.resize(name.size()-4);
    for (char& c : name) if (c == '_' || c == '.') c = ' ';
    return name;
}

void LibraryScanner::scan_iso_dir(const std::string& path, GameSource source, std::vector<GameEntry>& out) {
    DIR* d = opendir(path.c_str());
    if (!d) return;
    while (auto* e = readdir(d)) {
        if (e->d_name[0] == '.') continue;
        std::string name = e->d_name;
        if (!ends_with_ci(name, ".iso")) continue;
        const std::string full=path+"/"+name;
        struct stat st{};
        if(stat(full.c_str(),&st)!=0 || !S_ISREG(st.st_mode)) continue;
        GameEntry g;
        g.title = stem_title(name);
        g.title_id = extract_title_id_from_text(name); // Filename hint until ISO SFO reading is added.
        g.path = full;
        g.source = source;
        g.format = GameFormat::ISO;
        out.push_back(std::move(g));
    }
    closedir(d);
}

void LibraryScanner::scan_folder_dir(const std::string& path, GameSource source, std::vector<GameEntry>& out) {
    DIR* d = opendir(path.c_str());
    if (!d) return;
    while (auto* e = readdir(d)) {
        if (e->d_name[0] == '.') continue;
        std::string full = path + "/" + e->d_name;
        struct stat st{};
        if (stat(full.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) continue;
        std::string sfb = full + "/PS3_DISC.SFB";
        std::string ps3game = full + "/PS3_GAME";
        struct stat s1{}, s2{};
        const bool disc=stat(sfb.c_str(),&s1)==0 && S_ISREG(s1.st_mode);
        const bool game=stat(ps3game.c_str(),&s2)==0 && S_ISDIR(s2.st_mode);
        if(!disc && !game) continue;

        GameEntry g;
        g.title = stem_title(e->d_name);
        g.path = full;
        g.source = source;
        g.format = GameFormat::Folder;

        const std::string sfo = full + "/PS3_GAME/PARAM.SFO";
        std::string sfo_title = read_param_sfo_string(sfo, "TITLE");
        std::string sfo_id = read_param_sfo_string(sfo, "TITLE_ID");
        if (!sfo_title.empty()) g.title = sfo_title;
        if (!sfo_id.empty()) g.title_id = sfo_id;
        else g.title_id = extract_title_id_from_text(e->d_name);

        out.push_back(std::move(g));
    }
    closedir(d);
}

std::vector<GameEntry> LibraryScanner::scan_ps3() {
    std::vector<LibraryScanRoot> roots{{"/dev_hdd0",GameSource::HDD}};
    for (int i = 0; i <= 7; ++i) {
        char root[32];
        ::snprintf(root, sizeof(root), "/dev_usb%03d", i);
        roots.push_back({root,GameSource::USB});
    }
    return scan_locations(roots);
}
std::vector<GameEntry> LibraryScanner::scan_locations(const std::vector<LibraryScanRoot>& roots){
    std::vector<GameEntry> out;
    for(const auto& root:roots){
#ifdef __PSL1GHT__
        RuntimeDiag::log("SCAN: begin root=%s",root.path.c_str());
#endif
        scan_iso_dir(root.path+"/PS3ISO",root.source,out);
        scan_folder_dir(root.path+"/GAMES",root.source,out);
        scan_folder_dir(root.path+"/GAMEZ",root.source,out);
#ifdef __PSL1GHT__
        RuntimeDiag::log("SCAN: end root=%s total=%u",root.path.c_str(),unsigned(out.size()));
#endif
    }
    std::sort(out.begin(), out.end(), [](const GameEntry& a, const GameEntry& b){ return a.title < b.title; });
    return out;
}
