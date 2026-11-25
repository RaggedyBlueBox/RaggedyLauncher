#include "raggedy/core/target.hpp"

namespace raggedy::core {
namespace {

bool has_text(const std::string& value) {
    return !value.empty();
}

bool supports(const Target& target, const Pack& pack) {
    for (const auto& id : target.supported_packs) {
        if (id == pack.id) return true;
    }
    return false;
}

}

bool valid_pack(const Pack& pack) {
    return has_text(pack.id) && has_text(pack.name);
}

bool valid_target(const Target& target) {
    return has_text(target.id)
        && has_text(target.minecraft_version)
        && has_text(target.loader)
        && target.java_major >= 8;
}

std::vector<Target> filter_targets(const std::vector<Target>& targets, const Pack& pack, bool controller_only) {
    std::vector<Target> result;
    if (!valid_pack(pack)) return result;
    for (const auto& target : targets) {
        if (!valid_target(target)) continue;
        if (!supports(target, pack)) continue;
        if (controller_only && !target.controller_ready) continue;
        result.push_back(target);
    }
    return result;
}

const Target* find_target(const std::vector<Target>& targets, const std::string& id) {
    for (const auto& target : targets) {
        if (target.id == id) return &target;
    }
    return nullptr;
}

std::string target_label(const Target& target) {
    return target.minecraft_version + " + " + target.loader;
}

}
