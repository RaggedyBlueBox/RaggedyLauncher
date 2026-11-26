#include "pch.h"
#include "App.xaml.h"

#include "Generated Files/App.g.cpp"

#include <winrt/Windows.ApplicationModel.h>

using namespace winrt;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;

namespace winrt::RaggedyLauncher::implementation
{
    raggedy::app::Launcher* App::active_launcher_ = nullptr;

    App::App()
    {
        InitializeComponent();
        launcher_ = std::make_unique<raggedy::app::Launcher>(http_, files_, crypto_, runner_, clock_, logger_);
        launcher_->set_state_root("state");
        active_launcher_ = launcher_.get();

        std::string error;
        launcher_->load_settings(error);
        runner_.set_bridge_url(launcher_->settings().process_bridge_url);
        launcher_->load_profiles(error);

        Suspending([this](auto const&, auto const&)
        {
            if (launcher_ != nullptr)
            {
                launcher_->cancel();
            }
        });
    }

    raggedy::app::Launcher* App::launcher()
    {
        return active_launcher_;
    }

    void App::OnLaunched(LaunchActivatedEventArgs const&)
    {
        auto root = Window::Current().Content().as<Frame>();
        if (root == nullptr)
        {
            root = Frame();
            Window::Current().Content(root);
        }

        if (!root.Navigate(winrt::xaml_typename<RaggedyLauncher::MainPage>()))
        {
            root.Navigate(winrt::xaml_typename<RaggedyLauncher::MainPage>());
        }

        Window::Current().Activate();
    }
}
