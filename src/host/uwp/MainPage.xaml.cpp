#include "pch.h"
#include "App.xaml.h"
#include "MainPage.xaml.h"

#if __has_include("Generated Files/MainPage.g.cpp")
#include "Generated Files/MainPage.g.cpp"
#elif __has_include("MainPage.g.cpp")
#include "MainPage.g.cpp"
#endif

#include "raggedy/qr/microsoft_link.hpp"
#include "raggedy/storage/profile_store.hpp"
#include "raggedy/ui/device_code_view.hpp"
#include "raggedy/ui/screen_models.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <string>
#include <string_view>

using namespace winrt;
using namespace Windows::ApplicationModel::DataTransfer;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Media::Imaging;

namespace winrt::RaggedyLauncher::implementation
{
    namespace
    {
        std::wstring widen(std::string_view text)
        {
            return std::wstring{text.begin(), text.end()};
        }
    }

    MainPage::MainPage()
    {
        InitializeComponent();
        launcher_ = App::launcher();
        if (launcher_ != nullptr)
        {
            settings_ = launcher_->settings();
            profiles_ = launcher_->profiles();
        }
        catalog_ = raggedy::catalog::make_built_in_catalog();
        QrImage().Source(CreateQrBitmap(8, 2));

        poll_timer_.Interval(std::chrono::seconds{1});
        poll_timer_.Tick([this](auto const&, auto const&)
        {
            if (launcher_ == nullptr)
            {
                return;
            }

            const auto stage = launcher_->auth_flow().session.stage;
            if (stage != raggedy::auth::Stage::waiting && stage != raggedy::auth::Stage::verifying)
            {
                return;
            }

            std::string error;
            const auto result = launcher_->poll_sign_in(error);
            if (result == raggedy::auth::PollResult::pending)
            {
                return;
            }
            if (result == raggedy::auth::PollResult::success)
            {
                StatusText().Text(L"Signed in and entitled.");
                ContentStatusText().Text(L"Signed in and entitled.");
            }
            else
            {
                StatusText().Text(to_hstring(error));
            }
            poll_timer_.Stop();
            Refresh();
        });
        poll_timer_.Start();

        CopyButton().Click([this](auto const&, auto const&)
        {
            DataPackage package;
            package.SetText(to_hstring(raggedy::auth::format_user_code(session_.challenge.user_code)));
            Clipboard::SetContent(package);
            StatusText().Text(L"Code copied.");
        });

        RetryButton().Click([this](auto const&, auto const&)
        {
            std::string error;
            if (launcher_ != nullptr && launcher_->begin_sign_in(error))
            {
                StatusText().Text(L"Requesting a sign-in code.");
                poll_timer_.Start();
            }
            else
            {
                StatusText().Text(to_hstring(error));
            }
            Refresh();
        });

        BackButton().Click([this](auto const&, auto const&)
        {
            std::string error;
            if (launcher_ != nullptr)
            {
                launcher_->sign_out(error);
            }
            Refresh();
        });

        MenuList().SelectionChanged([this](auto const&, auto const& args)
        {
            if (args.AddedItems().Size() == 0)
            {
                return;
            }

            const auto item = unbox_value<hstring>(args.AddedItems().First().Current());
            if (item == hstring{L"Versions"})
            {
                mode_ = PageMode::versions;
                RefreshShell();
                return;
            }
            if (item == hstring{L"Play"})
            {
                StartPlay();
                return;
            }
            if (item == hstring{L"Sign out"})
            {
                std::string error;
                if (launcher_ != nullptr)
                {
                    launcher_->sign_out(error);
                }
                Refresh();
                return;
            }
            ContentStatusText().Text(hstring{L"Selected: "} + item);
        });

        VersionList().SelectionChanged([this](auto const&, auto const& args)
        {
            if (args.AddedItems().Size() == 0)
            {
                return;
            }
            SelectVersionItem(unbox_value<hstring>(args.AddedItems().First().Current()));
        });

        MenuButton().Click([this](auto const&, auto const&)
        {
            mode_ = PageMode::menu;
            RefreshShell();
        });

        SignOutButton().Click([this](auto const&, auto const&)
        {
            std::string error;
            if (launcher_ != nullptr)
            {
                launcher_->sign_out(error);
            }
            Refresh();
        });

        PlayButton().Click([this](auto const&, auto const&)
        {
            StartPlay();
        });

        Refresh();
    }

