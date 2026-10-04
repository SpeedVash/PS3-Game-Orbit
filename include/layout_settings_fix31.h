#pragma once
#include "coverflow_state.h"
#include <string>

// Separate sidecar: the 1.0 favorites/selection file stays compatible.
class LayoutSettingsFix31 {
public:
    bool load(const std::string& path);
    bool save(const std::string& path) const;
    OrbitLayout layout() const { return layout_; }
    void capture(OrbitLayout layout) { layout_=layout; }
private:
    OrbitLayout layout_=OrbitLayout::Classic;
};
