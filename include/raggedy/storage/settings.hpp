#pragma once

#include "raggedy/core/json.hpp"

#include <string>

namespace raggedy::storage
{
    struct Settings
    {
        int schema_version = 1;
        std::string theme = "night-sky";
        std::string microsoft_client_id;
        std::string process_bridge_url;
        std::string selected_profile_id;
        bool telemetry_enabled = false;
        bool remote_files_enabled = false;
        bool performance_overwrite = true;
        int ui_fps_cap = 30;
        bool controller_prompts = true;
    };

    Settings default_settings();
    bool valid_settings(const Settings& settings);
    Settings settings_from_json(const core::JsonValue& value);
    core::JsonValue settings_to_json(const Settings& settings);
}
