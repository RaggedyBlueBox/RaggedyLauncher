#include "raggedy/app/launcher.hpp"
#include "raggedy/auth/device_code.hpp"
#include "raggedy/catalog/built_in.hpp"
#include "raggedy/controller/navigation.hpp"
#include "raggedy/core/json.hpp"
#include "raggedy/diagnostics/report.hpp"
#include "raggedy/download/queue.hpp"
#include "raggedy/mods/mod_plan.hpp"
#include "raggedy/platform/interfaces.hpp"
#include "raggedy/remote/session.hpp"
#include "raggedy/runtime/launch_plan_builder.hpp"
#include "raggedy/runtime/session.hpp"
#include "raggedy/storage/profile_store.hpp"
#include "raggedy/storage/settings.hpp"
#include "raggedy/ui/screen_models.hpp"

#include <cassert>
#include <chrono>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace
{
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

    class FakeHttp : public raggedy::platform::IHttpClient
    {
    public:
        int status = 200;
        std::vector<std::byte> body;
        std::string error;

        raggedy::platform::HttpResponse send(const raggedy::platform::HttpRequest& request) override
        {
            last_method = request.method;
            last_url = request.url;
            return {status, body, error};
        }

        std::string last_method;
        std::string last_url;
    };

    class FakeFiles : public raggedy::platform::IFileSystem
    {
    public:
        bool result = true;
        std::string written_path;
        std::string written_text;
        std::vector<std::byte> written_bytes;
        std::map<std::string, std::string> texts;

        bool read_text(const std::string& path, std::string& out) override
        {
            const auto item = texts.find(path);
            if (item == texts.end())
            {
                return false;
            }
            out = item->second;
            return true;
        }

        bool write_text(const std::string& path, const std::string& text) override
        {
            written_path = path;
            written_text = text;
            return result;
        }

        bool write_bytes(const std::string& path, const std::vector<std::byte>& bytes) override
        {
            written_path = path;
            written_bytes = bytes;
            return result;
        }

        bool exists(const std::string&) override
        {
            return false;
        }

        bool create_directories(const std::string&) override
        {
            return true;
        }
    };

    class FakeCrypto : public raggedy::platform::ICrypto
    {
    public:
        std::string hash = "expected-hash";

        std::string sha256_hex(const std::vector<std::byte>&) override
        {
            return hash;
        }
    };

    class FakeRunner : public raggedy::platform::IProcessRunner
    {
    public:
        int code = 0;
        std::vector<std::string> arguments;
        std::string output = "game output";

        int run(const std::vector<std::string>& args, std::string& out) override
        {
            arguments = args;
            out = output;
            return code;
        }
    };

    class FakeSequenceHttp : public raggedy::platform::IHttpClient
    {
    public:
        std::vector<raggedy::platform::HttpResponse> responses;
        std::vector<raggedy::platform::HttpRequest> requests;
        std::size_t index = 0;

        raggedy::platform::HttpResponse send(const raggedy::platform::HttpRequest& request) override
        {
            requests.push_back(request);
            if (index >= responses.size())
            {
                return {500, {}, "missing response"};
            }
            return responses[index++];
        }
    };

    class FakeClock : public raggedy::platform::IClock
    {
    public:
        std::chrono::steady_clock::time_point value = std::chrono::steady_clock::now();

        std::chrono::steady_clock::time_point now() override
        {
            return value;
        }
    };

    class FakeLogger : public raggedy::platform::ILogger
    {
    public:
        std::vector<std::string> messages;

        void log(std::string_view message) override
        {
            messages.emplace_back(message);
        }
    };
}

