#pragma once

#include "pch.h"
#include "MainPage.xaml.g.h"

#include "raggedy/auth/device_code.hpp"

namespace winrt::RaggedyLauncher::implementation
{
    struct MainPage : MainPageT<MainPage>
    {
        MainPage();

        void Update(raggedy::auth::DeviceCodeSession const& session);

    private:
        raggedy::auth::DeviceCodeSession session_;

        void Refresh();
        Windows::UI::Xaml::Media::Imaging::WriteableBitmap CreateQrBitmap(int scale, int quiet_modules);
    };
}

namespace winrt::RaggedyLauncher::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
