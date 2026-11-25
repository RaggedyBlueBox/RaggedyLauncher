#include "raggedy/core/paths.hpp"

#include <cctype>

namespace raggedy::core
{
    bool valid_relative_path(std::string_view value)
    {
        if (value.empty() || value.size() > 256 || value.front() == '/' || value.find('\\') != std::string_view::npos
            || value.find(':') != std::string_view::npos)
        {
            return false;
        }

        std::size_t start = 0;
        while (start <= value.size())
        {
            const std::size_t end = value.find('/', start);
            const std::size_t length = end == std::string_view::npos ? value.size() - start : end - start;
            const std::string_view part = value.substr(start, length);
            if (part.empty() || part == "." || part == "..")
            {
                return false;
            }

            for (const char raw : part)
            {
                const unsigned char c = static_cast<unsigned char>(raw);
                const bool allowed = std::isalnum(c) != 0
                    || c == '_' || c == '.' || c == '-';
                if (!allowed)
                {
                    return false;
                }
            }

            if (end == std::string_view::npos)
            {
                break;
            }
            start = end + 1;
        }
        return true;
    }

    bool valid_file_name(std::string_view value)
    {
        if (value.empty() || value.size() > 180 || value == "." || value == "..")
        {
            return false;
        }

        for (const char raw : value)
        {
            const unsigned char c = static_cast<unsigned char>(raw);
            if (c < 0x20 || c == 0x7F || c == '/' || c == '\\' || c == ':' || c == '*' || c == '?'
                || c == '"' || c == '<' || c == '>' || c == '|')
            {
                return false;
            }
        }
        return true;
    }
}
