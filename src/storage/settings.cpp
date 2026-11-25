#include "raggedy/storage/settings.hpp"

#include "raggedy/core/profile.hpp"

#include <cmath>
#include <limits>
#include <utility>

namespace raggedy::storage
{
    namespace
    {
        int read_int(const core::JsonValue& value, int fallback)
        {
            if (value.type != core::JsonType::number || !std::isfinite(value.number))
            {
                return fallback;
            }
            if (value.number < static_cast<double>(std::numeric_limits<int>::min())
                || value.number > static_cast<double>(std::numeric_limits<int>::max()))
            {
                return fallback;
            }
            const int number = static_cast<int>(value.number);
            if (static_cast<double>(number) != value.number)
            {
                return fallback;
            }
            return number;
        }

        bool read_bool(const core::JsonValue& value, bool fallback)
        {
            return value.type == core::JsonType::boolean ? value.boolean : fallback;
        }

        std::string read_string(const core::JsonValue& value, const std::string& fallback)
        {
            return value.type == core::JsonType::string ? value.string : fallback;
        }

        bool valid_client_id(const std::string& value)
        {
            if (value.empty())
            {
                return true;
            }
            if (value.size() > 64)
            {
                return false;
            }
            for (const char c : value)
            {
                const bool hex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
                if (!hex && c != '-')
                {
                    return false;
                }
            }
            return true;
        }

        bool valid_bridge_url(const std::string& value)
        {
            return value.empty()
                || (value.size() <= 200 && value.rfind("http://127.0.0.1:", 0) == 0);
        }
    }

    Settings default_settings()
    {
        return Settings{};
    }

    bool valid_settings(const Settings& settings)
    {
        if (settings.schema_version != 1)
        {
            return false;
        }
        if (settings.ui_fps_cap < 20 || settings.ui_fps_cap > 60)
        {
            return false;
        }
        if (settings.theme.empty() || settings.theme.size() > 64)
        {
            return false;
        }
        if (!valid_client_id(settings.microsoft_client_id))
        {
            return false;
        }
        if (!valid_bridge_url(settings.process_bridge_url))
        {
            return false;
        }
        if (!settings.selected_profile_id.empty() && !core::valid_identifier(settings.selected_profile_id))
        {
            return false;
        }
        return true;
    }

    Settings settings_from_json(const core::JsonValue& value)
    {
        Settings settings = default_settings();
        if (value.type != core::JsonType::object)
        {
            return settings;
        }

        if (const core::JsonValue* field = value.get("schema_version"))
        {
            settings.schema_version = read_int(*field, settings.schema_version);
        }
        if (const core::JsonValue* field = value.get("theme"))
        {
            settings.theme = read_string(*field, settings.theme);
        }
        if (const core::JsonValue* field = value.get("microsoft_client_id"))
        {
            settings.microsoft_client_id = read_string(*field, settings.microsoft_client_id);
        }
        if (const core::JsonValue* field = value.get("process_bridge_url"))
        {
            settings.process_bridge_url = read_string(*field, settings.process_bridge_url);
        }
        if (const core::JsonValue* field = value.get("selected_profile_id"))
        {
            settings.selected_profile_id = read_string(*field, settings.selected_profile_id);
        }
        if (const core::JsonValue* field = value.get("telemetry_enabled"))
        {
            settings.telemetry_enabled = read_bool(*field, settings.telemetry_enabled);
        }
        if (const core::JsonValue* field = value.get("remote_files_enabled"))
        {
            settings.remote_files_enabled = read_bool(*field, settings.remote_files_enabled);
        }
        if (const core::JsonValue* field = value.get("performance_overwrite"))
        {
            settings.performance_overwrite = read_bool(*field, settings.performance_overwrite);
        }
        if (const core::JsonValue* field = value.get("ui_fps_cap"))
        {
            settings.ui_fps_cap = read_int(*field, settings.ui_fps_cap);
        }
        if (const core::JsonValue* field = value.get("controller_prompts"))
        {
            settings.controller_prompts = read_bool(*field, settings.controller_prompts);
        }
        return settings;
    }

    core::JsonValue settings_to_json(const Settings& settings)
    {
        return core::JsonValue::from_object({
            {"schema_version", core::JsonValue::from_number(settings.schema_version)},
            {"theme", core::JsonValue::from_string(settings.theme)},
            {"microsoft_client_id", core::JsonValue::from_string(settings.microsoft_client_id)},
            {"process_bridge_url", core::JsonValue::from_string(settings.process_bridge_url)},
            {"selected_profile_id", core::JsonValue::from_string(settings.selected_profile_id)},
            {"telemetry_enabled", core::JsonValue::from_bool(settings.telemetry_enabled)},
            {"remote_files_enabled", core::JsonValue::from_bool(settings.remote_files_enabled)},
            {"performance_overwrite", core::JsonValue::from_bool(settings.performance_overwrite)},
            {"ui_fps_cap", core::JsonValue::from_number(settings.ui_fps_cap)},
            {"controller_prompts", core::JsonValue::from_bool(settings.controller_prompts)}
        });
    }
}
