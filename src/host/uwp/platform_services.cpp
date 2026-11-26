#include "pch.h"
#include "platform_services.hpp"

#include "raggedy/core/json.hpp"
#include "raggedy/diagnostics/report.hpp"

#include <winrt/Windows.Security.Cryptography.Core.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Web.Http.Headers.h>
#include <winrt/Windows.Web.Http.h>

#include <algorithm>
#include <cstdint>
#include <debugapi.h>
#include <string>
#include <string_view>
#include <vector>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Security::Cryptography;
using namespace Windows::Security::Cryptography::Core;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Web::Http;

namespace winrt::RaggedyLauncher::implementation
{
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

        std::string text_from(const Windows::Storage::Streams::IBuffer& buffer)
        {
            com_ptr<IBufferByteAccess> access;
            check_hresult(winrt::get_abi(buffer)->QueryInterface(__uuidof(IBufferByteAccess), reinterpret_cast<void**>(access.put())));
            std::uint8_t* raw = nullptr;
            check_hresult(access->Buffer(&raw));
            return std::string{reinterpret_cast<const char*>(raw), reinterpret_cast<const char*>(raw) + buffer.Length()};
        }

        Windows::Storage::Streams::IBuffer buffer_from(const std::vector<std::byte>& bytes)
        {
            return CryptographicBuffer::CreateFromByteArray({reinterpret_cast<const byte*>(bytes.data()), static_cast<uint32_t>(bytes.size())});
        }

        std::vector<std::wstring> split_path(std::string_view path)
        {
            std::string normalized{path};
            std::replace(normalized.begin(), normalized.end(), '\\', '/');
            std::vector<std::wstring> parts;
            std::size_t start = 0;
            while (start <= normalized.size())
            {
                const std::size_t slash = normalized.find('/', start);
                const std::string part = normalized.substr(start, slash == std::string::npos ? std::string::npos : slash - start);
                if (!part.empty() && part != ".")
                {
                    parts.push_back(to_hstring(part).c_str());
                }
                if (slash == std::string::npos)
                {
                    break;
                }
                start = slash + 1;
            }
            return parts;
        }

        StorageFolder open_folder(const std::vector<std::wstring>& parts, bool create)
        {
            StorageFolder folder = ApplicationData::Current().LocalFolder();
            for (const std::wstring& part : parts)
            {
                folder = create
                    ? folder.CreateFolderAsync(part, CreationCollisionOption::OpenIfExists).get()
                    : folder.GetFolderAsync(part).get();
            }
            return folder;
        }

        std::pair<StorageFolder, std::wstring> open_file_parent(const std::string& path, bool create)
        {
            std::vector<std::wstring> parts = split_path(path);
            if (parts.empty())
            {
                return {nullptr, L""};
            }
            const std::wstring name = parts.back();
            parts.pop_back();
            return {open_folder(parts, create), name};
        }