    void MainPage::Refresh()
    {
        if (launcher_ == nullptr)
        {
            return;
        }

        std::string error;
        const auto session = launcher_->auth_flow().session;
        const auto view = raggedy::ui::make_device_code_view(session, error);
        DeviceCodeText().Text(to_hstring(view.user_code));
        StatusText().Text(to_hstring(view.status));
        CopyButton().Visibility(view.show_code ? Visibility::Visible : Visibility::Collapsed);
        RetryButton().Visibility(view.show_retry ? Visibility::Visible : Visibility::Collapsed);
        BackButton().Visibility(view.show_back ? Visibility::Visible : Visibility::Collapsed);
        SignInProgress().IsActive(view.show_progress);
        SignInProgress().Visibility(view.show_progress ? Visibility::Visible : Visibility::Collapsed);
    }

    void MainPage::RefreshShell()
    {
        const auto menu = raggedy::ui::make_main_menu_model(true, true, catalog_.valid());
        MenuList().Items().Clear();
        for (const std::wstring& action : menu.actions)
        {
            MenuList().Items().Append(box_value(action));
        }
        MenuStatusText().Text(menu.status);

        const bool versions = mode_ == PageMode::versions;
        ContentTitleText().Text(versions ? L"Versions" : L"Main menu");
        MenuContentPanel().Visibility(versions ? Visibility::Collapsed : Visibility::Visible);
        VersionsContentPanel().Visibility(versions ? Visibility::Visible : Visibility::Collapsed);

        if (versions)
        {
            PopulateVersions();
        }
        else
        {
            ContentStatusText().Text(menu.status);
        }
    }

    void MainPage::PopulateVersions()
    {
        const std::string pack_id = "simply-legacy";
        const raggedy::core::Profile* profile = raggedy::storage::find_profile(profiles_, profiles_.active_profile_id);
        if (profile != nullptr && !profile->target_id.empty())
        {
            selected_target_id_ = profile->target_id;
        }

        const auto entries = catalog_.entries_for(pack_id, true);
        const auto model = raggedy::ui::make_version_screen_model(entries, selected_target_id_, 0, 0, true);

        PackList().Items().Clear();
        VersionList().Items().Clear();
        version_options_.clear();

        for (const std::wstring& pack : model.packs)
        {
            PackList().Items().Append(box_value(pack));
        }

        for (const raggedy::catalog::Entry& entry : entries)
        {
            const std::wstring label = widen(entry.minecraft_version.empty() ? entry.id : entry.minecraft_version)
                + L" + " + widen(entry.loader);
            if (std::find_if(version_options_.begin(), version_options_.end(), [&label](const auto& option)
                {
                    return option.first == label;
                }) != version_options_.end())
            {
                continue;
            }
            version_options_.push_back({label, entry.id});
        }

        for (const std::wstring& version : model.versions)
        {
            VersionList().Items().Append(box_value(version));
        }

        for (const auto& option : version_options_)
        {
            if (option.second == selected_target_id_)
            {
                selected_target_id_ = option.second;
                break;
            }
        }

        ContentStatusText().Text(model.status);
    }

    void MainPage::SelectVersionItem(hstring const& item)
    {
        const std::wstring selected{item.c_str(), item.size()};
        for (const auto& option : version_options_)
        {
            if (option.first != selected)
            {
                continue;
            }

            selected_target_id_ = option.second;
            if (launcher_ != nullptr)
            {
                auto& store = launcher_->profiles();
                raggedy::core::Profile* profile = nullptr;
                for (auto& candidate : store.profiles)
                {
                    if (candidate.id == store.active_profile_id)
                    {
                        profile = &candidate;
                        break;
                    }
                }
                if (profile != nullptr)
                {
                    profile->target_id = selected_target_id_;
                }
                else if (!store.profiles.empty())
                {
                    store.active_profile_id = store.profiles.front().id;
                    store.profiles.front().target_id = selected_target_id_;
                }
                std::string error;
                launcher_->save_profiles(error);
            }
            ContentStatusText().Text(L"Selected " + item + L". Downloads start when you press Play.");
            return;
        }
    }

    void MainPage::StartPlay()
    {
        if (launcher_ == nullptr)
        {
            ContentStatusText().Text(L"Launcher services are not available.");
            return;
        }

        std::string error;
        const bool started = launcher_->play(launcher_->profiles(), catalog_, selected_target_id_, error);
        if (started)
        {
            ContentStatusText().Text(L"Game session finished.");
        }
        else
        {
            ContentStatusText().Text(L"Play failed: " + to_hstring(error));
        }
    }

    void MainPage::Update(raggedy::auth::DeviceCodeSession const& session)
    {
        session_ = session;
        Refresh();
    }
}
