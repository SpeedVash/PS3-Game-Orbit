#pragma once
#include <string>

// Extracts a common PS3 Title ID pattern (e.g. BLUS30109, BLES01234)
// from arbitrary text such as an ISO filename.
std::string extract_title_id_from_text(const std::string& text);

// Reads a UTF-8/string value from a PARAM.SFO file.
// Returns empty string if file/key is unavailable or unsupported.
std::string read_param_sfo_string(const std::string& sfo_path, const std::string& key);
