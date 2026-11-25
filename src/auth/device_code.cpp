#include "raggedy/auth/device_code.hpp"

#include "raggedy/core/json.hpp"

#include <cctype>
#include <cmath>
#include <cstddef>
#include <limits>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

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

    namespace
    {
        constexpr std::string_view kDeviceCodePath = "/oauth2/v2.0/devicecode";
        constexpr std::string_view kTokenPath = "/oauth2/v2.0/token";
        constexpr std::string_view kXboxAuthenticateUrl = "https://user.auth.xboxlive.com/user/authenticate";
        constexpr std::string_view kXstsAuthorizeUrl = "https://xsts.auth.xboxlive.com/xsts/authorize";
        constexpr std::string_view kMinecraftLoginUrl = "https://api.minecraftservices.com/authentication/login_with_xbox";
        constexpr std::string_view kMinecraftEntitlementsUrl = "https://api.minecraftservices.com/entitlements/mcstore";
        constexpr std::string_view kMinecraftProfileUrl = "https://api.minecraftservices.com/minecraft/profile";

        std::vector<std::byte> bytes_from(std::string_view text)
        {
            std::vector<std::byte> result;
            result.reserve(text.size());
            for (const char c : text)
            {
                result.push_back(static_cast<std::byte>(static_cast<unsigned char>(c)));
            }
            return result;
        }

        std::string text_from(const std::vector<std::byte>& bytes)
        {
            std::string text;
            text.reserve(bytes.size());
            for (const std::byte value : bytes)
            {
                text.push_back(std::to_integer<char>(value));
            }
            return text;
        }

        std::string form_encode(std::string_view value)
        {
            static constexpr char hex[] = "0123456789ABCDEF";
            std::string encoded;
            encoded.reserve(value.size() * 3);
            for (const char raw : value)
            {
                const unsigned char c = static_cast<unsigned char>(raw);
                const bool unreserved = (c >= 'A' && c <= 'Z')
                    || (c >= 'a' && c <= 'z')
                    || (c >= '0' && c <= '9')
                    || c == '-' || c == '_' || c == '.' || c == '~';
                if (unreserved)
                {
                    encoded.push_back(static_cast<char>(c));
                }
                else
                {
                    encoded.push_back('%');
                    encoded.push_back(hex[c >> 4]);
                    encoded.push_back(hex[c & 0x0F]);
                }
            }
            return encoded;
        }

        std::string lower_ascii(std::string value)
        {
            for (char& c : value)
            {
                if (c >= 'A' && c <= 'Z')
                {
                    c = static_cast<char>(c - 'A' + 'a');
                }
            }
            return value;
        }

        bool valid_tenant(const std::string& tenant)
        {
            if (tenant == "common" || tenant == "consumers" || tenant == "organizations")
            {
                return true;
            }
            if (tenant.empty() || tenant.size() > 64)
            {
                return false;
            }
            for (const char c : tenant)
            {
                const bool hex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
                if (!hex && c != '-')
                {
                    return false;
                }
            }
            return true;
        }

        bool status_ok(int status)
        {
            return status >= 200 && status <= 299;
        }

        std::string string_field(const core::JsonValue& value, std::string_view key)
        {
            const core::JsonValue* field = value.get(key);
            return field != nullptr && field->type == core::JsonType::string ? field->string : std::string{};
        }

        int positive_int_field(const core::JsonValue& value, std::string_view key)
        {
            const core::JsonValue* field = value.get(key);
            if (field == nullptr || field->type != core::JsonType::number || !std::isfinite(field->number))
            {
                return 0;
            }
            if (field->number <= 0.0 || field->number > static_cast<double>(std::numeric_limits<int>::max()))
            {
                return 0;
            }
            return static_cast<int>(field->number);
        }

        bool parse_object(const platform::HttpResponse& response, core::JsonValue& out)
        {
            return parse_json(text_from(response.body), out) && out.type == core::JsonType::object;
        }

        std::string display_claim(const core::JsonValue& payload, std::string_view key)
        {
            const core::JsonValue* claims = payload.get("DisplayClaims");
            if (claims == nullptr || claims->type != core::JsonType::object)
            {
                return {};
            }
            const core::JsonValue* xui = claims->get("xui");
            if (xui == nullptr || xui->type != core::JsonType::array || xui->size() == 0)
            {
                return {};
            }
            return string_field(xui->at(0), key);
        }

        platform::HttpResponse send_form(platform::IHttpClient& http,
                                         const std::string& url,
                                         const std::string& body)
        {
            platform::HttpRequest request;
            request.method = "POST";
            request.url = url;
            request.headers["Content-Type"] = "application/x-www-form-urlencoded";
            request.body = bytes_from(body);
            return http.send(request);
        }

        platform::HttpResponse send_json(platform::IHttpClient& http,
                                         const std::string& url,
                                         const core::JsonValue& payload,
                                         const std::map<std::string, std::string>& extra_headers)
        {
            platform::HttpRequest request;
            request.method = "POST";
            request.url = url;
            request.headers["Content-Type"] = "application/json";
            request.headers["Accept"] = "application/json";
            for (const auto& [key, value] : extra_headers)
            {
                request.headers[key] = value;
            }
            request.body = bytes_from(core::write_json(payload));
            return http.send(request);
        }

        platform::HttpResponse send_get(platform::IHttpClient& http,
                                        const std::string& url,
                                        const std::string& authorization)
        {
            platform::HttpRequest request;
            request.method = "GET";
            request.url = url;
            request.headers["Accept"] = "application/json";
            request.headers["Authorization"] = authorization;
            return http.send(request);
        }

        void fail(DeviceCodeFlow& flow, std::string error)
        {
            flow.tokens.clear();
            flow.entitlement_verified = false;
            mark_failed(flow.session, std::move(error));
        }

        bool has_java_entitlement(const core::JsonValue& payload)
        {
            const core::JsonValue* items = payload.get("items");
            if (items == nullptr || items->type != core::JsonType::array)
            {
                return false;
            }

            for (const core::JsonValue& item : items->array)
            {
                const std::string name = lower_ascii(string_field(item, "name"));
                const std::string kind = lower_ascii(string_field(item, "kind"));
                if (name == "game" || name == "minecraft" || name == "minecraft_java"
                    || kind == "minecraft" || kind.find("minecraft") != std::string::npos)
                {
                    return true;
                }
            }
            return false;
        }
    }

    void AuthTokens::clear()
    {
        microsoft_access_token.clear();
        microsoft_refresh_token.clear();
        xbox_token.clear();
        xbox_user_hash.clear();
        xsts_token.clear();
        xuid.clear();
        minecraft_access_token.clear();
        account_id.clear();
        display_name.clear();
    }

    bool request_device_code(platform::IHttpClient& http,
                             platform::IClock& clock,
                             const AuthConfig& config,
                             DeviceCodeFlow& flow,
                             std::string& error)
    {
        flow.config = config;
        flow.tokens.clear();
        flow.device_code.clear();
        flow.entitlement_verified = false;
        begin_request(flow.session);

        if (config.client_id.empty() || !valid_tenant(config.tenant))
        {
            error = "Microsoft sign-in needs an approved application ID.";
            fail(flow, error);
            return false;
        }

        const std::string url = "https://login.microsoftonline.com/" + config.tenant + std::string{kDeviceCodePath};
        const std::string body = "client_id=" + form_encode(config.client_id) + "&scope=" + form_encode(config.scope);
        const platform::HttpResponse response = send_form(http, url, body);

        core::JsonValue payload;
        if (!status_ok(response.status) || !parse_object(response, payload))
        {
            error = "Microsoft sign-in request failed.";
            fail(flow, error);
            return false;
        }

        DeviceCodeChallenge challenge;
        challenge.user_code = string_field(payload, "user_code");
        challenge.verification_uri = string_field(payload, "verification_uri");
        challenge.expires_in_seconds = positive_int_field(payload, "expires_in");
        challenge.poll_interval_seconds = positive_int_field(payload, "interval");
        flow.device_code = string_field(payload, "device_code");

        if (!valid_challenge(challenge) || flow.device_code.empty())
        {
            error = "Microsoft sign-in returned an invalid code.";
            fail(flow, error);
            return false;
        }

        const auto now = clock.now();
        flow.poll_interval_seconds = challenge.poll_interval_seconds;
        flow.expires_at = now + std::chrono::seconds(challenge.expires_in_seconds);
        flow.next_poll_at = now + std::chrono::seconds(flow.poll_interval_seconds);
        set_challenge(flow.session, std::move(challenge));
        return flow.session.stage == Stage::waiting;
    }

    PollResult poll_device_code(platform::IHttpClient& http,
                                platform::IClock& clock,
                                DeviceCodeFlow& flow,
                                std::string& error)
    {
        if (flow.device_code.empty())
        {
            error = "Microsoft sign-in has no active code.";
            fail(flow, error);
            return PollResult::error;
        }

        const auto now = clock.now();
        if (now < flow.next_poll_at)
        {
            return PollResult::pending;
        }
        if (now >= flow.expires_at)
        {
            error = "The sign-in code expired. Request a new code.";
            fail(flow, error);
            return PollResult::expired;
        }

        const std::string url = "https://login.microsoftonline.com/" + flow.config.tenant + std::string{kTokenPath};
        const std::string body = std::string{"grant_type=urn%3Aietf%3Aparams%3Aoauth%3Agrant-type%3Adevice_code"}
            + "&client_id=" + form_encode(flow.config.client_id)
            + "&device_code=" + form_encode(flow.device_code);
        const platform::HttpResponse response = send_form(http, url, body);

        core::JsonValue payload;
        if (!parse_object(response, payload))
        {
            error = "Microsoft sign-in returned an invalid response.";
            fail(flow, error);
            return PollResult::error;
        }

        if (!status_ok(response.status))
        {
            const std::string code = string_field(payload, "error");
            if (code == "authorization_pending")
            {
                flow.next_poll_at = now + std::chrono::seconds(flow.poll_interval_seconds);
                return PollResult::pending;
            }
            if (code == "slow_down")
            {
                flow.poll_interval_seconds = flow.poll_interval_seconds >= 55 ? 60 : flow.poll_interval_seconds + 5;
                flow.next_poll_at = now + std::chrono::seconds(flow.poll_interval_seconds);
                return PollResult::pending;
            }
            if (code == "authorization_declined")
            {
                error = "Sign-in was declined.";
                fail(flow, error);
                return PollResult::denied;
            }
            if (code == "expired_token")
            {
                error = "The sign-in code expired. Request a new code.";
                fail(flow, error);
                return PollResult::expired;
            }
            if (code == "bad_verification_code")
            {
                error = "The sign-in code was not recognized. Request a new code.";
                fail(flow, error);
                return PollResult::error;
            }

            error = "Microsoft sign-in failed.";
            fail(flow, error);
            return PollResult::error;
        }

        flow.tokens.microsoft_access_token = string_field(payload, "access_token");
        flow.tokens.microsoft_refresh_token = string_field(payload, "refresh_token");
        if (flow.tokens.microsoft_access_token.empty())
        {
            error = "Microsoft sign-in did not return an access token.";
            fail(flow, error);
            return PollResult::error;
        }

        mark_verifying(flow.session);
        return PollResult::success;
    }

    bool verify_entitlement(platform::IHttpClient& http, DeviceCodeFlow& flow, std::string& error)
    {
        if (flow.tokens.microsoft_access_token.empty())
        {
            error = "Sign in with Microsoft first.";
            fail(flow, error);
            return false;
        }

        const core::JsonValue xbox_request = core::JsonValue::from_object({
            {"Properties", core::JsonValue::from_object({
                {"AuthMethod", core::JsonValue::from_string("ProofOfPossession")},
                {"SiteName", core::JsonValue::from_string("uri:http://xboxlive.com")},
                {"Version", core::JsonValue::from_string("1.0")}
            })},
            {"RelyingParty", core::JsonValue::from_string("http://auth.xboxlive.com")},
            {"TokenType", core::JsonValue::from_string("JWT")}
        });
        const platform::HttpResponse xbox_response = send_json(http, std::string{kXboxAuthenticateUrl}, xbox_request, {{"x-xbl-contract-version", "1"}});

        core::JsonValue xbox_payload;
        if (!status_ok(xbox_response.status) || !parse_object(xbox_response, xbox_payload))
        {
            error = "Xbox sign-in failed.";
            fail(flow, error);
            return false;
        }

        flow.tokens.xbox_token = string_field(xbox_payload, "Token");
        flow.tokens.xbox_user_hash = display_claim(xbox_payload, "uhs");
        if (flow.tokens.xbox_token.empty() || flow.tokens.xbox_user_hash.empty())
        {
            error = "Xbox sign-in failed.";
            fail(flow, error);
            return false;
        }

        const core::JsonValue xsts_request = core::JsonValue::from_object({
            {"Properties", core::JsonValue::from_object({
                {"Sandbox", core::JsonValue::from_string("RETAIL")},
                {"SiteName", core::JsonValue::from_string("uri:http://minecraft.net")},
                {"Version", core::JsonValue::from_string("1")}
            })},
            {"RelyingParty", core::JsonValue::from_string("rp://api.minecraftservices.com/")},
            {"TokenType", core::JsonValue::from_string("JWT")}
        });
        const std::string xbox_authorization = "XBL3.0 x=" + flow.tokens.xbox_user_hash + ";" + flow.tokens.xbox_token;
        const platform::HttpResponse xsts_response = send_json(http, std::string{kXstsAuthorizeUrl}, xsts_request, {{"Authorization", xbox_authorization}});

        core::JsonValue xsts_payload;
        if (!status_ok(xsts_response.status) || !parse_object(xsts_response, xsts_payload))
        {
            error = "Xbox authorization failed. An Xbox account may need to be set up first.";
            fail(flow, error);
            return false;
        }

        flow.tokens.xsts_token = string_field(xsts_payload, "Token");
        flow.tokens.xuid = display_claim(xsts_payload, "xuid");
        if (flow.tokens.xsts_token.empty() || flow.tokens.xbox_user_hash.empty())
        {
            error = "Xbox authorization failed.";
            fail(flow, error);
            return false;
        }

        const core::JsonValue minecraft_request = core::JsonValue::from_object({
            {"identityToken", core::JsonValue::from_string("XBL3.0 x=" + flow.tokens.xbox_user_hash + ";" + flow.tokens.xsts_token)}
        });
        const platform::HttpResponse minecraft_response = send_json(http, std::string{kMinecraftLoginUrl}, minecraft_request, {});

        core::JsonValue minecraft_payload;
        if (!status_ok(minecraft_response.status) || !parse_object(minecraft_response, minecraft_payload))
        {
            if (minecraft_response.status == 403)
            {
                error = "Minecraft services rejected this application ID. It must be approved for Minecraft access.";
            }
            else
            {
                error = "Minecraft sign-in failed.";
            }
            fail(flow, error);
            return false;
        }

        flow.tokens.minecraft_access_token = string_field(minecraft_payload, "access_token");
        if (flow.tokens.minecraft_access_token.empty())
        {
            error = "Minecraft sign-in did not return an access token.";
            fail(flow, error);
            return false;
        }

        const std::string minecraft_authorization = "Bearer " + flow.tokens.minecraft_access_token;
        const platform::HttpResponse entitlements_response = send_get(http, std::string{kMinecraftEntitlementsUrl}, minecraft_authorization);

        core::JsonValue entitlements_payload;
        if (!status_ok(entitlements_response.status) || !parse_object(entitlements_response, entitlements_payload))
        {
            error = "Minecraft ownership check failed.";
            fail(flow, error);
            return false;
        }
        if (!has_java_entitlement(entitlements_payload))
        {
            error = "Minecraft Java Edition was not found on this account.";
            fail(flow, error);
            return false;
        }

        const platform::HttpResponse profile_response = send_get(http, std::string{kMinecraftProfileUrl}, minecraft_authorization);

        core::JsonValue profile_payload;
        if (!status_ok(profile_response.status) || !parse_object(profile_response, profile_payload))
        {
            error = "Minecraft profile lookup failed.";
            fail(flow, error);
            return false;
        }

        flow.tokens.account_id = string_field(profile_payload, "id");
        flow.tokens.display_name = string_field(profile_payload, "name");
        if (flow.tokens.account_id.empty() || flow.tokens.display_name.empty())
        {
            error = "Minecraft profile lookup failed.";
            fail(flow, error);
            return false;
        }

        flow.entitlement_verified = true;
        mark_signed_in(flow.session, true);
        return true;
    }

    void sign_out(DeviceCodeFlow& flow)
    {
        flow.tokens.clear();
        flow.device_code.clear();
        flow.entitlement_verified = false;
        flow.expires_at = std::chrono::steady_clock::time_point{};
        flow.next_poll_at = std::chrono::steady_clock::time_point{};
        cancel(flow.session);
    }

    bool can_launch(const DeviceCodeFlow& flow)
    {
        return may_launch(flow.session)
            && flow.entitlement_verified
            && !flow.tokens.minecraft_access_token.empty()
            && !flow.tokens.account_id.empty();
    }
}
