#pragma once

#include <string_view>

namespace raggedy::core
{
    bool valid_relative_path(std::string_view value);
    bool valid_file_name(std::string_view value);
}
