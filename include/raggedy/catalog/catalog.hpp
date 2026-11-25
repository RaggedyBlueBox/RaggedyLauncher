#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace raggedy::catalog
{
    struct Pack
    {
        std::string id;
        std::string name;
    };

    struct Entry
    {
        std::string id;
        std::string minecraft_version;
        std::string loader;
        int java_major = 0;
        bool controller_ready = false;
        std::vector<std::string> pack_ids;
        std::string note;
    };

    class Catalog
    {
    public:
        void add_pack(Pack pack);
        void add_entry(Entry entry);
        bool valid() const;
        std::vector<Entry> entries_for(std::string_view pack_id, bool controller_only) const;
        const Entry* find(std::string_view id) const;

    private:
        std::vector<Pack> packs_;
        std::vector<Entry> entries_;
    };
}
