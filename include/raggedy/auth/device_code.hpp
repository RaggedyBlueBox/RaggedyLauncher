#pragma once

#include "raggedy/platform/interfaces.hpp"

#include <chrono>
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

    struct AuthConfig
    {
        std::string client_id;
        std::string tenant = "consumers";
        std::string scope = "XboxLive.signin openid profile offline_access";
    };

    struct AuthTokens
    {
        std::string microsoft_access_token;
        std::string microsoft_refresh_token;
        std::string xbox_token;
        std::string xbox_user_hash;
        std::string xsts_token;
        std::string xuid;
        std::string minecraft_access_token;
        std::string account_id;
        std::string display_name;

        void clear();
    };

    struct DeviceCodeFlow
    {
        DeviceCodeSession session;
        AuthConfig config;
        std::string device_code;
        std::chrono::steady_clock::time_point expires_at{};
        std::chrono::steady_clock::time_point next_poll_at{};
        int poll_interval_seconds = 5;
        AuthTokens tokens;
        bool entitlement_verified = false;
    };

    enum class PollResult
    {
        pending,
        success,
        expired,
        denied,
        error
    };

    bool request_device_code(platform::IHttpClient& http,
                             platform::IClock& clock,
                             const AuthConfig& config,
                             DeviceCodeFlow& flow,
                             std::string& error);

    PollResult poll_device_code(platform::IHttpClient& http,
                                platform::IClock& clock,
                                DeviceCodeFlow& flow,
                                std::string& error);

    bool verify_entitlement(platform::IHttpClient& http, DeviceCodeFlow& flow, std::string& error);
    void sign_out(DeviceCodeFlow& flow);
    bool can_launch(const DeviceCodeFlow& flow);
}
