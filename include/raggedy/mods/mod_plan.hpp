#pragma once

#include <string>
#include <vector>

namespace raggedy::mods
{
    enum class ModSource
    {
        modrinth,
        curseforge
    };

    struct ModSelection
    {
        std::string id;
        std::string version;
        ModSource source = ModSource::modrinth;
        bool requires_user_key = false;
    };

    struct ModPlanStep
    {
        std::string action;
        std::string source;
        std::string id;
        std::string version;
    };

    struct ModPlan
    {
        std::vector<ModPlanStep> steps;
        bool valid = false;
    };

    ModPlan build_managed_mod_plan(
        const std::string& pack_id,
        const std::string& minecraft_version,
        const std::string& loader,
        const std::vector<ModSelection>& selections);
    std::string source_name(ModSource source);
}