int main()
{
    raggedy::core::JsonValue parsed;
    assert(raggedy::core::parse_json(R"({"a":[1,true,"x"],"b":null})", parsed));
    assert(parsed.get("a") != nullptr);
    assert(parsed.get("a")->at(0).as_number() == 1.0);
    assert(parsed.get("a")->at(1).as_bool() == true);
    assert(parsed.get("a")->at(2).as_string() == "x");
    assert(parsed.get("b")->type == raggedy::core::JsonType::null_value);
    const std::string json_text = raggedy::core::write_json(parsed);
    raggedy::core::JsonValue reparsed;
    assert(raggedy::core::parse_json(json_text, reparsed));
    assert(reparsed.get("a")->size() == 3);

    auto settings = raggedy::storage::default_settings();
    settings.selected_profile_id = "simply-legacy";
    settings.microsoft_client_id = "65f4d123-aaaa-bbbb-cccc-3333cccc4444";
    settings.process_bridge_url = "http://127.0.0.1:4137/launch";
    settings.telemetry_enabled = true;
    settings.ui_fps_cap = 45;
    assert(raggedy::storage::valid_settings(settings));
    const auto reloaded_settings = raggedy::storage::settings_from_json(raggedy::storage::settings_to_json(settings));
    assert(reloaded_settings.selected_profile_id == "simply-legacy");
    assert(reloaded_settings.microsoft_client_id == settings.microsoft_client_id);
    assert(reloaded_settings.process_bridge_url == settings.process_bridge_url);
    assert(reloaded_settings.telemetry_enabled);
    assert(reloaded_settings.ui_fps_cap == 45);
    settings.ui_fps_cap = 15;
    assert(!raggedy::storage::valid_settings(settings));
    settings.ui_fps_cap = 45;
    settings.microsoft_client_id = "not a client id";
    assert(!raggedy::storage::valid_settings(settings));
    settings.microsoft_client_id = "65f4d123-aaaa-bbbb-cccc-3333cccc4444";
    settings.process_bridge_url = "http://example.invalid/launch";
    assert(!raggedy::storage::valid_settings(settings));

    auto profiles = raggedy::storage::default_profile_store();
    assert(raggedy::storage::valid_profile_store(profiles));
    assert(raggedy::storage::set_active_profile(profiles, "re-console"));
    assert(profiles.active_profile_id == "re-console");
    assert(raggedy::storage::find_profile(profiles, "re-console") != nullptr);
    assert(raggedy::storage::remove_profile(profiles, "re-console"));
    assert(profiles.active_profile_id.empty());
    const auto reloaded_profiles = raggedy::storage::profile_store_from_json(raggedy::storage::profile_store_to_json(raggedy::storage::default_profile_store()));
    assert(raggedy::storage::valid_profile_store(reloaded_profiles));
    assert(reloaded_profiles.profiles.size() == 2);

    const auto catalog = raggedy::catalog::make_built_in_catalog();
    assert(catalog.valid());
    assert(catalog.entries_for("simply-legacy", true).size() == 3);
    assert(catalog.entries_for("re-console", true).size() == 13);
    assert(catalog.find("sl-26-3") != nullptr);
    assert(catalog.find("rc-1-20-1")->java_major == 17);

    FakeHttp http;
    http.body = bytes_from("payload");
    FakeFiles files;
    FakeCrypto crypto;
    raggedy::download::DownloadQueue queue(http, files, crypto);
    queue.enqueue({"https://example.invalid/file", "file.txt", "expected-hash"});
    assert(queue.run_one());
    assert(queue.tasks().size() == 1);
    assert(queue.tasks()[0].status == raggedy::download::DownloadStatus::complete);
    assert(files.written_bytes == bytes_from("payload"));
    assert(http.last_method == "GET");
    queue.enqueue({"https://example.invalid/bad", "bad.txt", "wrong-hash"});
    assert(!queue.run_one());
    assert(queue.tasks()[1].status == raggedy::download::DownloadStatus::failed);

    raggedy::runtime::PlanInput plan_input;
    plan_input.java_executable = "java";
    plan_input.class_path = {"game.jar", "loader.jar"};
    plan_input.jvm_args = {"-Xmx2G"};
    plan_input.main_class = "net.example.Main";
    plan_input.game_args = {"--version", "26.3"};
    plan_input.native_directory = "natives";
    raggedy::core::LaunchPlan plan;
    std::string error;
    assert(raggedy::runtime::build_launch_plan(plan_input, plan, error));
    assert(plan.arguments.front() == "java");
    assert(plan.arguments[1] == "-Xmx2G");
    assert(plan.arguments[2] == "-Djava.library.path=natives");
    assert(plan.arguments[3] == "-cp");
    assert(plan.arguments.back() == "26.3");

    FakeRunner runner;
    raggedy::runtime::GameSession game(runner);
    raggedy::runtime::SessionRequest request;
    request.plan = plan;
    assert(!game.start(request, error));
    assert(game.state() == raggedy::runtime::SessionState::failed);
    request.authenticated = true;
    request.entitlement_verified = true;
    assert(game.start(request, error));
    assert(game.state() == raggedy::runtime::SessionState::exited);
    assert(runner.arguments == plan.arguments);
    assert(game.output() == "game output");

    const raggedy::mods::ModPlan mod_plan = raggedy::mods::build_managed_mod_plan(
        "simply-legacy", "26.3", "Fabric",
        {
            {"mod-one", "1.0", raggedy::mods::ModSource::modrinth, false},
            {"mod-two", "2.0", raggedy::mods::ModSource::curseforge, false}
        });
    assert(mod_plan.valid);
    assert(mod_plan.steps.size() == 2);
    assert(mod_plan.steps[0].action == "download");
    assert(mod_plan.steps[1].action == "needs-user-key");
    assert(raggedy::mods::source_name(raggedy::mods::ModSource::modrinth) == "Modrinth");

    raggedy::controller::NavigationState nav;
    auto moved = raggedy::controller::apply_input(nav, raggedy::controller::InputAction::right, false, 2, 5);
    assert(moved.changed && moved.state.pack_index == 1);
    moved = raggedy::controller::apply_input(moved.state, raggedy::controller::InputAction::down, false, 2, 5);
    assert(moved.state.region == raggedy::controller::FocusRegion::version_rows);
    moved = raggedy::controller::apply_input(moved.state, raggedy::controller::InputAction::down, true, 2, 5);
    assert(moved.state.row_index == 1);
    moved = raggedy::controller::apply_input(moved.state, raggedy::controller::InputAction::confirm, false, 2, 5);
    assert(moved.confirm);
    moved = raggedy::controller::apply_input(moved.state, raggedy::controller::InputAction::back, false, 2, 5);
    assert(moved.go_back && moved.state.region == raggedy::controller::FocusRegion::pack_tabs);
    assert(raggedy::controller::repeat_delay().count() == 300);
    assert(raggedy::controller::repeat_interval().count() == 100);

    const auto menu = raggedy::ui::make_main_menu_model(false, false, false);
    assert(menu.actions.front() == L"Sign in");
    const auto version_model = raggedy::ui::make_version_screen_model(
        catalog.entries_for("simply-legacy", true), "sl-26-3", 0, 0, true);
    assert(version_model.packs.front() == L"Simply Legacy");
    assert(!version_model.versions.empty());
    assert(version_model.status.find(L"Downloads") != std::wstring::npos);
    const auto credits_model = raggedy::ui::make_credits_screen_model();
    assert(!credits_model.lines.empty());

    const std::string redacted = raggedy::diagnostics::redact_message(
        "token=secret-12345 mail test@example.com host 192.168.1.4 code ABCD-1234");
    assert(redacted.find("secret-12345") == std::string::npos);
    assert(redacted.find("test@example.com") == std::string::npos);
    assert(redacted.find("192.168.1.4") == std::string::npos);
    assert(redacted.find("ABCD-1234") == std::string::npos);
    auto report = raggedy::diagnostics::make_report("build-7", true, {{"info", redacted}});
    assert(raggedy::diagnostics::safe_to_send(report));
    report.telemetry_opt_in = false;
    assert(!raggedy::diagnostics::safe_to_send(report));

    assert(raggedy::remote::valid_pin("123456"));
    assert(!raggedy::remote::valid_pin("abc123"));
    const auto now = std::chrono::steady_clock::now();
    const auto remote = raggedy::remote::create_session("123456", 8080, now);
    assert(remote.active);
    assert(raggedy::remote::session_active(remote, now, std::chrono::minutes{15}));
    assert(!raggedy::remote::session_active(remote, now + std::chrono::minutes{20}, std::chrono::minutes{15}));
    assert(raggedy::remote::allowed_lan_host("192.168.1.5"));
    assert(raggedy::remote::allowed_lan_host("xbox.local"));
    assert(!raggedy::remote::allowed_lan_host("8.8.8.8"));
    assert(raggedy::remote::allowed_operation(remote, raggedy::remote::RemoteOperationType::read_file, now));

    FakeSequenceHttp auth_http;
    auth_http.responses = {
        {200, bytes_from(R"({"device_code":"device","user_code":"abcd1234","verification_uri":"https://example.invalid/device","expires_in":600,"interval":1})"), {}},
        {400, bytes_from(R"({"error":"authorization_pending"})"), {}},
        {200, bytes_from(R"({"access_token":"ms-access","refresh_token":"ms-refresh"})"), {}},
        {200, bytes_from(R"({"Token":"xbl-token","DisplayClaims":{"xui":[{"uhs":"user-hash"}]}})"), {}},
        {200, bytes_from(R"({"Token":"xsts-token","DisplayClaims":{"xui":[{"xuid":"xuid-one"}]}})"), {}},
        {200, bytes_from(R"({"access_token":"mc-access"})"), {}},
        {200, bytes_from(R"({"items":[{"name":"game","kind":"minecraft"}]})"), {}},
        {200, bytes_from(R"({"id":"profile-id","name":"TestPlayer"})"), {}}
    };
    FakeClock auth_clock;
    raggedy::auth::DeviceCodeFlow flow;
    std::string auth_error;
    const raggedy::auth::AuthConfig auth_config{"client-id", "common", "XboxLive.signin openid profile offline_access"};
    assert(raggedy::auth::request_device_code(auth_http, auth_clock, auth_config, flow, auth_error));
    assert(flow.session.stage == raggedy::auth::Stage::waiting);
    assert(flow.session.challenge.user_code == "abcd1234");
    auth_clock.value += std::chrono::seconds{2};
    assert(raggedy::auth::poll_device_code(auth_http, auth_clock, flow, auth_error) == raggedy::auth::PollResult::pending);
    auth_clock.value += std::chrono::seconds{2};
    assert(raggedy::auth::poll_device_code(auth_http, auth_clock, flow, auth_error) == raggedy::auth::PollResult::success);
    assert(flow.session.stage == raggedy::auth::Stage::verifying);
    assert(raggedy::auth::verify_entitlement(auth_http, flow, auth_error));
    assert(raggedy::auth::can_launch(flow));
    assert(flow.tokens.account_id == "profile-id");
    assert(flow.tokens.display_name == "TestPlayer");
    assert(flow.tokens.xuid == "xuid-one");
    assert(flow.session.detail.empty());
    assert(auth_http.requests[0].method == "POST");
    assert(auth_http.requests[6].headers.at("Authorization") == "Bearer mc-access");
    raggedy::auth::sign_out(flow);
    assert(!raggedy::auth::can_launch(flow));
    assert(flow.session.stage == raggedy::auth::Stage::cancelled);
    assert(flow.tokens.minecraft_access_token.empty());

    FakeSequenceHttp denied_http;
    denied_http.responses = {
        {200, bytes_from(R"({"device_code":"device","user_code":"abcd1234","verification_uri":"https://example.invalid/device","expires_in":600,"interval":1})"), {}},
        {400, bytes_from(R"({"error":"authorization_declined"})"), {}}
    };
    FakeClock denied_clock;
    raggedy::auth::DeviceCodeFlow denied_flow;
    assert(raggedy::auth::request_device_code(denied_http, denied_clock, auth_config, denied_flow, auth_error));
    denied_clock.value += std::chrono::seconds{2};
    assert(raggedy::auth::poll_device_code(denied_http, denied_clock, denied_flow, auth_error) == raggedy::auth::PollResult::denied);
    assert(denied_flow.session.stage == raggedy::auth::Stage::failed);
    assert(auth_error.find("declined") != std::string::npos);

    FakeSequenceHttp launcher_http;
    launcher_http.responses = {
        {200, bytes_from(R"({"device_code":"device","user_code":"abcd1234","verification_uri":"https://example.invalid/device","expires_in":600,"interval":1})"), {}},
        {200, bytes_from(R"({"access_token":"ms-access","refresh_token":"ms-refresh"})"), {}},
        {200, bytes_from(R"({"Token":"xbl-token","DisplayClaims":{"xui":[{"uhs":"user-hash"}]}})"), {}},
        {200, bytes_from(R"({"Token":"xsts-token","DisplayClaims":{"xui":[{"xuid":"xuid-one"}]}})"), {}},
        {200, bytes_from(R"({"access_token":"mc-access"})"), {}},
        {200, bytes_from(R"({"items":[{"name":"game","kind":"minecraft"}]})"), {}},
        {200, bytes_from(R"({"id":"profile-id","name":"TestPlayer"})"), {}},
        {200, bytes_from("payload"), {}}
    };
    FakeFiles launcher_files;
    launcher_files.texts["state/manifests/sl-26-3.json"] = R"({"id":"sl-26-3","java_executable":"java","class_path":["game.jar"],"jvm_args":["-Xmx2G"],"main_class":"net.minecraft.client.main.Main","game_args":["--version","26.3"],"native_directory":"natives","required_directories":["versions/sl-26-3"],"downloads":[{"url":"https://example.invalid/game.jar","destination":"versions/sl-26-3/game.jar","expected_sha256":"0000000000000000000000000000000000000000000000000000000000000000"}]})";
    FakeCrypto launcher_crypto;
    launcher_crypto.hash = "0000000000000000000000000000000000000000000000000000000000000000";
    FakeRunner launcher_runner;
    FakeClock launcher_clock;
    FakeLogger logger;
    raggedy::app::Launcher launcher(launcher_http, launcher_files, launcher_crypto, launcher_runner, launcher_clock, logger);
    assert(launcher.set_state_root("state"));
    launcher.settings().microsoft_client_id = "client-id";
    assert(launcher.begin_sign_in(auth_error));
    launcher_clock.value += std::chrono::seconds{2};
    assert(launcher.poll_sign_in(auth_error) == raggedy::auth::PollResult::success);
    assert(launcher.play(raggedy::storage::default_profile_store(), catalog, "sl-26-3", auth_error));
    assert(launcher_runner.arguments.front() == "java");
    assert(launcher.session_state() == raggedy::runtime::SessionState::exited);
    assert(launcher_files.written_bytes == bytes_from("payload"));
    assert(launcher.downloads().tasks().size() == 1);
    assert(launcher.downloads().tasks().front().status == raggedy::download::DownloadStatus::complete);
    for (const std::string& message : logger.messages)
    {
        assert(message.find("ms-access") == std::string::npos);
        assert(message.find("mc-access") == std::string::npos);
    }

    return 0;
}
