#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace raggedy::mods
{
    std::string clean_text(std::string_view value, std::size_t limit = 256);
    std::string lower_ascii(std::string_view value);
    bool https_url(std::string_view value);
    bool sha256_hex(std::string_view value);
}
