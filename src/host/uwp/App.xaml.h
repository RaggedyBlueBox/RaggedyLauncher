#pragma once

#include "pch.h"
#include "App.g.h"
#include "App.xaml.g.h"
#include "platform_services.hpp"

#include "raggedy/app/launcher.hpp"

#include <memory>

namespace winrt::RaggedyLauncher::implementation
{
    struct App : AppT<App>
    {
        App();

        static raggedy::app::Launcher* launcher();

        void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs const& args);

    private:
        UwpHttpClient http_;
        LocalStateFileSystem files_;
        WindowsCrypto crypto_;
        BridgeProcessRunner runner_{};
        SteadyClock clock_;
        RedactingLogger logger_;
        std::unique_ptr<raggedy::app::Launcher> launcher_;
        static raggedy::app::Launcher* active_launcher_;
    };
}

namespace winrt::RaggedyLauncher::factory_implementation
{
    struct App : AppT<App, implementation::App>
    {
    };
}
