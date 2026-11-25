#pragma once

#include <string_view>

namespace raggedy::core
{
    struct Version
    {
        int major = 0;
        int minor = 0;
        int patch = 0;
    };

    bool parse_version(std::string_view text, Version& out);
    int compare_version(const Version& lhs, const Version& rhs);
    bool version_at_least(std::string_view version, std::string_view minimum);
    bool version_in_range(std::string_view version, std::string_view minimum, std::string_view maximum);
}
