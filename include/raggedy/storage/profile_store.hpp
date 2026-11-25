#pragma once

#include "raggedy/core/json.hpp"
#include "raggedy/core/profile.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace raggedy::storage
{
    struct ProfileStore
    {
        std::vector<core::Profile> profiles;
        std::string active_profile_id;
    };

    ProfileStore default_profile_store();
    bool valid_profile_store(const ProfileStore& store);
    ProfileStore profile_store_from_json(const core::JsonValue& value);
    core::JsonValue profile_store_to_json(const ProfileStore& store);
    const core::Profile* find_profile(const ProfileStore& store, std::string_view id);
    bool set_active_profile(ProfileStore& store, std::string_view id);
    bool add_profile(ProfileStore& store, core::Profile profile);
    bool remove_profile(ProfileStore& store, std::string_view id);
}
