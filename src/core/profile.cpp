#include "raggedy/core/profile.hpp"

#include <unordered_set>

namespace raggedy::core
{
    bool valid_identifier(std::string_view value)
    {
        if (value.empty() || value.size() > 64)
        {
            return false;
        }

        const char first = value.front();
        if (!((first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z')))
        {
            return false;
        }

        for (const char c : value)
        {
            const bool letter = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
            const bool digit = c >= '0' && c <= '9';
            if (!letter && !digit && c != '_' && c != '-')
            {
                return false;
            }
        }

        return true;
    }

    bool valid_profile(const Profile& profile)
    {
        if (!valid_identifier(profile.id) || !valid_identifier(profile.pack_id) || !valid_identifier(profile.target_id))
        {
            return false;
        }

        if (profile.name.empty() || profile.name.size() > 80)
        {
            return false;
        }

        std::unordered_set<std::string> seen;
        for (const std::string& mod_id : profile.enabled_mod_ids)
        {
            if (!valid_identifier(mod_id) || !seen.insert(mod_id).second)
            {
                return false;
            }
        }

        return true;
    }
}
