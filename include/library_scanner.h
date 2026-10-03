#pragma once
#include <vector>
#include <string>
#include "game_entry.h"

struct LibraryScanRoot { std::string path;GameSource source; };

class LibraryScanner {
public:
    std::vector<GameEntry> scan_ps3();
    std::vector<GameEntry> scan_locations(const std::vector<LibraryScanRoot>& roots);
private:
    void scan_iso_dir(const std::string& path, GameSource source, std::vector<GameEntry>& out);
    void scan_folder_dir(const std::string& path, GameSource source, std::vector<GameEntry>& out);
};
