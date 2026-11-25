#pragma once

#include <chrono>
#include <string>
#include <string_view>

namespace raggedy::remote
{
    struct RemoteSession
    {
        std::string pin;
        bool active = false;
        std::chrono::steady_clock::time_point created{};
        int port = 0;
    };

    enum class RemoteOperationType
    {
        list_files,
        read_file,
        write_file,
        upload,
        download
    };

    bool valid_pin(std::string_view pin);

    RemoteSession create_session(std::string_view pin, int port, std::chrono::steady_clock::time_point now);

    bool session_active(const RemoteSession& session, std::chrono::steady_clock::time_point now, std::chrono::seconds lifetime);

    bool allowed_lan_host(std::string_view host);

    bool allowed_operation(const RemoteSession& session, RemoteOperationType operation, std::chrono::steady_clock::time_point now);

    std::string session_status(const RemoteSession& session, std::chrono::steady_clock::time_point now, std::chrono::seconds lifetime);
}
