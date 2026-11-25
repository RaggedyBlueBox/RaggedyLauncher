#pragma once

#include "raggedy/platform/interfaces.hpp"

#include <string>
#include <vector>

namespace raggedy::download
{
    enum class DownloadStatus
    {
        queued,
        downloading,
        verifying,
        complete,
        failed,
        cancelled
    };

    struct DownloadRequest
    {
        std::string url;
        std::string destination;
        std::string expected_sha256;
    };

    struct DownloadTask
    {
        DownloadRequest request;
        DownloadStatus status = DownloadStatus::queued;
        std::string error;
    };

    class DownloadQueue
    {
    public:
        DownloadQueue(platform::IHttpClient& http, platform::IFileSystem& files, platform::ICrypto& crypto);
        void enqueue(DownloadRequest request);
        void clear();
        bool run_one();
        void cancel_all();
        const std::vector<DownloadTask>& tasks() const;

    private:
        platform::IHttpClient& http_;
        platform::IFileSystem& files_;
        platform::ICrypto& crypto_;
        std::vector<DownloadTask> tasks_;
    };
}
