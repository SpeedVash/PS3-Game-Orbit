#pragma once
#include <string>

enum class GameSource { HDD, USB };
enum class GameFormat { ISO, Folder };

enum class GameCoverKind { None, FullCover, FrontOnly };

struct GameEntry {
    std::string title;
    std::string title_id;
    std::string path;

    // Resolved after the library scan by CoverResolver.
    std::string cover_path;
    GameCoverKind cover_kind = GameCoverKind::None;
    unsigned cover_orientation = 1; // Per-game manual orientation; 1 = source/EXIF.

    GameSource source = GameSource::HDD;
    GameFormat format = GameFormat::ISO;
    bool favorite = false;
};
