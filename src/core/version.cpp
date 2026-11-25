#include "raggedy/core/version.hpp"

#include <array>
#include <charconv>
#include <system_error>
#include <tuple>

namespace raggedy::core
{
    bool parse_version(std::string_view text, Version& out)
    {
        if (text.empty())
        {
            return false;
        }

        std::array<int, 3> parts{0, 0, 0};
        std::size_t start = 0;

        for (std::size_t i = 0; i < 3; ++i)
        {
            const std::size_t dot = text.find('.', start);
            if (dot == std::string_view::npos && i == 0)
            {
                return false;
            }

            const std::string_view part = dot == std::string_view::npos
                ? text.substr(start)
                : text.substr(start, dot - start);

            if (part.empty())
            {
                return false;
            }

            for (const char c : part)
            {
                if (c < '0' || c > '9')
                {
                    return false;
                }
            }

            int value = 0;
            const auto [end, error] = std::from_chars(part.data(), part.data() + part.size(), value);
            if (error != std::errc{} || end != part.data() + part.size())
            {
                return false;
            }

            parts[i] = value;

            if (dot == std::string_view::npos)
            {
                out = Version{parts[0], parts[1], parts[2]};
                return true;
            }

            start = dot + 1;
        }

        return false;
    }

    int compare_version(const Version& lhs, const Version& rhs)
    {
        const auto left = std::tie(lhs.major, lhs.minor, lhs.patch);
        const auto right = std::tie(rhs.major, rhs.minor, rhs.patch);
        if (left < right)
        {
            return -1;
        }
        if (left > right)
        {
            return 1;
        }
        return 0;
    }

    bool version_at_least(std::string_view version, std::string_view minimum)
    {
        Version left;
        Version right;
        return parse_version(version, left) && parse_version(minimum, right) && compare_version(left, right) >= 0;
    }

    bool version_in_range(std::string_view version, std::string_view minimum, std::string_view maximum)
    {
        Version value;
        Version low;
        Version high;
        return parse_version(version, value)
            && parse_version(minimum, low)
            && parse_version(maximum, high)
            && compare_version(value, low) >= 0
            && compare_version(value, high) <= 0;
    }
}
