#include "raggedy/catalog/catalog.hpp"

#include "raggedy/core/profile.hpp"
#include "raggedy/core/version.hpp"

#include <algorithm>
#include <unordered_set>
#include <utility>

namespace raggedy::catalog
{
    namespace
    {
        bool has_pack(const Entry& entry, std::string_view pack_id)
        {
            return std::any_of(entry.pack_ids.begin(), entry.pack_ids.end(), [pack_id](const std::string& id)
            {
                return id == pack_id;
            });
        }

        bool valid_pack(const Pack& pack)
        {
            return core::valid_identifier(pack.id) && !pack.name.empty() && pack.name.size() <= 80;
        }

        bool valid_entry(const Entry& entry)
        {
            core::Version version;
            return core::valid_identifier(entry.id)
                && core::parse_version(entry.minecraft_version, version)
                && !entry.loader.empty()
                && entry.java_major >= 8
                && !entry.pack_ids.empty();
        }
    }

    void Catalog::add_pack(Pack pack)
    {
        packs_.push_back(std::move(pack));
    }

    void Catalog::add_entry(Entry entry)
    {
        entries_.push_back(std::move(entry));
    }

    bool Catalog::valid() const
    {
        std::unordered_set<std::string> pack_ids;
        for (const Pack& pack : packs_)
        {
            if (!valid_pack(pack) || !pack_ids.insert(pack.id).second)
            {
                return false;
            }
        }

        std::unordered_set<std::string> entry_ids;
        for (const Entry& entry : entries_)
        {
            if (!valid_entry(entry) || !entry_ids.insert(entry.id).second)
            {
                return false;
            }

            for (const std::string& pack_id : entry.pack_ids)
            {
                if (!core::valid_identifier(pack_id) || pack_ids.find(pack_id) == pack_ids.end())
                {
                    return false;
                }
            }
        }

        return true;
    }

    std::vector<Entry> Catalog::entries_for(std::string_view pack_id, bool controller_only) const
    {
        std::vector<Entry> result;
        for (const Entry& entry : entries_)
        {
            if (!valid_entry(entry) || !has_pack(entry, pack_id))
            {
                continue;
            }

            if (controller_only && !entry.controller_ready)
            {
                continue;
            }

            result.push_back(entry);
        }
        return result;
    }

    const Entry* Catalog::find(std::string_view id) const
    {
        for (const Entry& entry : entries_)
        {
            if (entry.id == id)
            {
                return &entry;
            }
        }
        return nullptr;
    }
}
