#include "raggedy/mods/mod_pack.hpp"

#include "raggedy/core/json.hpp"
#include "raggedy/core/paths.hpp"
#include "raggedy/mods/safety.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <set>
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

        long long size_field(const core::JsonValue& value)
        {
            const core::JsonValue* field = value.get("size");
            if (field == nullptr || field->type != core::JsonType::number || !std::isfinite(field->number))
            {
                return 0;
            }
            if (field->number < 0 || field->number > static_cast<double>(std::numeric_limits<long long>::max()))
            {
                return 0;
            }
            return static_cast<long long>(field->number);
        }
    }

    bool valid_mod_pack(const ModPack& pack, std::string& error)
    {
        error.clear();
        if (pack.name.empty() || pack.version_id.empty())
        {
            error = "Mod pack is missing a name or version.";
            return false;
        }

        std::set<std::string> paths;
        for (const ModPackFile& file : pack.files)
        {
            if (!core::valid_relative_path(file.path))
            {
                error = "Mod pack contains an unsafe file path.";
                return false;
            }
            if (!paths.insert(file.path).second)
            {
                error = "Mod pack repeats a file path.";
                return false;
            }
            if (!https_url(file.url))
            {
                error = "Mod pack contains an invalid download URL.";
                return false;
            }
            if (!sha256_hex(file.sha256))
            {
                error = "Mod pack contains an invalid hash.";
                return false;
            }
            if (file.size < 0)
            {
                error = "Mod pack contains an invalid file size.";
                return false;
            }
        }
        return true;
    }

    bool parse_mod_pack(std::string_view text, ModPack& pack, std::string& error)
    {
        core::JsonValue payload;
        if (!core::parse_json(text, payload) || payload.type != core::JsonType::object)
        {
            error = "Mod pack is not a valid JSON object.";
            return false;
        }

        const core::JsonValue* files = payload.get("files");
        if (files == nullptr || files->type != core::JsonType::array)
        {
            error = "Mod pack is missing its file list.";
            return false;
        }

        ModPack parsed;
        parsed.name = clean_text(string_field(payload, "name"), 128);
        parsed.version_id = clean_text(string_field(payload, "version_id"), 128);
        parsed.summary = clean_text(string_field(payload, "summary"), 256);
        parsed.files.reserve(std::min<std::size_t>(files->array.size(), 1000));

        for (const core::JsonValue& item : files->array)
        {
            if (item.type != core::JsonType::object)
            {
                continue;
            }

            ModPackFile file;
            file.path = string_field(item, "path");
            file.url = string_field(item, "url");
            file.sha256 = string_field(item, "sha256");
            file.size = size_field(item);
            file.optional = bool_field(item, "optional");
            parsed.files.push_back(std::move(file));
        }

        if (!valid_mod_pack(parsed, error))
        {
            return false;
        }
        pack = std::move(parsed);
        return true;
    }

    std::string write_mod_pack(const ModPack& pack)
    {
        std::vector<core::JsonValue> files;
        files.reserve(pack.files.size());
        for (const ModPackFile& file : pack.files)
        {
            files.push_back(core::JsonValue::from_object({
                {"path", core::JsonValue::from_string(file.path)},
                {"url", core::JsonValue::from_string(file.url)},
                {"sha256", core::JsonValue::from_string(file.sha256)},
                {"size", core::JsonValue::from_number(static_cast<double>(file.size))},
                {"optional", core::JsonValue::from_bool(file.optional)}
            }));
        }

        const core::JsonValue payload = core::JsonValue::from_object({
            {"name", core::JsonValue::from_string(pack.name)},
            {"version_id", core::JsonValue::from_string(pack.version_id)},
            {"summary", core::JsonValue::from_string(pack.summary)},
            {"files", core::JsonValue::from_array(std::move(files))}
        });
        return core::write_json(payload);
    }

    std::vector<std::string> mod_pack_paths(const ModPack& pack)
    {
        std::vector<std::string> paths;
        paths.reserve(pack.files.size());
        for (const ModPackFile& file : pack.files)
        {
            paths.push_back(file.path);
        }
        return paths;
    }

    std::vector<ModPackFile> required_mod_pack_files(const ModPack& pack)
    {
        std::vector<ModPackFile> required;
        for (const ModPackFile& file : pack.files)
        {
            if (!file.optional)
            {
                required.push_back(file);
            }
        }
        return required;
    }
}
