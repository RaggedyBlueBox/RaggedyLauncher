#include "raggedy/auth/device_code.hpp"

#include <cctype>
#include <utility>

namespace raggedy::auth
{
    bool valid_challenge(const DeviceCodeChallenge& challenge)
    {
        const bool uri_ok = challenge.verification_uri.rfind("https://", 0) == 0;
        return !challenge.user_code.empty()
            && challenge.user_code.size() <= 16
            && uri_ok
            && challenge.expires_in_seconds > 0
            && challenge.poll_interval_seconds > 0;
    }

    std::string format_user_code(std::string_view code)
    {
        std::string formatted;
        for (std::size_t i = 0; i < code.size(); ++i)
        {
            if (i != 0 && i % 4 == 0)
            {
                formatted.push_back('-');
            }
            const unsigned char value = static_cast<unsigned char>(code[i]);
            formatted.push_back(static_cast<char>(std::toupper(value)));
        }
        return formatted;
    }

    void begin_request(DeviceCodeSession& session)
    {
        session.stage = Stage::requesting;
        session.challenge = DeviceCodeChallenge{};
        session.detail.clear();
        session.entitlement_verified = false;
    }

    void set_challenge(DeviceCodeSession& session, DeviceCodeChallenge challenge)
    {
        session.challenge = std::move(challenge);
        session.detail.clear();
        session.stage = valid_challenge(session.challenge) ? Stage::waiting : Stage::failed;
    }

    void mark_verifying(DeviceCodeSession& session)
    {
        if (session.stage == Stage::waiting)
        {
            session.stage = Stage::verifying;
        }
    }

    void mark_signed_in(DeviceCodeSession& session, bool entitlement_verified)
    {
        session.stage = Stage::signed_in;
        session.entitlement_verified = entitlement_verified;
        session.detail.clear();
    }

    void mark_failed(DeviceCodeSession& session, std::string detail)
    {
        session.stage = Stage::failed;
        session.detail = std::move(detail);
        session.entitlement_verified = false;
    }

    void cancel(DeviceCodeSession& session)
    {
        session.stage = Stage::cancelled;
        session.detail.clear();
        session.entitlement_verified = false;
    }

    bool needs_display_code(const DeviceCodeSession& session)
    {
        return (session.stage == Stage::waiting || session.stage == Stage::verifying)
            && valid_challenge(session.challenge);
    }

    bool may_download(const DeviceCodeSession& session)
    {
        return session.stage == Stage::signed_in && session.entitlement_verified;
    }

    bool may_launch(const DeviceCodeSession& session)
    {
        return may_download(session);
    }
}
