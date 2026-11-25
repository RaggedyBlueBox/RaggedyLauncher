#include "raggedy/performance/options.hpp"

#include "raggedy/core/paths.hpp"

#include <algorithm>
#include <cctype>
#include <utility>

namespace raggedy::performance
{
    namespace
    {
        std::string trim(std::string_view value)
        {
            const std::size_t begin = value.find_first_not_of(" \t\r\n");
            if (begin == std::string_view::npos)
            {
                return {};
            }
            const std::size_t end = value.find_last_not_of(" \t\r\n");
            return std::string(value.substr(begin, end - begin + 1));
        }

        std::vector<OptionEntry> desired_options(std::string_view preset)
        {
            const std::string owner{feature_owner()};
            if (preset == "stable")
            {
                return {
                    {"maxFps", "60", owner},
                    {"vsync", "true", owner},
                    {"renderDistance", "8", owner},
                    {"simulationDistance", "6", owner},
                    {"entityDistance", "50%", owner},
                    {"graphicsMode", "fast", owner},
                    {"fancyGraphics", "false", owner},
                    {"ambientOcclusion", "false", owner},
                    {"renderClouds", "fast", owner},
                    {"guiScale", "0", owner}
                };
            }
            if (preset == "balanced")
            {
                return {
                    {"maxFps", "120", owner},
                    {"vsync", "false", owner},
                    {"renderDistance", "12", owner},
                    {"simulationDistance", "8", owner},
                    {"entityDistance", "75%", owner},
                    {"graphicsMode", "fancy", owner},
                    {"fancyGraphics", "true", owner},
                    {"ambientOcclusion", "true", owner},
                    {"renderClouds", "true", owner},
                    {"guiScale", "0", owner}
                };
            }
            return {};
        }
    }

    std::string_view feature_owner()
    {
        return "raggedy-performance";
    }

    std::vector<OptionEntry> parse_options(std::string_view text)
    {
        std::vector<OptionEntry> entries;
        std::size_t line_start = 0;
        while (line_start <= text.size())
        {
            const std::size_t line_end = text.find('\n', line_start);
            const std::size_t length = line_end == std::string_view::npos ? text.size() - line_start : line_end - line_start;
            const std::string line = trim(text.substr(line_start, length));
            if (!line.empty() && line.front() != '#')
            {
                const std::size_t separator = line.find(':') != std::string_view::npos ? line.find(':') : line.find('=');
                if (separator != std::string_view::npos)
                {
                    OptionEntry entry;
                    entry.key = trim(line.substr(0, separator));
                    entry.value = trim(line.substr(separator + 1));
                    if (!entry.key.empty() && entry.key.size() <= 128 && entry.value.size() <= 256)
                    {
                        entries.push_back(std::move(entry));
                    }
                }
            }

            if (line_end == std::string_view::npos)
            {
                break;
            }
            line_start = line_end + 1;
        }
        return entries;
    }

    std::string render_options(const std::vector<OptionEntry>& entries)
    {
        std::string text;
        for (const OptionEntry& entry : entries)
        {
            text += entry.key;
            text.push_back(':');
            text += entry.value;
            text.push_back('\n');
        }
        return text;
    }

    PerformancePlan apply_performance_options(const std::vector<OptionEntry>& current,
                                              std::string_view preset,
                                              bool overwrite)
    {
        PerformancePlan plan;
        plan.entries = current;
        const std::vector<OptionEntry> desired = desired_options(preset);
        if (desired.empty())
        {
            plan.valid = false;
            plan.conflicts.push_back("Unknown performance preset.");
            return plan;
        }

        const std::string owner{feature_owner()};
        for (const OptionEntry& wanted : desired)
        {
            const auto found = std::find_if(plan.entries.begin(), plan.entries.end(), [&wanted](const OptionEntry& entry)
            {
                return entry.key == wanted.key;
            });

            if (found == plan.entries.end())
            {
                plan.entries.push_back(wanted);
                continue;
            }

            if (!found->owner.empty() && found->owner != owner)
            {
                plan.conflicts.push_back(wanted.key + " is managed by " + found->owner + ".");
                continue;
            }

            if (overwrite)
            {
                found->value = wanted.value;
                found->owner = owner;
            }
        }
        plan.valid = plan.conflicts.empty();
        return plan;
    }

    bool restore_defaults(const std::vector<ManagedFile>& files,
                          std::string_view owner,
                          std::vector<std::string>& paths,
                          std::string& error)
    {
        error.clear();
        for (const ManagedFile& file : files)
        {
            if (file.owner != owner)
            {
                continue;
            }
            if (!core::valid_relative_path(file.path))
            {
                error = "Managed file path is unsafe.";
                paths.clear();
                return false;
            }
            paths.push_back(file.path);
        }
        return true;
    }
}