        raggedy::platform::HttpResponse failed_response(std::string error)
        {
            return {0, {}, std::move(error)};
        }
    }

    raggedy::platform::HttpResponse UwpHttpClient::send(const raggedy::platform::HttpRequest& request)
    {
        try
        {
            HttpRequestMessage message;
            message.Method(request.method == "GET" ? HttpMethod::Get() : HttpMethod::Post());
            message.RequestUri(Uri{to_hstring(request.url)});

            for (const auto& [key, value] : request.headers)
            {
                message.Headers().TryAppendWithoutValidation(to_hstring(key), to_hstring(value));
            }

            if (!request.body.empty())
            {
                HttpBufferContent content(buffer_from(request.body));
                for (const auto& [key, value] : request.headers)
                {
                    content.Headers().TryAppendWithoutValidation(to_hstring(key), to_hstring(value));
                }
                message.Content(content);
            }

            const HttpResponseMessage result = HttpClient{}.SendRequestAsync(message).get();
            raggedy::platform::HttpResponse response;
            response.status = static_cast<int>(result.StatusCode());
            if (result.Content() != nullptr)
            {
                const IBuffer buffer = result.Content().ReadAsBufferAsync().get();
                const std::string raw = text_from(buffer);
                response.body.assign(raw.begin(), raw.end());
            }
            return response;
        }
        catch (const hresult_error&)
        {
            return failed_response("Network request failed.");
        }
    }

    bool LocalStateFileSystem::read_text(const std::string& path, std::string& out)
    {
        try
        {
            const auto [folder, name] = open_file_parent(path, false);
            if (folder == nullptr || name.empty())
            {
                return false;
            }
            const StorageFile file = folder.GetFileAsync(name).get();
            out = to_string(FileIO::ReadTextAsync(file).get());
            return true;
        }
        catch (const hresult_error&)
        {
            return false;
        }
    }

    bool LocalStateFileSystem::write_text(const std::string& path, const std::string& text)
    {
        try
        {
            const auto [folder, name] = open_file_parent(path, true);
            if (folder == nullptr || name.empty())
            {
                return false;
            }
            const StorageFile file = folder.CreateFileAsync(name, CreationCollisionOption::ReplaceExisting).get();
            FileIO::WriteTextAsync(file, to_hstring(text)).get();
            return true;
        }
        catch (const hresult_error&)
        {
            return false;
        }
    }

    bool LocalStateFileSystem::write_bytes(const std::string& path, const std::vector<std::byte>& bytes)
    {
        try
        {
            const auto [folder, name] = open_file_parent(path, true);
            if (folder == nullptr || name.empty())
            {
                return false;
            }
            const StorageFile file = folder.CreateFileAsync(name, CreationCollisionOption::ReplaceExisting).get();
            const IRandomAccessStream stream = file.OpenAsync(FileAccessMode::ReadWrite).get();
            stream.Size(0);
            stream.Seek(0);
            DataWriter writer(stream);
            writer.WriteBytes({reinterpret_cast<const byte*>(bytes.data()), static_cast<uint32_t>(bytes.size())});
            writer.StoreAsync().get();
            writer.FlushAsync().get();
            return true;
        }
        catch (const hresult_error&)
        {
            return false;
        }
    }

    bool LocalStateFileSystem::exists(const std::string& path)
    {
        try
        {
            const auto [folder, name] = open_file_parent(path, false);
            if (folder == nullptr || name.empty())
            {
                return false;
            }
            folder.GetFileAsync(name).get();
            return true;
        }
        catch (const hresult_error&)
        {
            return false;
        }
    }

    bool LocalStateFileSystem::create_directories(const std::string& path)
    {
        try
        {
            open_folder(split_path(path), true);
            return true;
        }
        catch (const hresult_error&)
        {
            return false;
        }
    }

    std::string WindowsCrypto::sha256_hex(const std::vector<std::byte>& bytes)
    {
        HashAlgorithmProvider provider = HashAlgorithmProvider::OpenAlgorithm(L"SHA256");
        const IBuffer hash = provider.CreateHash(buffer_from(bytes));
        return to_string(CryptographicBuffer::ConvertBinaryToString(BinaryStringEncoding::Hex, hash));
    }

    BridgeProcessRunner::BridgeProcessRunner() = default;

    BridgeProcessRunner::BridgeProcessRunner(std::string bridge_url)
        : bridge_url_(std::move(bridge_url))
    {
    }

    void BridgeProcessRunner::set_bridge_url(std::string bridge_url)
    {
        bridge_url_ = std::move(bridge_url);
    }

    int BridgeProcessRunner::run(const std::vector<std::string>& arguments, std::string& output)
    {
        if (bridge_url_.empty())
        {
            output = "A process bridge URL is required to launch the game.";
            return 1;
        }

        std::vector<raggedy::core::JsonValue> encoded;
        encoded.reserve(arguments.size());
        for (const std::string& argument : arguments)
        {
            encoded.push_back(raggedy::core::JsonValue::from_string(argument));
        }
        const raggedy::core::JsonValue payload = raggedy::core::JsonValue::from_object({
            {"arguments", raggedy::core::JsonValue::from_array(std::move(encoded))}
        });

        try
        {
            HttpRequestMessage message;
            message.Method(HttpMethod::Post());
            message.RequestUri(Uri{to_hstring(bridge_url_)});
            HttpBufferContent content(buffer_from(bytes_from(raggedy::core::write_json(payload))));
            content.Headers().TryAppendWithoutValidation(L"Content-Type", L"application/json");
            message.Content(content);

            const HttpResponseMessage result = HttpClient{}.SendRequestAsync(message).get();
            const int status = static_cast<int>(result.StatusCode());
            if (status < 200 || status > 299)
            {
                output = "The process bridge rejected the launch request.";
                return 1;
            }

            const IBuffer buffer = result.Content().ReadAsBufferAsync().get();
            const std::string raw = text_from(buffer);
            raggedy::core::JsonValue value;
            if (!raggedy::core::parse_json(raw, value) || value.type != raggedy::core::JsonType::object)
            {
                output = "The process bridge returned an invalid response.";
                return 1;
            }

            const raggedy::core::JsonValue* code = value.get("exit_code");
            const raggedy::core::JsonValue* text = value.get("output");
            output = text != nullptr && text->type == raggedy::core::JsonType::string ? text->string : std::string{};
            return code != nullptr && code->type == raggedy::core::JsonType::number ? static_cast<int>(code->number) : 1;
        }
        catch (const hresult_error&)
        {
            output = "The process bridge could not be reached.";
            return 1;
        }
    }

    std::chrono::steady_clock::time_point SteadyClock::now()
    {
        return std::chrono::steady_clock::now();
    }

    void RedactingLogger::log(std::string_view message)
    {
        const std::string redacted = raggedy::diagnostics::redact_message(std::string{message});
        OutputDebugStringW(to_hstring(redacted).c_str());
    }
}
