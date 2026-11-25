#pragma once

#include "raggedy/core/json.hpp"
#include "raggedy/mods/mod_plan.hpp"

#include <string>
#include <vector>

namespace raggedy::mods
{
    struct ModResult
    {
        std::string id;
        std::string name;
        std::string summary;
        std::string icon_url;
        std::string file_name;
        std::string version_id;
        std::string game_version;
        std::string loader;
        std::string download_url;
        std::string sha256;
        ModSource source = ModSource::modrinth;
        bool optional = false;
        bool manual_download = false;
    };

    std::vector<ModResult> parse_source_results(const core::JsonValue& payload, std::string& error);
    std::vector<ModResult> filter_results(const std::vector<ModResult>& results,
                                          std::string_view game_version,
                                          std::string_view loader);
    std::vector<std::wstring> result_labels(const std::vector<ModResult>& results);
    std::vector<ModSelection> to_selections(const std::vector<ModResult>& results);
    bool valid_result(const ModResult& result, std::string& error);
}
