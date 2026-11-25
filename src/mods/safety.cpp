#include "raggedy/mods/safety.hpp"

#include <cctype>

namespace raggedy::mods
{
    std::string clean_text(std::string_view value, std::size_t limit)
    {
        std::string result;
        result.reserve(value.size());
        for (const char raw : value)
        {
            const unsigned char c = static_cast<unsigned char>(raw);
            if (c < 0x20 || c == 0x7F || c == '<' || c == '>')
            {
                result.push_back(' ');
            }
            else
            {
                result.push_back(raw);
            }
        }

        const std::size_t begin = result.find_first_not_of(' ');
        if (begin == std::string::npos)
        {
            return {};
        }
        const std::size_t end = result.find_last_not_of(' ');
        result = result.substr(begin, end - begin + 1);
        if (result.size() > limit)
        {
            result.resize(limit);
        }
        return result;
    }

    std::string lower_ascii(std::string_view value)
    {
        std::string result(value);
        for (char& c : result)
        {
            if (c >= 'A' && c <= 'Z')
            {
                c = static_cast<char>(c - 'A' + 'a');
            }
        }
        return result;
    }

    bool https_url(std::string_view value)
    {
        constexpr std::string_view prefix = "https://";
        if (value.size() <= prefix.size() || value.size() > 2048 || value.substr(0, prefix.size()) != prefix)
        {
            return false;
        }

        for (const char raw : value)
        {
            const unsigned char c = static_cast<unsigned char>(raw);
            if (c < 0x20 || c == 0x7F || c == ' ' || c == '<' || c == '>' || c == '"')
            {
                return false;
            }
        }
        return true;
    }

    bool sha256_hex(std::string_view value)
    {
        if (value.size() != 64)
        {
            return false;
        }

        for (const char c : value)
        {
            const bool hex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
            if (!hex)
            {
                return false;
            }
        }
        return true;
    }
}
