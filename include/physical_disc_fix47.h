#pragma once
#ifdef PS3_GAME_ORBIT_FIX47
#include "game_entry.h"
#include <cstdint>
#include <string>
#include <vector>

enum class DiscAccessFix47 { None, Physical, Emulated, Unknown };
struct DiscSampleFix47 {
    bool present=false;
    GameEntry game;
};
using DiscProbeFix47=DiscAccessFix47 (*)();
DiscAccessFix47 probe_disc_access_fix47();
DiscSampleFix47 read_disc_sample_fix47(const std::string& root="/dev_bdvd",
    DiscProbeFix47 probe=probe_disc_access_fix47,const std::string& covers="/dev_hdd0/PS3COVERS");
bool same_disc_sample_fix47(const DiscSampleFix47& a,const DiscSampleFix47& b);
struct DiscCatalogChangeFix47 {bool changed=false;int index=-1;bool preserves_indices=true;};
DiscCatalogChangeFix47 sync_disc_catalog_fix47(std::vector<GameEntry>& games,const DiscSampleFix47& sample);
#endif
