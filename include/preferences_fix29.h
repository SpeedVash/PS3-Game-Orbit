#pragma once
#include <string>
#include <unordered_map>
#include "coverflow_state.h"

class PreferencesFix29 {
public:
    bool load(const std::string& path);
    bool save(const std::string& path) const;
    void apply(std::vector<GameEntry>& games) const;
    void capture(const CoverflowState& state);
    FilterMode filter() const {return filter_;}
    const std::string& selected_path() const {return selected_;}
private:
    struct Record {bool favorite=false;unsigned orientation=1;};
    std::unordered_map<std::string,Record> records_;
    FilterMode filter_=FilterMode::All;
    std::string selected_;
};
