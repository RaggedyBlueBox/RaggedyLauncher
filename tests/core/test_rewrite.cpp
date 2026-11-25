#include "raggedy/auth/device_code.hpp"
#include "raggedy/catalog/catalog.hpp"
#include "raggedy/core/profile.hpp"
#include "raggedy/core/version.hpp"
#include "raggedy/graphics/translation.hpp"
#include "raggedy/qr/microsoft_link.hpp"
#include "raggedy/ui/device_code_view.hpp"
#include "raggedy/ui/theme.hpp"

#include <cassert>
#include <string>

int main()
{
    raggedy::core::Version version;
    assert(raggedy::core::parse_version("26.3", version));
    assert(version.major == 26 && version.minor == 3 && version.patch == 0);
    assert(raggedy::core::parse_version("1.21.10", version));
    assert(version.patch == 10);
    assert(!raggedy::core::parse_version("1.21.x", version));
    assert(raggedy::core::version_at_least("1.21.10", "1.21"));
    assert(raggedy::core::version_in_range("1.21.8", "1.21", "1.21.10"));
    assert(!raggedy::core::version_in_range("1.20.4", "1.21", "1.21.10"));

    raggedy::core::Profile profile;
    profile.id = "main-pack";
    profile.name = "Main Pack";
    profile.pack_id = "pack-one";
    profile.target_id = "target-263";
    profile.enabled_mod_ids = {"controller-kit", "renderer-fix"};
    assert(raggedy::core::valid_profile(profile));
    profile.enabled_mod_ids.push_back("controller-kit");
    assert(!raggedy::core::valid_profile(profile));

    raggedy::catalog::Catalog catalog;
    catalog.add_pack({"pack-one", "Pack One"});
    catalog.add_pack({"pack-two", "Pack Two"});
    catalog.add_entry({"target-263", "26.3", "Fabric", 25, true, {"pack-one"}, "Ready"});
    catalog.add_entry({"target-1210", "1.21.10", "Fabric", 21, true, {"pack-one", "pack-two"}, "Ready"});
    catalog.add_entry({"target-old", "1.20.1", "Forge", 17, false, {"pack-two"}, "Needs review"});
    assert(catalog.valid());
    assert(catalog.entries_for("pack-one", true).size() == 2);
    assert(catalog.entries_for("pack-two", true).size() == 1);
    assert(catalog.find("target-old") != nullptr);
    assert(catalog.find("missing") == nullptr);

    raggedy::auth::DeviceCodeSession session;
    assert(!raggedy::auth::may_launch(session));
    raggedy::auth::begin_request(session);
    assert(session.stage == raggedy::auth::Stage::requesting);
    raggedy::auth::set_challenge(session, {"abcd1234efgh", "https://example.invalid/link", 600, 5});
    assert(session.stage == raggedy::auth::Stage::waiting);
    assert(raggedy::auth::format_user_code(session.challenge.user_code) == "ABCD-1234-EFGH");
    assert(raggedy::auth::needs_display_code(session));
    raggedy::auth::mark_verifying(session);
    assert(session.stage == raggedy::auth::Stage::verifying);
    raggedy::auth::mark_signed_in(session, false);
    assert(!raggedy::auth::may_download(session));
    raggedy::auth::mark_signed_in(session, true);
    assert(raggedy::auth::may_launch(session));

    const raggedy::ui::DeviceCodeView view = raggedy::ui::make_device_code_view(session);
    assert(!view.show_qr);
    assert(view.status.find(L"ready") != std::wstring::npos);
    raggedy::auth::set_challenge(session, {"abcd1234efgh", "https://example.invalid/link", 600, 5});
    const raggedy::ui::DeviceCodeView waiting = raggedy::ui::make_device_code_view(session);
    assert(waiting.show_qr && waiting.show_copy);
    assert(waiting.formatted_code == L"ABCD-1234-EFGH");

    assert(raggedy::qr::qr_size == 25);
    assert(raggedy::qr::microsoft_link_module(0, 0));
    assert(!raggedy::qr::microsoft_link_module(0, 6));
    assert(raggedy::qr::microsoft_link_module(24, 0));

    assert(raggedy::graphics::supported_translation(
        raggedy::graphics::ApiFamily::opengl,
        raggedy::graphics::ApiFamily::direct3d12));
    assert(!raggedy::graphics::supported_translation(
        raggedy::graphics::ApiFamily::direct3d12,
        raggedy::graphics::ApiFamily::vulkan));
    assert(raggedy::graphics::translation_plan(
        raggedy::graphics::ApiFamily::vulkan,
        raggedy::graphics::ApiFamily::direct3d11).size() == 8);
    assert(raggedy::graphics::family_name(raggedy::graphics::ApiFamily::direct3d12) == "Direct3D 12");

    assert(raggedy::ui::kTheme.corner_radius > 0.0f);
    assert(raggedy::ui::kTheme.accent_mint.g > raggedy::ui::kTheme.accent_mint.r);

    return 0;
}
