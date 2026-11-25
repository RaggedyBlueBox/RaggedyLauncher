#include "raggedy/runtime/launch_plan_builder.hpp"

#include <utility>

namespace raggedy::runtime
{
    namespace
    {
        bool has_empty_entry(const std::vector<std::string>& values)
        {
            for (const std::string& value : values)
            {
                if (value.empty())
                {
                    return true;
                }
            }
            return false;
        }

        std::string join_class_path(const std::vector<std::string>& entries)
        {
#ifdef _WIN32
            constexpr char separator = ';';
#else
            constexpr char separator = ':';
#endif
            std::string joined;
            for (const std::string& entry : entries)
            {
                if (!joined.empty())
                {
                    joined.push_back(separator);
                }
                joined += entry;
            }
            return joined;
        }
    }

    bool build_launch_plan(const PlanInput& input, core::LaunchPlan& out, std::string& error)
    {
        out = core::LaunchPlan{};
        error.clear();

        if (input.java_executable.empty())
        {
            error = "Java executable is empty";
            return false;
        }
        if (input.class_path.empty())
        {
            error = "Class path is empty";
            return false;
        }
        if (has_empty_entry(input.class_path))
        {
            error = "Class path contains an empty entry";
            return false;
        }
        if (input.main_class.empty())
        {
            error = "Main class is empty";
            return false;
        }
        if (has_empty_entry(input.jvm_args) || has_empty_entry(input.game_args))
        {
            error = "Arguments contain an empty entry";
            return false;
        }

        core::LaunchPlan plan;
        plan.java_executable = input.java_executable;
        plan.class_path = input.class_path;
        plan.arguments.push_back(plan.java_executable);
        plan.arguments.insert(plan.arguments.end(), input.jvm_args.begin(), input.jvm_args.end());
        if (!input.native_directory.empty())
        {
            plan.arguments.push_back("-Djava.library.path=" + input.native_directory);
        }
        plan.arguments.push_back("-cp");
        plan.arguments.push_back(join_class_path(input.class_path));
        plan.arguments.push_back(input.main_class);
        plan.arguments.insert(plan.arguments.end(), input.game_args.begin(), input.game_args.end());

        if (!core::valid_launch_plan(plan))
        {
            error = "Launch plan is invalid";
            return false;
        }

        out = std::move(plan);
        return true;
    }
}
