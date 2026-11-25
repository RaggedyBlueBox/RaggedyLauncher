#include "raggedy/ui/device_code_view.hpp"

#include <string_view>

namespace raggedy::ui
{
    namespace
    {
        std::wstring widen(std::string_view text)
        {
            return std::wstring(text.begin(), text.end());
        }

        std::wstring status_text(const auth::DeviceCodeSession& session)
        {
            switch (session.stage)
            {
            case auth::Stage::idle:
                return L"Ready when you are.";
            case auth::Stage::requesting:
                return L"Asking Microsoft for a sign-in code...";
            case auth::Stage::waiting:
                return L"Waiting for you to approve this sign-in.";
            case auth::Stage::verifying:
                return L"Checking your account and Minecraft Java ownership...";
            case auth::Stage::signed_in:
                return session.entitlement_verified
                    ? L"Signed in. Minecraft Java is ready."
                    : L"Signed in, but Minecraft Java was not found on this account.";
            case auth::Stage::failed:
                return session.detail.empty()
                    ? L"Sign-in failed. Please try again."
                    : L"Sign-in failed. " + widen(session.detail);
            case auth::Stage::cancelled:
                return L"Sign-in cancelled.";
            }
            return L"";
        }
    }

    DeviceCodeView make_device_code_view(const auth::DeviceCodeSession& session)
    {
        DeviceCodeView view;
        view.title = L"Raggedy Launcher";
        view.heading = L"Sign in with Microsoft";
        view.instructions = L"Open the link on your phone or computer, then enter this code. The QR code opens the same page.";
        view.formatted_code = widen(auth::format_user_code(session.challenge.user_code));
        view.link_label = session.challenge.verification_uri.empty()
            ? L"microsoft.com/link"
            : widen(session.challenge.verification_uri);
        view.status = status_text(session);
        view.show_qr = auth::needs_display_code(session);
        view.show_copy = auth::needs_display_code(session);
        view.show_retry = session.stage == auth::Stage::failed || session.stage == auth::Stage::cancelled;
        view.show_back = session.stage != auth::Stage::waiting && session.stage != auth::Stage::verifying;
        view.show_progress = session.stage == auth::Stage::requesting || session.stage == auth::Stage::verifying;
        return view;
    }
}
