#include "raggedy/app/launcher.hpp"

#include "raggedy/core/profile.hpp"

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace raggedy::app
{
    namespace
    {
        std::string string_field(const core::JsonValue& value, std::string_view key)
        {
            const core::JsonValue* field = value.get(key);
            return field != nullptr && field->type == core::JsonType::string ? field->string : std::string{};
        }

        std::vector<std::string> string_array(const core::JsonValue* value)
        {
            std::vector<std::string> result;
            if (value == nullptr || value->type != core::JsonType::array)
            {
                return result;
            }
            for (const core::JsonValue& item : value->array)
            {
                if (item.type == core::JsonType::string && !item.string.empty())
                {
                    result.push_back(item.string);
                }
            }
            return result;
        }

        bool valid_sha256(const std::string& value)
        {
            if (value.size() != 64)
            {
                return false;
            }
            for (const char c : value)
            {
                const bool hex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
                if (!hex)
                {
                    return false;
                }
            }
            return true;
        }

        std::string manifest_path(const std::string& state_root, std::string_view id)
        {
            return state_root + "/manifests/" + std::string{id} + ".json";
        }
    }

    bool manifest_from_json(const core::JsonValue& value, TargetManifest& out, std::string& error)
    {
        out = TargetManifest{};
        if (value.type != core::JsonType::object)
        {
            error = "Target manifest must be a JSON object.";
            return false;
        }

        out.id = string_field(value, "id");
        out.plan_input.java_executable = string_field(value, "java_executable");
        out.plan_input.class_path = string_array(value.get("class_path"));
        out.plan_input.jvm_args = string_array(value.get("jvm_args"));
        out.plan_input.main_class = string_field(value, "main_class");
        out.plan_input.game_args = string_array(value.get("game_args"));
        out.plan_input.native_directory = string_field(value, "native_directory");
        out.required_directories = string_array(value.get("required_directories"));

        if (!core::valid_identifier(out.id) || out.plan_input.java_executable.empty()
            || out.plan_input.class_path.empty() || out.plan_input.main_class.empty())
        {
            error = "Target manifest is missing required fields.";
            return false;
        }

        const core::JsonValue* downloads = value.get("downloads");
        if (downloads == nullptr || downloads->type != core::JsonType::array)
        {
            error = "Target manifest has no download list.";
            return false;
        }

        for (const core::JsonValue& item : downloads->array)
        {
            if (item.type != core::JsonType::object)
            {
                error = "Target manifest contains an invalid download.";
                return false;
            }

            download::DownloadRequest request;
            request.url = string_field(item, "url");
            request.destination = string_field(item, "destination");
            request.expected_sha256 = string_field(item, "expected_sha256");
            if (request.url.rfind("https://", 0) != 0 || request.destination.empty()
                || !valid_sha256(request.expected_sha256))
            {
                error = "Target manifest contains an invalid download.";
                return false;
            }
            out.downloads.push_back(std::move(request));
        }

        return true;
    }

    Launcher::Launcher(platform::IHttpClient& http,
                       platform::IFileSystem& files,
                       platform::ICrypto& crypto,
                       platform::IProcessRunner& runner,
                       platform::IClock& clock,
                       platform::ILogger& logger)
        : http_(http)
        , files_(files)
        , clock_(clock)
        , logger_(logger)
        , downloads_(http, files, crypto)
        , session_(runner)
    {
    }

    bool Launcher::set_state_root(std::string root)
    {
        if (root.empty() || root.size() > 260)
        {
            return false;
        }
        state_root_ = std::move(root);
        return true;
    }

    bool Launcher::load_settings(std::string& error)
    {
        std::string text;
        if (!files_.read_text(state_path("settings.json"), text))
        {
            settings_ = storage::default_settings();
            return true;
        }

        core::JsonValue value;
        if (!core::parse_json(text, value))
        {
            error = "Settings file could not be parsed.";
            return false;
        }

        settings_ = storage::settings_from_json(value);
        if (!storage::valid_settings(settings_))
        {
            error = "Settings file is invalid.";
            return false;
        }
        return true;
    }

    bool Launcher::save_settings(std::string& error) const
    {
        if (!storage::valid_settings(settings_))
        {
            error = "Settings are invalid.";
            return false;
        }
        if (!files_.write_text(state_path("settings.json"), core::write_json(storage::settings_to_json(settings_))))
        {
            error = "Settings could not be saved.";
            return false;
        }
        return true;
    }

    bool Launcher::load_profiles(std::string& error)
    {
        std::string text;
        if (!files_.read_text(state_path("profiles.json"), text))
        {
            profiles_ = storage::default_profile_store();
            return true;
        }

        core::JsonValue value;
        if (!core::parse_json(text, value))
        {
            error = "Profiles file could not be parsed.";
            return false;
        }

        profiles_ = storage::profile_store_from_json(value);
        if (!storage::valid_profile_store(profiles_))
        {
            error = "Profiles file is invalid.";
            return false;
        }
        return true;
    }

    bool Launcher::save_profiles(std::string& error) const
    {
        if (!storage::valid_profile_store(profiles_))
        {
            error = "Profiles are invalid.";
            return false;
        }
        if (!files_.write_text(state_path("profiles.json"), core::write_json(storage::profile_store_to_json(profiles_))))
        {
            error = "Profiles could not be saved.";
            return false;
        }
        return true;
    }

    bool Launcher::begin_sign_in(std::string& error)
    {
        auth::AuthConfig config;
        config.client_id = settings_.microsoft_client_id;
        const bool result = auth::request_device_code(http_, clock_, config, flow_, error);
        if (result)
        {
            logger_.log("Launcher: sign-in code requested");
        }
        return result;
    }

    auth::PollResult Launcher::poll_sign_in(std::string& error)
    {
        const auth::PollResult result = auth::poll_device_code(http_, clock_, flow_, error);
        if (result != auth::PollResult::success)
        {
            return result;
        }
        if (!auth::verify_entitlement(http_, flow_, error))
        {
            return auth::PollResult::error;
        }
        logger_.log("Launcher: entitlement verified");
        return auth::PollResult::success;
    }

    bool Launcher::sign_out(std::string& error)
    {
        error.clear();
        downloads_.cancel_all();
        auth::sign_out(flow_);
        logger_.log("Launcher: signed out");
        return true;
    }

    bool Launcher::play(const storage::ProfileStore& store,
                        const catalog::Catalog& catalog,
                        std::string_view target_id,
                        std::string& error)
    {
        if (!auth::can_launch(flow_))
        {
            error = "Sign in and verify Minecraft Java ownership before playing.";
            return false;
        }

        std::string target{target_id};
        if (target.empty() && !store.active_profile_id.empty())
        {
            const core::Profile* profile = storage::find_profile(store, store.active_profile_id);
            if (profile != nullptr)
            {
                target = profile->target_id;
            }
        }
        if (!core::valid_identifier(target))
        {
            error = "Choose a valid version first.";
            return false;
        }
        if (catalog.find(target) == nullptr)
        {
            error = "The selected version is not in the built-in catalog.";
            return false;
        }

        std::string text;
        if (!files_.read_text(manifest_path(state_root_, target), text))
        {
            error = "No target manifest is available for this version.";
            return false;
        }

        core::JsonValue value;
        TargetManifest manifest;
        if (!core::parse_json(text, value) || !manifest_from_json(value, manifest, error))
        {
            if (error.empty())
            {
                error = "Target manifest could not be parsed.";
            }
            return false;
        }
        if (manifest.id != target)
        {
            error = "Target manifest does not match the selected version.";
            return false;
        }

        downloads_.clear();
        for (const std::string& directory : manifest.required_directories)
        {
            if (!files_.create_directories(directory))
            {
                error = "Failed to prepare version directories.";
                return false;
            }
        }
        for (const download::DownloadRequest& request : manifest.downloads)
        {
            downloads_.enqueue(request);
        }
        while (downloads_.run_one())
        {
        }

        for (const download::DownloadTask& task : downloads_.tasks())
        {
            if (task.status != download::DownloadStatus::complete)
            {
                error = task.error.empty() ? "A required file could not be prepared." : task.error;
                return false;
            }
        }

        core::LaunchPlan plan;
        if (!runtime::build_launch_plan(manifest.plan_input, plan, error))
        {
            return false;
        }

        runtime::SessionRequest request;
        request.plan = plan;
        request.authenticated = auth::can_launch(flow_);
        request.entitlement_verified = flow_.entitlement_verified;
        if (!session_.start(request, error))
        {
            return false;
        }

        logger_.log("Launcher: game session finished");
        return true;
    }

    void Launcher::cancel()
    {
        downloads_.cancel_all();
        auth::sign_out(flow_);
        logger_.log("Launcher: work cancelled");
    }

    const auth::DeviceCodeFlow& Launcher::auth_flow() const
    {
        return flow_;
    }

    const storage::Settings& Launcher::settings() const
    {
        return settings_;
    }

    storage::Settings& Launcher::settings()
    {
        return settings_;
    }

    const storage::ProfileStore& Launcher::profiles() const
    {
        return profiles_;
    }

    storage::ProfileStore& Launcher::profiles()
    {
        return profiles_;
    }

    const download::DownloadQueue& Launcher::downloads() const
    {
        return downloads_;
    }

    runtime::SessionState Launcher::session_state() const
    {
        return session_.state();
    }

    const std::string& Launcher::session_output() const
    {
        return session_.output();
    }

    std::string Launcher::state_path(std::string_view name) const
    {
        return state_root_ + "/" + std::string{name};
    }
}
