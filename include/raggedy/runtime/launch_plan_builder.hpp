#pragma once

#include "raggedy/core/launch_plan.hpp"

#include <string>
#include <vector>

namespace raggedy::runtime
{
    struct PlanInput
    {
        std::string java_executable;
        std::vector<std::string> class_path;
        std::vector<std::string> jvm_args;
        std::string main_class;
        std::vector<std::string> game_args;
        std::string native_directory;
    };

    bool build_launch_plan(const PlanInput& input, core::LaunchPlan& out, std::string& error);
}
