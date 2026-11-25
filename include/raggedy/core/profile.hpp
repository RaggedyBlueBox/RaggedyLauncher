#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace raggedy::core
{
    struct Profile
    {
        std::string id;
        std::string name;
        std::string pack_id;
        std::string target_id;
        std::vector<std::string> enabled_mod_ids;
        bool telemetry_enabled = false;
    };

    bool valid_identifier(std::string_view value);
    bool valid_profile(const Profile& profile);
}
