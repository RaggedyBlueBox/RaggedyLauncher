#include "raggedy/catalog/built_in.hpp"

namespace raggedy::catalog
{
    Catalog make_built_in_catalog()
    {
        Catalog catalog;
        catalog.add_pack({"simply-legacy", "Simply Legacy"});
        catalog.add_pack({"re-console", "Re-Console"});

        const std::string simply_legacy = "simply-legacy";
        const std::string re_console = "re-console";

        catalog.add_entry({"sl-26-3", "26.3", "Fabric", 25, true, {simply_legacy},
            "Simply Legacy release line 26.3 with controller-ready Fabric support."});
        catalog.add_entry({"sl-26-1-2", "26.1.2", "Fabric", 25, true, {simply_legacy},
            "Simply Legacy 26.1.2 on the earlier 26.x line."});
        catalog.add_entry({"sl-1-21-10", "1.21.10", "Fabric", 21, true, {simply_legacy},
            "Simply Legacy compatibility target on 1.21.10."});

        catalog.add_entry({"rc-26-3", "26.3", "Fabric", 25, true, {re_console},
            "Re-Console release line 26.3 with controller-ready Fabric support."});
        catalog.add_entry({"rc-26-2", "26.2", "Fabric", 25, true, {re_console},
            "Re-Console target for the 26.2 line."});
        catalog.add_entry({"rc-26-1-2", "26.1.2", "Fabric", 25, true, {re_console},
            "Re-Console target for the 26.1.2 line."});
        catalog.add_entry({"rc-1-21-10", "1.21.10", "Fabric", 21, true, {re_console},
            "Re-Console compatibility target on 1.21.10."});
        catalog.add_entry({"rc-1-21-8", "1.21.8", "Fabric", 21, true, {re_console},
            "Re-Console compatibility target on 1.21.8."});
        catalog.add_entry({"rc-1-21-5", "1.21.5", "Fabric", 21, true, {re_console},
            "Re-Console compatibility target on 1.21.5."});
        catalog.add_entry({"rc-1-21-4", "1.21.4", "Fabric", 21, true, {re_console},
            "Re-Console compatibility target on 1.21.4."});
        catalog.add_entry({"rc-1-21-3", "1.21.3", "Fabric", 21, true, {re_console},
            "Re-Console compatibility target on 1.21.3."});
        catalog.add_entry({"rc-1-21-1", "1.21.1", "Fabric", 21, true, {re_console},
            "Re-Console compatibility target on 1.21.1."});
        catalog.add_entry({"rc-1-21", "1.21", "Fabric", 21, true, {re_console},
            "Re-Console compatibility target on 1.21."});
        catalog.add_entry({"rc-1-20-6", "1.20.6", "Fabric", 17, true, {re_console},
            "Re-Console 1.20.6 line using Java 17."});
        catalog.add_entry({"rc-1-20-4", "1.20.4", "Fabric", 17, true, {re_console},
            "Re-Console 1.20.4 line using Java 17."});
        catalog.add_entry({"rc-1-20-1", "1.20.1", "Fabric", 17, true, {re_console},
            "Re-Console 1.20.1 line using Java 17."});

        return catalog;
    }
}
