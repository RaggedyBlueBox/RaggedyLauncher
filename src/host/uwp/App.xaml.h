#pragma once

#include "pch.h"

namespace winrt::RaggedyLauncher::implementation
{
    struct App : ApplicationT<App>
    {
        App();
        void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs const& args);
    };
}

namespace winrt::RaggedyLauncher::factory_implementation
{
    struct App : AppT<App, implementation::App>
    {
    };
}
