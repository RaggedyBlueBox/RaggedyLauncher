#pragma once

#include "pch.h"
#include "MainPage.xaml.g.h"

#include "raggedy/app/launcher.hpp"
#include "raggedy/auth/device_code.hpp"
#include "raggedy/catalog/built_in.hpp"
#include "raggedy/storage/profile_store.hpp"
#include "raggedy/storage/settings.hpp"

#include <string>
#include <utility>
#include <vector>

namespace winrt::RaggedyLauncher::implementation
{
    enum class PageMode
    {
        sign_in,
        menu,
        versions
    };

    struct MainPage : MainPageT<MainPage>
    {
        MainPage();

        void Update(raggedy::auth::DeviceCodeSession const& session);

    private:
        Windows::UI::Xaml::DispatcherTimer poll_timer_;
        raggedy::app::Launcher* launcher_ = nullptr;
        raggedy::auth::DeviceCodeSession session_;
        raggedy::storage::Settings settings_;
        raggedy::storage::ProfileStore profiles_;
        raggedy::catalog::Catalog catalog_;
        PageMode mode_ = PageMode::sign_in;
        std::string selected_target_id_ = "sl-26-3";
        std::vector<std::pair<std::wstring, std::string>> version_options_;

        void Refresh();
        void RefreshSignIn();
        void RefreshShell();
        void PopulateVersions();
        void SelectVersionItem(hstring const& item);
        void StartPlay();
        Windows::UI::Xaml::Media::Imaging::WriteableBitmap CreateQrBitmap(int scale, int quiet_modules);
    };
}

namespace winrt::RaggedyLauncher::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
