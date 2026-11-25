#pragma once

#include <chrono>
#include <cstddef>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace raggedy::platform
{
    struct HttpRequest
    {
        std::string method;
        std::string url;
        std::map<std::string, std::string> headers;
        std::vector<std::byte> body;
    };

    struct HttpResponse
    {
        int status = 0;
        std::vector<std::byte> body;
        std::string error;
    };

    class IHttpClient
    {
    public:
        virtual ~IHttpClient() = default;
        virtual HttpResponse send(const HttpRequest& request) = 0;
    };

    class IFileSystem
    {
    public:
        virtual ~IFileSystem() = default;
        virtual bool read_text(const std::string& path, std::string& out) = 0;
        virtual bool write_text(const std::string& path, const std::string& text) = 0;
        virtual bool write_bytes(const std::string& path, const std::vector<std::byte>& bytes) = 0;
        virtual bool exists(const std::string& path) = 0;
        virtual bool create_directories(const std::string& path) = 0;
    };

    class ICrypto
    {
    public:
        virtual ~ICrypto() = default;
        virtual std::string sha256_hex(const std::vector<std::byte>& bytes) = 0;
    };

    class IProcessRunner
    {
    public:
        virtual ~IProcessRunner() = default;
        virtual int run(const std::vector<std::string>& arguments, std::string& output) = 0;
    };

    class ILogger
    {
    public:
        virtual ~ILogger() = default;
        virtual void log(std::string_view message) = 0;
    };

    class IClock
    {
    public:
        virtual ~IClock() = default;
        virtual std::chrono::steady_clock::time_point now() = 0;
    };
}
