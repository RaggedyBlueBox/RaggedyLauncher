#include "pch.h"
#include "MainPage.xaml.g.hpp"
#include "MainPage.xaml.h"

#include "raggedy/qr/microsoft_link.hpp"
#include "raggedy/ui/device_code_view.hpp"

#include <cstdint>

using namespace winrt;
using namespace Windows::ApplicationModel::DataTransfer;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Media::Imaging;

namespace winrt::RaggedyLauncher::implementation
{
    MainPage::MainPage()
    {
        InitializeComponent();
        QrImage().Source(CreateQrBitmap(8, 2));

        CopyButton().Click([this](auto const&, auto const&)
        {
            DataPackage package;
            package.SetText(to_hstring(raggedy::auth::format_user_code(session_.challenge.user_code)));
            Clipboard::SetContent(package);
            StatusText().Text(L"Code copied.");
        });

        RetryButton().Click([this](auto const&, auto const&)
        {
            raggedy::auth::begin_request(session_);
            Refresh();
        });

        BackButton().Click([this](auto const&, auto const&)
        {
            raggedy::auth::cancel(session_);
            Refresh();
        });

        Refresh();
    }

    Windows::UI::Xaml::Media::Imaging::WriteableBitmap MainPage::CreateQrBitmap(int scale, int quiet_modules)
    {
        const int module_count = static_cast<int>(raggedy::qr::qr_size) + quiet_modules * 2;
        const int pixel_count = module_count * scale;
        WriteableBitmap bitmap(pixel_count, pixel_count);

        auto buffer = bitmap.PixelBuffer();
        com_ptr<::IBufferByteAccess> access;
        buffer.as(access);

        byte* raw = nullptr;
        check_hresult(access->Buffer(&raw));
        auto* pixels = reinterpret_cast<std::uint32_t*>(raw);

        for (int y = 0; y < pixel_count; ++y)
        {
            for (int x = 0; x < pixel_count; ++x)
            {
                const int module_x = x / scale - quiet_modules;
                const int module_y = y / scale - quiet_modules;
                const bool dark = raggedy::qr::microsoft_link_module(module_x, module_y);
                pixels[y * pixel_count + x] = dark ? 0xFF1B2444u : 0xFFFFFFFFu;
            }
        }

        bitmap.Invalidate();
        return bitmap;
    }

    void MainPage::Refresh()
    {
        const auto view = raggedy::ui::make_device_code_view(session_);

        TitleText().Text(to_hstring(view.title));
        HeadingText().Text(to_hstring(view.heading));
        InstructionsText().Text(to_hstring(view.instructions));
        CodeText().Text(view.formatted_code.empty() ? L"---- ----" : view.formatted_code);
        LinkText().Text(view.link_label);
        StatusText().Text(view.status);

        QrCard().Visibility(view.show_qr ? Visibility::Visible : Visibility::Collapsed);
        CopyButton().Visibility(view.show_copy ? Visibility::Visible : Visibility::Collapsed);
        RetryButton().Visibility(view.show_retry ? Visibility::Visible : Visibility::Collapsed);
        BackButton().Visibility(view.show_back ? Visibility::Visible : Visibility::Collapsed);
        SignInProgress().IsActive(view.show_progress);
        SignInProgress().Visibility(view.show_progress ? Visibility::Visible : Visibility::Collapsed);
    }

    void MainPage::Update(raggedy::auth::DeviceCodeSession const& session)
    {
        session_ = session;
        Refresh();
    }
}
