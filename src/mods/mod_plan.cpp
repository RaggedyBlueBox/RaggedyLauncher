#include "raggedy/mods/mod_plan.hpp"

#include <string>
#include <vector>

namespace raggedy::mods
{
    std::string source_name(ModSource source)
    {
        switch (source)
        {
        case ModSource::modrinth:
            return "Modrinth";
        case ModSource::curseforge:
            return "CurseForge";
        }
        return "Unknown";
    }

    ModPlan build_managed_mod_plan(
        const std::string& pack_id,
        const std::string& minecraft_version,
        const std::string& loader,
        const std::vector<ModSelection>& selections)
    {
        ModPlan plan;
        plan.valid = !pack_id.empty() && !minecraft_version.empty() && !loader.empty();
        plan.steps.reserve(selections.size());

        for (const ModSelection& item : selections)
        {
            ModSelection selection = item;
            if (selection.source == ModSource::curseforge)
            {
                selection.requires_user_key = true;
            }

            if (selection.id.empty() || selection.version.empty())
            {
                plan.valid = false;
                continue;
            }

            const std::string action = selection.requires_user_key ? "needs-user-key" : "download";
            plan.steps.push_back(ModPlanStep{action, source_name(selection.source), selection.id, selection.version});
        }

        return plan;
    }
}
