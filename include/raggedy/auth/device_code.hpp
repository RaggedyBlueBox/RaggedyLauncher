#pragma once

#include <string>
#include <string_view>

namespace raggedy::auth
{
    enum class Stage
    {
        idle,
        requesting,
        waiting,
        verifying,
        signed_in,
        failed,
        cancelled
    };

    struct DeviceCodeChallenge
    {
        std::string user_code;
        std::string verification_uri;
        int expires_in_seconds = 0;
        int poll_interval_seconds = 0;
    };

    struct DeviceCodeSession
    {
        Stage stage = Stage::idle;
        DeviceCodeChallenge challenge;
        std::string detail;
        bool entitlement_verified = false;
    };

    bool valid_challenge(const DeviceCodeChallenge& challenge);
    std::string format_user_code(std::string_view code);
    void begin_request(DeviceCodeSession& session);
    void set_challenge(DeviceCodeSession& session, DeviceCodeChallenge challenge);
    void mark_verifying(DeviceCodeSession& session);
    void mark_signed_in(DeviceCodeSession& session, bool entitlement_verified);
    void mark_failed(DeviceCodeSession& session, std::string detail);
    void cancel(DeviceCodeSession& session);
    bool needs_display_code(const DeviceCodeSession& session);
    bool may_download(const DeviceCodeSession& session);
    bool may_launch(const DeviceCodeSession& session);
}
