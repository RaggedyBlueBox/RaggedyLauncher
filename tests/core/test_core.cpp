#include "raggedy/core/launch_plan.hpp"
#include "raggedy/core/target.hpp"

#include <cassert>

int main() {
    using namespace raggedy::core;

    const Pack legacy{"legacy", "Legacy Pack"};
    const Pack console{"console", "Console Pack"};
    const Target first{"t1", "26.3", "Fabric", 25, true, {"legacy"}};
    const Target second{"t2", "1.21.11", "Fabric", 21, false, {"console"}};

    assert(valid_pack(legacy));
    assert(!valid_pack(Pack{}));
    assert(valid_target(first));
    assert(!valid_target(Target{}));

    const auto visible = filter_targets({first, second}, legacy, true);
    assert(visible.size() == 1);
    assert(visible.front().id == "t1");
    assert(target_label(first) == "26.3 + Fabric");

    const Target* found = find_target({first, second}, "t2");
    assert(found != nullptr);
    assert(found->minecraft_version == "1.21.11");
    assert(find_target({first, second}, "missing") == nullptr);

    LaunchPlan plan{"java", {"client.jar"}, {"-Xmx2G", "example.Main"}};
    assert(valid_launch_plan(plan));
    plan.java_executable.clear();
    assert(!valid_launch_plan(plan));

    return 0;
}
