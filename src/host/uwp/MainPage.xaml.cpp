#include "pch.h"
#include "App.xaml.h"
#include "MainPage.xaml.h"

#if __has_include("Generated Files/MainPage.g.cpp")
#include "Generated Files/MainPage.g.cpp"
#elif __has_include("MainPage.g.cpp")
#include "MainPage.g.cpp"
#endif

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

        winrt::hstring to_hstring(std::string_view text)
        {
            return winrt::hstring{widen(text)};
        }

        winrt::hstring to_hstring(const std::wstring& text)
        {
            return winrt::hstring{text};
        }

        winrt::hstring to_hstring(const std::string& text)
        {
            return winrt::hstring{widen(text)};
        }

        // QR code generation for Microsoft link
        constexpr std::size_t qr_size = 25;

        constexpr std::array<std::string_view, qr_size> microsoft_link_rows{
            "1111110010011101110111111",
            "1000010010011111000100001",
            "1011010100001100100101101",
            "1011010111000111100101101",
            "1000010101000000000100001",
            "1111110100101011010111111",
            "0000000100101010010000000",
            "1011110000000010010111100",
            "0101000000011101111110001",
            "0010000100110100110011010",
            "1111110010001101000001100",
            "1010000111000111111100001",
            "1111010101100000011111100",
            "1100101100001010110010010",
            "1001110000100010100011100",
            "1000111001111110011010100",
            "1010000000110101100110010",
            "1011010000001100011110111",
            "0000000110000111110001111",
            "1111110000000001110101100",
            "1000010100001010110000011",
            "1011010100101101110011100",
            "1011010111100110110111110",
            "1000010010011001011011010",
            "1111110101011000101111100"
        };

        constexpr bool microsoft_link_module(int x, int y)
        {
            if (x < 0 || y < 0 || x >= static_cast<int>(qr_size) || y >= static_cast<int>(qr_size))
            {
                return false;
            }
            return microsoft_link_rows[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)] == '1';
        }

        std::string make_microsoft_link_uri(std::string_view verification_uri, std::string_view user_code)
        {
            std::string uri(verification_uri);
            if (!uri.empty() && uri.back() != '/')
            {
                uri.push_back('/');
            }
            uri += user_code;
            return uri;
        }

        winrt::Windows::UI::Xaml::Media::Imaging::WriteableBitmap create_qr_bitmap(std::string_view uri, int scale, int quiet_modules)
        {
            const int modules = static_cast<int>(qr_size) + 2 * quiet_modules;
            const int pixel_size = modules * scale;
            auto bitmap = winrt::Windows::UI::Xaml::Media::Imaging::WriteableBitmap(pixel_size, pixel_size);

            auto pixel_buffer = bitmap.PixelBuffer();
            std::vector<uint32_t> pixels(pixel_size * pixel_size);

            for (int y = 0; y < pixel_size; ++y)
            {
                for (int x = 0; x < pixel_size; ++x)
                {
                    const int mx = x / scale - quiet_modules;
                    const int my = y / scale - quiet_modules;
                    const bool dark = microsoft_link_module(mx, my);
                    pixels[y * pixel_size + x] = dark ? 0xFF000000 : 0xFFFFFFFF;
                }
            }

            // Copy pixels to buffer using DataWriter
            auto data_writer = winrt::Windows::Storage::Streams::DataWriter();
            for (uint32_t pixel : pixels)
            {
                data_writer.WriteUInt32(pixel);
            }
            auto buffer = data_writer.DetachBuffer();
            pixel_buffer = buffer;

            bitmap.Invalidate();
            return bitmap;
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

    winrt::Windows::UI::Xaml::Media::Imaging::WriteableBitmap MainPage::CreateQrBitmap(int scale, int quiet_modules)
    {
        const std::string uri = make_microsoft_link_uri(session_.challenge.verification_uri, session_.challenge.user_code);
        return create_qr_bitmap(uri, scale, quiet_modules);
    }

    void MainPage::Refresh()
    {
        if (launcher_ == nullptr)
        {
            return;
        }

        const auto session = launcher_->auth_flow().session;
        const auto view = raggedy::ui::make_device_code_view(session);
        CodeText().Text(to_hstring(view.formatted_code));
        StatusText().Text(to_hstring(view.status));
        CopyButton().Visibility(view.show_copy ? Visibility::Visible : Visibility::Collapsed);
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
        MenuStatusText().Text(to_hstring(menu.status));

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
            ContentStatusText().Text(to_hstring(menu.status));
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

        ContentStatusText().Text(to_hstring(model.status));
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
