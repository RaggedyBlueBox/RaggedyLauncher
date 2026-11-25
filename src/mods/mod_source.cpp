#include "raggedy/mods/mod_source.hpp"

#include "raggedy/core/paths.hpp"
#include "raggedy/mods/safety.hpp"

#include <algorithm>
#include <string>
#include <string_view>
#include <utility>

namespace raggedy::mods
{
    namespace
    {
        std::string string_field(const core::JsonValue& value, std::string_view key)
        {
            const core::JsonValue* field = value.get(key);
            return field != nullptr && field->type == core::JsonType::string ? field->string : std::string{};
        }

        bool bool_field(const core::JsonValue& value, std::string_view key)
        {
            const core::JsonValue* field = value.get(key);
            return field != nullptr && field->type == core::JsonType::boolean && field->boolean;
        }

        std::wstring widen(std::string_view value)
        {
            return std::wstring(value.begin(), value.end());
        }
    }

    bool valid_result(const ModResult& result, std::string& error)
    {
        error.clear();
        if (result.id.empty() || result.name.empty() || result.version_id.empty()
            || !core::valid_file_name(result.file_name))
        {
            error = "Mod result is missing required fields.";
            return false;
        }
        if (!https_url(result.icon_url) && !result.icon_url.empty())
        {
            error = "Mod result icon URL is invalid.";
            return false;
        }
        if (!result.manual_download && !https_url(result.download_url))
        {
            error = "Mod result download URL is invalid.";
            return false;
        }
        if (!result.manual_download && !sha256_hex(result.sha256))
        {
            error = "Mod result hash is invalid.";
            return false;
        }
        return true;
    }

    std::vector<ModResult> parse_source_results(const core::JsonValue& payload, std::string& error)
    {
        error.clear();
        const core::JsonValue* items = payload.type == core::JsonType::object ? payload.get("hits") : &payload;
        if (items == nullptr || items->type != core::JsonType::array)
        {
            error = "Mod source results must be an array.";
            return {};
        }

        std::vector<ModResult> results;
        results.reserve(std::min<std::size_t>(items->array.size(), 200));
        for (const core::JsonValue& item : items->array)
        {
            if (item.type != core::JsonType::object)
            {
                continue;
            }

            ModResult result;
            result.id = clean_text(string_field(item, "id"), 128);
            result.name = clean_text(string_field(item, "name"), 128);
            result.summary = clean_text(string_field(item, "summary"), 256);
            result.icon_url = string_field(item, "icon_url");
            if (!https_url(result.icon_url))
            {
                result.icon_url.clear();
            }
            result.file_name = clean_text(string_field(item, "file_name"), 180);
            result.version_id = clean_text(string_field(item, "version_id"), 128);
            result.game_version = clean_text(string_field(item, "game_version"), 64);
            result.loader = lower_ascii(clean_text(string_field(item, "loader"), 64));
            result.download_url = string_field(item, "download_url");
            if (!https_url(result.download_url))
            {
                result.download_url.clear();
            }
            result.sha256 = string_field(item, "sha256");
            if (!sha256_hex(result.sha256))
            {
                result.sha256.clear();
            }
            result.source = lower_ascii(string_field(item, "source")) == "curseforge"
                ? ModSource::curseforge
                : ModSource::modrinth;
            result.optional = bool_field(item, "optional");
            result.manual_download = bool_field(item, "manual_download");

            std::string result_error;
            if (!valid_result(result, result_error))
            {
                continue;
            }
            results.push_back(std::move(result));
        }

        if (results.empty() && !items->array.empty())
        {
            error = "No usable mod results were found.";
        }
        return results;
    }

    std::vector<ModResult> filter_results(const std::vector<ModResult>& results,
                                          std::string_view game_version,
                                          std::string_view loader)
    {
        const std::string wanted_version = lower_ascii(game_version);
        const std::string wanted_loader = lower_ascii(loader);
        std::vector<ModResult> filtered;
        filtered.reserve(results.size());

        for (const ModResult& result : results)
        {
            const std::string result_version = lower_ascii(result.game_version);
            const std::string result_loader = lower_ascii(result.loader);
            if (!wanted_version.empty() && result_version != wanted_version && result_version != "any")
            {
                continue;
            }
            if (!wanted_loader.empty() && result_loader != wanted_loader && result_loader != "any")
            {
                continue;
            }
            filtered.push_back(result);
        }
        return filtered;
    }

    std::vector<std::wstring> result_labels(const std::vector<ModResult>& results)
    {
        std::vector<std::wstring> labels;
        labels.reserve(results.size());
        for (const ModResult& result : results)
        {
            std::wstring label = widen(result.name);
            if (!result.version_id.empty())
            {
                label += L" " + widen(result.version_id);
            }
            if (!result.loader.empty())
            {
                label += L" + " + widen(result.loader);
            }
            if (result.manual_download)
            {
                label += L" (manual)";
            }
            if (result.optional)
            {
                label += L" (optional)";
            }
            labels.push_back(std::move(label));
        }
        return labels;
    }

    std::vector<ModSelection> to_selections(const std::vector<ModResult>& results)
    {
        std::vector<ModSelection> selections;
        selections.reserve(results.size());
        for (const ModResult& result : results)
        {
            if (result.id.empty() || result.version_id.empty())
            {
                continue;
            }
            selections.push_back(ModSelection{
                result.id,
                result.version_id,
                result.source,
                result.source == ModSource::curseforge
            });
        }
        return selections;
    }
}
