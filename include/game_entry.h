#pragma once
#include <string>
#ifdef PS3_GAME_ORBIT_FIX41
#include "case_style_fix41.h"
#endif

enum class GameSource { HDD, USB, Disc };
enum class GameFormat { ISO, Folder, Disc };

enum class GameCoverKind { None, FullCover, FrontOnly };

struct GameEntry {
    std::string title;
    std::string title_id;
    std::string path;

    // Resolved after the library scan by CoverResolver.
    std::string cover_path;
    GameCoverKind cover_kind = GameCoverKind::None;
    unsigned cover_orientation = 1; // Per-game manual orientation; 1 = source/EXIF.
#ifdef PS3_GAME_ORBIT_FIX33
    std::string inside_cover_path;
    std::string disc_art_path;
#endif

    GameSource source = GameSource::HDD;
    GameFormat format = GameFormat::ISO;
    bool favorite = false;
    // Physical discs share /dev_bdvd but retain preferences by their title ID.
    std::string library_key{};
    bool physical_disc = false;
    const std::string& identity_path() const { return library_key.empty() ? path : library_key; }
#ifdef PS3_GAME_ORBIT_FIX41
    GameCaseStyleFix41 case_style=GameCaseStyleFix41::Clear;
#endif
};
