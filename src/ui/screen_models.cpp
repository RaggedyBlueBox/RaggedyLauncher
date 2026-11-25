#include "raggedy/ui/screen_models.hpp"

#include <algorithm>
#include <cctype>

namespace raggedy::ui
{
    namespace
    {
        std::wstring widen(std::string_view text)
        {
            return std::wstring(text.begin(), text.end());
        }

        std::string lowered(std::string_view text)
        {
            std::string result(text);
            std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c)
            {
                return static_cast<char>(std::tolower(c));
            });
            return result;
        }

        std::wstring pack_label(std::string_view id)
        {
            const std::string value = lowered(id);
            if (value.find("legacy") != std::string::npos)
            {
                return L"Simply Legacy";
            }
            if (value.find("console") != std::string::npos)
            {
                return L"Re-Console";
            }
            return widen(id);
        }

        std::wstring version_label(const catalog::Entry& entry)
        {
            std::wstring label = widen(entry.minecraft_version.empty() ? entry.id : entry.minecraft_version);
            if (!entry.loader.empty())
            {
                label += L" + " + widen(entry.loader);
            }
            return label;
        }

        std::wstring toggle(bool enabled)
        {
            return enabled ? L"On" : L"Off";
        }

        bool has_pack(const catalog::Entry& entry, const std::string& pack_id)
        {
            return std::find(entry.pack_ids.begin(), entry.pack_ids.end(), pack_id) != entry.pack_ids.end();
        }
    }

    MainMenuModel make_main_menu_model(bool signed_in, bool entitled, bool packs_ready)
    {
        MainMenuModel model;
        if (!signed_in)
        {
            model.actions = {L"Sign in", L"Settings", L"Credits"};
            model.status = L"Sign in with Microsoft to continue.";
            return model;
        }

        if (!entitled)
        {
            model.actions = {L"Sign out", L"Settings", L"Credits"};
            model.status = L"Signed in, but Minecraft Java was not found on this account.";
            return model;
        }

        model.actions = {
            L"Play",
            L"Versions",
            L"Profiles",
            L"Mods",
            L"Controller",
            L"Settings",
            L"Diagnostics",
            L"Credits",
            L"Sign out"
        };
        model.status = packs_ready
            ? L"Ready to play. Downloads start when you press Play."
            : L"Loading the version list. Nothing downloads until you press Play.";
        return model;
    }

    VersionScreenModel make_version_screen_model(
        const std::vector<catalog::Entry>& entries,
        const std::string& selected_target_id,
        std::size_t pack_index,
        std::size_t version_index,
        bool pack_focused)
    {
        VersionScreenModel model;
        model.pack_focused = pack_focused;

        std::vector<std::string> pack_ids;
        for (const catalog::Entry& entry : entries)
        {
            for (const std::string& id : entry.pack_ids)
            {
                if (std::find(pack_ids.begin(), pack_ids.end(), id) == pack_ids.end())
                {
                    pack_ids.push_back(id);
                }
            }
        }

        for (const std::string& id : pack_ids)
        {
            model.packs.push_back(pack_label(id));
        }
        model.pack_index = pack_ids.empty() ? 0 : std::min(pack_index, pack_ids.size() - 1);

        std::vector<std::string> visible_ids;
        if (model.pack_index < pack_ids.size())
        {
            for (const catalog::Entry& entry : entries)
            {
                if (!has_pack(entry, pack_ids[model.pack_index]))
                {
                    continue;
                }

                const std::wstring label = version_label(entry);
                if (std::find(model.versions.begin(), model.versions.end(), label) == model.versions.end())
                {
                    model.versions.push_back(label);
                    visible_ids.push_back(entry.id);
                }
            }
        }

        std::size_t selected = version_index;
        if (selected >= visible_ids.size())
        {
            selected = 0;
            for (std::size_t i = 0; i < visible_ids.size(); ++i)
            {
                if (visible_ids[i] == selected_target_id)
                {
                    selected = i;
                    break;
                }
            }
        }
        model.version_index = selected;

        model.status = pack_focused
            ? L"Left and right change pack. Down opens versions. Downloads start when you press Play."
            : L"Up and down change version. A selects. Downloads start when you press Play.";
        if (model.packs.empty())
        {
            model.status = L"No versions available. Nothing downloads until you press Play.";
        }
        return model;
    }

    ProfileScreenModel make_profile_screen_model(const std::vector<core::Profile>& profiles, const std::string& active_profile_id)
    {
        ProfileScreenModel model;
        bool found = false;
        for (const core::Profile& profile : profiles)
        {
            const std::wstring name = widen(profile.name.empty() ? profile.id : profile.name);
            model.profile_names.push_back(name);
            if (!found && profile.id == active_profile_id)
            {
                model.active_name = name;
                found = true;
            }
        }

        if (!found)
        {
            model.active_name = widen(active_profile_id);
        }

        model.status = profiles.empty()
            ? L"No profiles yet. Create one to choose a version."
            : L"Profiles keep mods, worlds, and settings separate.";
        return model;
    }

    SettingsScreenModel make_settings_screen_model(bool telemetry_enabled, bool remote_files_enabled, bool performance_overwrite)
    {
        SettingsScreenModel model;
        model.telemetry_enabled = telemetry_enabled;
        model.remote_files_enabled = remote_files_enabled;
        model.performance_overwrite = performance_overwrite;
        model.rows = {
            L"Telemetry: " + toggle(telemetry_enabled),
            L"Remote file tools: " + toggle(remote_files_enabled),
            L"Game options overwrite: " + toggle(performance_overwrite)
        };
        model.status = telemetry_enabled
            ? L"Telemetry is on. You can turn it off here at any time."
            : L"Telemetry is off by default. Reports stay on this device.";
        return model;
    }

    ModsScreenModel make_mods_screen_model(const std::vector<std::string>& mods)
    {
        ModsScreenModel model;
        for (const std::string& mod : mods)
        {
            model.entries.push_back(widen(mod));
        }
        model.status = mods.empty()
            ? L"No mods in this profile."
            : L"Mods stay in this profile. Changes apply when you confirm.";
        return model;
    }

    ControllerScreenModel make_controller_screen_model()
    {
        ControllerScreenModel model;
        model.rows = {
            L"In-game controller support",
            L"Button layout",
            L"Look sensitivity",
            L"Install or remove component"
        };
        model.status = L"Controller settings stay with this profile.";
        return model;
    }

    CreditsScreenModel make_credits_screen_model()
    {
        CreditsScreenModel model;
        model.lines = {
            L"Raggedy Launcher",
            L"Made for the Legacy Console Edition community.",
            L"Simply Legacy",
            L"Re-Console",
            L"Thanks to the mod, art, and documentation teams.",
            L"4J Studios created the original Legacy Console Edition.",
            L"Independent project. Not affiliated with Microsoft, Xbox, Mojang, or 4J Studios."
        };
        model.status = L"Press B to go back.";
        return model;
    }

    DiagnosticsScreenModel make_diagnostics_screen_model(bool telemetry_enabled)
    {
        DiagnosticsScreenModel model;
        model.telemetry_enabled = telemetry_enabled;
        model.rows = {
            L"Telemetry: " + toggle(telemetry_enabled),
            L"Build details",
            L"Export redacted report",
            L"Clear pending reports"
        };
        model.status = telemetry_enabled
            ? L"Telemetry is on. Reports are redacted before sending."
            : L"Telemetry is off. Reports stay on this device.";
        return model;
    }

    std::wstring screen_status(std::wstring_view message)
    {
        std::wstring result;
        bool pending_space = false;
        for (const wchar_t c : message)
        {
            if (c < L' ' || c == L' ')
            {
                if (!result.empty())
                {
                    pending_space = true;
                }
                continue;
            }
            if (pending_space)
            {
                result.push_back(L' ');
                pending_space = false;
            }
            result.push_back(c);
        }
        return result;
    }
}
