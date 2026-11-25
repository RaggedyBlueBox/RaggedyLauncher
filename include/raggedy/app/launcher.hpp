#pragma once

#include "raggedy/auth/device_code.hpp"
#include "raggedy/catalog/catalog.hpp"
#include "raggedy/core/json.hpp"
#include "raggedy/download/queue.hpp"
#include "raggedy/platform/interfaces.hpp"
#include "raggedy/runtime/launch_plan_builder.hpp"
#include "raggedy/runtime/session.hpp"
#include "raggedy/storage/profile_store.hpp"
#include "raggedy/storage/settings.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace raggedy::app
{
    struct TargetManifest
    {
        std::string id;
        runtime::PlanInput plan_input;
        std::vector<download::DownloadRequest> downloads;
        std::vector<std::string> required_directories;
    };

    bool manifest_from_json(const core::JsonValue& value, TargetManifest& out, std::string& error);

    class Launcher
    {
    public:
        Launcher(platform::IHttpClient& http,
                 platform::IFileSystem& files,
                 platform::ICrypto& crypto,
                 platform::IProcessRunner& runner,
                 platform::IClock& clock,
                 platform::ILogger& logger);

        bool set_state_root(std::string root);
        bool load_settings(std::string& error);
        bool save_settings(std::string& error) const;
        bool load_profiles(std::string& error);
        bool save_profiles(std::string& error) const;

        bool begin_sign_in(std::string& error);
        auth::PollResult poll_sign_in(std::string& error);
        bool sign_out(std::string& error);
        bool play(const storage::ProfileStore& profiles,
                  const catalog::Catalog& catalog,
                  std::string_view target_id,
                  std::string& error);
        void cancel();

        const auth::DeviceCodeFlow& auth_flow() const;
        const storage::Settings& settings() const;
        storage::Settings& settings();
        const storage::ProfileStore& profiles() const;
        storage::ProfileStore& profiles();
        const download::DownloadQueue& downloads() const;
        runtime::SessionState session_state() const;
        const std::string& session_output() const;

    private:
        platform::IHttpClient& http_;
        platform::IFileSystem& files_;
        platform::IClock& clock_;
        platform::ILogger& logger_;
        auth::DeviceCodeFlow flow_;
        download::DownloadQueue downloads_;
        runtime::GameSession session_;
        storage::Settings settings_;
        storage::ProfileStore profiles_;
        std::string state_root_ = "state";

        std::string state_path(std::string_view name) const;
    };
}
