#include "raggedy/storage/profile_store.hpp"

#include <unordered_set>
#include <utility>

namespace raggedy::storage
{
    namespace
    {
        core::Profile profile_from_json(const core::JsonValue& value)
        {
            core::Profile profile;
            if (value.type != core::JsonType::object)
            {
                return profile;
            }

            if (const core::JsonValue* field = value.get("id"))
            {
                if (field->type == core::JsonType::string)
                {
                    profile.id = field->string;
                }
            }
            if (const core::JsonValue* field = value.get("name"))
            {
                if (field->type == core::JsonType::string)
                {
                    profile.name = field->string;
                }
            }
            if (const core::JsonValue* field = value.get("pack_id"))
            {
                if (field->type == core::JsonType::string)
                {
                    profile.pack_id = field->string;
                }
            }
            if (const core::JsonValue* field = value.get("target_id"))
            {
                if (field->type == core::JsonType::string)
                {
                    profile.target_id = field->string;
                }
            }
            if (const core::JsonValue* field = value.get("enabled_mod_ids"))
            {
                if (field->type == core::JsonType::array)
                {
                    for (const core::JsonValue& item : field->array)
                    {
                        if (item.type == core::JsonType::string)
                        {
                            profile.enabled_mod_ids.push_back(item.string);
                        }
                    }
                }
            }
            if (const core::JsonValue* field = value.get("telemetry_enabled"))
            {
                if (field->type == core::JsonType::boolean)
                {
                    profile.telemetry_enabled = field->boolean;
                }
            }
            return profile;
        }

        core::JsonValue profile_to_json(const core::Profile& profile)
        {
            std::vector<core::JsonValue> mod_ids;
            mod_ids.reserve(profile.enabled_mod_ids.size());
            for (const std::string& mod_id : profile.enabled_mod_ids)
            {
                mod_ids.push_back(core::JsonValue::from_string(mod_id));
            }

            return core::JsonValue::from_object({
                {"id", core::JsonValue::from_string(profile.id)},
                {"name", core::JsonValue::from_string(profile.name)},
                {"pack_id", core::JsonValue::from_string(profile.pack_id)},
                {"target_id", core::JsonValue::from_string(profile.target_id)},
                {"enabled_mod_ids", core::JsonValue::from_array(std::move(mod_ids))},
                {"telemetry_enabled", core::JsonValue::from_bool(profile.telemetry_enabled)}
            });
        }
    }

    ProfileStore default_profile_store()
    {
        ProfileStore store;
        store.profiles.push_back(core::Profile{"simply-legacy", "Simply Legacy", "simply-legacy", "sl-26-3", {}, false});
        store.profiles.push_back(core::Profile{"re-console", "Re-Console", "re-console", "rc-26-3", {}, false});
        return store;
    }

    bool valid_profile_store(const ProfileStore& store)
    {
        std::unordered_set<std::string> ids;
        for (const core::Profile& profile : store.profiles)
        {
            if (!core::valid_profile(profile) || !ids.insert(profile.id).second)
            {
                return false;
            }
        }

        if (store.active_profile_id.empty())
        {
            return true;
        }
        return find_profile(store, store.active_profile_id) != nullptr;
    }

    ProfileStore profile_store_from_json(const core::JsonValue& value)
    {
        ProfileStore store = default_profile_store();
        if (value.type != core::JsonType::object)
        {
            return store;
        }

        if (const core::JsonValue* field = value.get("profiles"))
        {
            if (field->type == core::JsonType::array)
            {
                store.profiles.clear();
                for (const core::JsonValue& item : field->array)
                {
                    store.profiles.push_back(profile_from_json(item));
                }
            }
        }
        if (const core::JsonValue* field = value.get("active_profile_id"))
        {
            if (field->type == core::JsonType::string)
            {
                store.active_profile_id = field->string;
            }
        }
        return store;
    }

    core::JsonValue profile_store_to_json(const ProfileStore& store)
    {
        std::vector<core::JsonValue> profiles;
        profiles.reserve(store.profiles.size());
        for (const core::Profile& profile : store.profiles)
        {
            profiles.push_back(profile_to_json(profile));
        }

        return core::JsonValue::from_object({
            {"active_profile_id", core::JsonValue::from_string(store.active_profile_id)},
            {"profiles", core::JsonValue::from_array(std::move(profiles))}
        });
    }

    const core::Profile* find_profile(const ProfileStore& store, std::string_view id)
    {
        for (const core::Profile& profile : store.profiles)
        {
            if (std::string_view(profile.id) == id)
            {
                return &profile;
            }
        }
        return nullptr;
    }

    bool set_active_profile(ProfileStore& store, std::string_view id)
    {
        if (id.empty())
        {
            store.active_profile_id.clear();
            return true;
        }
        if (find_profile(store, id) == nullptr)
        {
            return false;
        }
        store.active_profile_id = std::string(id);
        return true;
    }

    bool add_profile(ProfileStore& store, core::Profile profile)
    {
        if (!core::valid_profile(profile) || find_profile(store, profile.id) != nullptr)
        {
            return false;
        }
        store.profiles.push_back(std::move(profile));
        return true;
    }

    bool remove_profile(ProfileStore& store, std::string_view id)
    {
        for (auto it = store.profiles.begin(); it != store.profiles.end(); ++it)
        {
            if (std::string_view(it->id) == id)
            {
                const bool active = std::string_view(store.active_profile_id) == id;
                store.profiles.erase(it);
                if (active)
                {
                    store.active_profile_id.clear();
                }
                return true;
            }
        }
        return false;
    }
}
