#include "pch.h"
#include "App.xaml.g.hpp"
#include "MainPage.xaml.h"

using namespace winrt;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;

namespace winrt::RaggedyLauncher::implementation
{
    App::App()
    {
        InitializeComponent();
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
