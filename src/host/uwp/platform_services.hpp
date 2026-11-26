#pragma once

#include "raggedy/platform/interfaces.hpp"

#include <chrono>
#include <cstdint>
#include <string>
#include <unknwn.h>
#include <vector>

namespace winrt::RaggedyLauncher::implementation
{
    struct __declspec(uuid("905a0fef-bc53-11df-8c49-001e4fc686da")) IBufferByteAccess : ::IUnknown
    {
        virtual HRESULT __stdcall Buffer(std::uint8_t** value) = 0;
    };

    class UwpHttpClient final : public raggedy::platform::IHttpClient
    {
    public:
        raggedy::platform::HttpResponse send(const raggedy::platform::HttpRequest& request) override;
    };

    class LocalStateFileSystem final : public raggedy::platform::IFileSystem
    {
    public:
        bool read_text(const std::string& path, std::string& out) override;
        bool write_text(const std::string& path, const std::string& text) override;
        bool write_bytes(const std::string& path, const std::vector<std::byte>& bytes) override;
        bool exists(const std::string& path) override;
        bool create_directories(const std::string& path) override;
    };

    class WindowsCrypto final : public raggedy::platform::ICrypto
    {
    public:
        std::string sha256_hex(const std::vector<std::byte>& bytes) override;
    };

    class BridgeProcessRunner final : public raggedy::platform::IProcessRunner
    {
    public:
        BridgeProcessRunner();
        explicit BridgeProcessRunner(std::string bridge_url);
        void set_bridge_url(std::string bridge_url);
        int run(const std::vector<std::string>& arguments, std::string& output) override;

    private:
        std::string bridge_url_;
    };

    class SteadyClock final : public raggedy::platform::IClock
    {
    public:
        std::chrono::steady_clock::time_point now() override;
    };

    class RedactingLogger final : public raggedy::platform::ILogger
    {
    public:
        void log(std::string_view message) override;
    };
}
