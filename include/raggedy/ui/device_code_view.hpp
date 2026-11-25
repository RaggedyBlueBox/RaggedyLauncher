#pragma once

#include "raggedy/auth/device_code.hpp"

#include <string>

namespace raggedy::ui
{
    struct DeviceCodeView
    {
        std::wstring title;
        std::wstring heading;
        std::wstring instructions;
        std::wstring formatted_code;
        std::wstring link_label;
        std::wstring status;
        bool show_qr = false;
        bool show_copy = false;
        bool show_retry = false;
        bool show_back = false;
        bool show_progress = false;
    };

    DeviceCodeView make_device_code_view(const raggedy::auth::DeviceCodeSession& session);
}
