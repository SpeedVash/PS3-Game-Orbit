#pragma once
#include "game_entry.h"
#include "ntfs_reader_fix48.h"
#include <memory>
namespace OrbitNtfsCatalog48 {
void scan(std::vector<GameEntry>& games);
bool prepare(const GameEntry& game,Orbit48RawMap& map,std::string& error);
void scan_volume(OrbitNtfs48::Volume& volume,unsigned device_index,unsigned partition_index,
    std::uint64_t device_id,std::vector<GameEntry>& games);
}
