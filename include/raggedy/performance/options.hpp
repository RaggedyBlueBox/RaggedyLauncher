#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace raggedy::performance
{
    struct OptionEntry
    {
        std::string key;
        std::string value;
        std::string owner;
    };

    struct PerformancePlan
    {
        std::vector<OptionEntry> entries;
        std::vector<std::string> conflicts;
        bool valid = true;
    };

    struct ManagedFile
    {
        std::string path;
        std::string owner;
    };

    std::string_view feature_owner();
    std::vector<OptionEntry> parse_options(std::string_view text);
    std::string render_options(const std::vector<OptionEntry>& entries);
    PerformancePlan apply_performance_options(const std::vector<OptionEntry>& current,
                                              std::string_view preset,
                                              bool overwrite);
    bool restore_defaults(const std::vector<ManagedFile>& files,
                          std::string_view owner,
                          std::vector<std::string>& paths,
                          std::string& error);
}
