#include "raggedy/core/launch_plan.hpp"

namespace raggedy::core {

bool valid_launch_plan(const LaunchPlan& plan) {
    if (plan.java_executable.empty()) return false;
    if (plan.class_path.empty()) return false;
    for (const auto& entry : plan.class_path) {
        if (entry.empty()) return false;
    }
    for (const auto& argument : plan.arguments) {
        if (argument.empty()) return false;
    }
    return true;
}

}
