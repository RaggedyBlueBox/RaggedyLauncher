#pragma once

#include <string>
#include <vector>

namespace raggedy::core {

struct Pack {
    std::string id;
    std::string name;
};

struct Target {
    std::string id;
    std::string minecraft_version;
    std::string loader;
    int java_major = 0;
    bool controller_ready = false;
    std::vector<std::string> supported_packs;
};

bool valid_pack(const Pack& pack);
bool valid_target(const Target& target);
std::vector<Target> filter_targets(const std::vector<Target>& targets, const Pack& pack, bool controller_only);
const Target* find_target(const std::vector<Target>& targets, const std::string& id);
std::string target_label(const Target& target);

}
