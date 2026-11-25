#pragma once

#include <string>
#include <vector>

namespace raggedy::core {

struct LaunchPlan {
    std::string java_executable;
    std::vector<std::string> class_path;
    std::vector<std::string> arguments;
};

bool valid_launch_plan(const LaunchPlan& plan);

}
