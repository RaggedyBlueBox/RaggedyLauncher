#include "raggedy/download/queue.hpp"

#include <cstddef>
#include <string>
#include <utility>

namespace raggedy::download
{
    namespace
    {
        std::string parent_path(const std::string& path)
        {
            const std::size_t slash = path.find_last_of("/\\");
            return slash == std::string::npos ? std::string{} : path.substr(0, slash);
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
    }

    DownloadQueue::DownloadQueue(platform::IHttpClient& http, platform::IFileSystem& files, platform::ICrypto& crypto)
        : http_(http), files_(files), crypto_(crypto)
    {
    }

    void DownloadQueue::enqueue(DownloadRequest request)
    {
        tasks_.push_back(DownloadTask{std::move(request), DownloadStatus::queued, {}});
    }

    void DownloadQueue::clear()
    {
        tasks_.clear();
    }

    bool DownloadQueue::run_one()
    {
        for (DownloadTask& task : tasks_)
        {
            if (task.status != DownloadStatus::queued)
            {
                continue;
            }

            task.status = DownloadStatus::downloading;
            task.error.clear();

            platform::HttpRequest request;
            request.method = "GET";
            request.url = task.request.url;
            const platform::HttpResponse response = http_.send(request);

            if (response.status < 200 || response.status > 299)
            {
                task.status = DownloadStatus::failed;
                task.error = "HTTP request failed with status " + std::to_string(response.status);
                if (!response.error.empty())
                {
                    task.error += ": " + response.error;
                }
                return false;
            }

            task.status = DownloadStatus::verifying;
            if (!task.request.expected_sha256.empty())
            {
                const std::string actual = lower_ascii(crypto_.sha256_hex(response.body));
                if (actual != lower_ascii(task.request.expected_sha256))
                {
                    task.status = DownloadStatus::failed;
                    task.error = "SHA-256 checksum mismatch for " + task.request.destination;
                    return false;
                }
            }

            const std::string directory = parent_path(task.request.destination);
            if (!directory.empty() && !files_.create_directories(directory))
            {
                task.status = DownloadStatus::failed;
                task.error = "Failed to create directory " + directory;
                return false;
            }

            if (!files_.write_bytes(task.request.destination, response.body))
            {
                task.status = DownloadStatus::failed;
                task.error = "Failed to write " + task.request.destination;
                return false;
            }

            task.status = DownloadStatus::complete;
            task.error.clear();
            return true;
        }

        return false;
    }

    void DownloadQueue::cancel_all()
    {
        for (DownloadTask& task : tasks_)
        {
            if (task.status == DownloadStatus::queued
                || task.status == DownloadStatus::downloading
                || task.status == DownloadStatus::verifying)
            {
                task.status = DownloadStatus::cancelled;
                task.error = "Cancelled";
            }
        }
    }

    const std::vector<DownloadTask>& DownloadQueue::tasks() const
    {
        return tasks_;
    }
}
