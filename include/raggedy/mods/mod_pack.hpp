#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace raggedy::mods
{
    struct ModPackFile
    {
        std::string path;
        std::string url;
        std::string sha256;
        long long size = 0;
        bool optional = false;
    };

    struct ModPack
    {
        std::string name;
        std::string version_id;
        std::string summary;
        std::vector<ModPackFile> files;
    };

    bool parse_mod_pack(std::string_view text, ModPack& pack, std::string& error);
    std::string write_mod_pack(const ModPack& pack);
    bool valid_mod_pack(const ModPack& pack, std::string& error);
    std::vector<std::string> mod_pack_paths(const ModPack& pack);
    std::vector<ModPackFile> required_mod_pack_files(const ModPack& pack);
}
