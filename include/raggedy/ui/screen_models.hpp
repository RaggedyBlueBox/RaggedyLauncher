#pragma once

#include "raggedy/catalog/catalog.hpp"
#include "raggedy/core/profile.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace raggedy::ui
{
    struct MainMenuModel
    {
        std::vector<std::wstring> actions;
        std::size_t selected = 0;
        std::wstring status;
    };

    struct VersionScreenModel
    {
        std::vector<std::wstring> packs;
        std::vector<std::wstring> versions;
        std::size_t pack_index = 0;
        std::size_t version_index = 0;
        bool pack_focused = true;
        std::wstring status;
    };

    struct ProfileScreenModel
    {
        std::vector<std::wstring> profile_names;
        std::wstring active_name;
        std::wstring status;
    };

    struct SettingsScreenModel
    {
        std::vector<std::wstring> rows;
        bool telemetry_enabled = false;
        bool remote_files_enabled = false;
        bool performance_overwrite = true;
        std::wstring status;
    };

    struct ModsScreenModel
    {
        std::vector<std::wstring> entries;
        std::wstring status;
    };

    struct ControllerScreenModel
    {
        std::vector<std::wstring> rows;
        std::wstring status;
    };

    struct CreditsScreenModel
    {
        std::vector<std::wstring> lines;
        std::wstring status;
    };

    struct DiagnosticsScreenModel
    {
        std::vector<std::wstring> rows;
        bool telemetry_enabled = false;
        std::wstring status;
    };

    MainMenuModel make_main_menu_model(bool signed_in, bool entitled, bool packs_ready);

    VersionScreenModel make_version_screen_model(
        const std::vector<catalog::Entry>& entries,
        const std::string& selected_target_id,
        std::size_t pack_index,
        std::size_t version_index,
        bool pack_focused);

    ProfileScreenModel make_profile_screen_model(const std::vector<core::Profile>& profiles, const std::string& active_profile_id);

    SettingsScreenModel make_settings_screen_model(bool telemetry_enabled, bool remote_files_enabled, bool performance_overwrite);

    ModsScreenModel make_mods_screen_model(const std::vector<std::string>& mods);

    ControllerScreenModel make_controller_screen_model();

    CreditsScreenModel make_credits_screen_model();

    DiagnosticsScreenModel make_diagnostics_screen_model(bool telemetry_enabled);

    std::wstring screen_status(std::wstring_view message);
}
