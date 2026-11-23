#pragma once

#include <string>
#include <vector>

#include "profiles.h"

// Data-driven performance renderers (Sodium family and tested alternatives).
// The matrix lives in config/performance-mods.json, packaged as
// runtime/performance-mods.json. A candidate is only auto-installed when its
// publisher metadata matches the exact target AND the matrix marks it proven
// on Xbox; unproven candidates are reported, never forced. Per-profile mode is
// "auto" (default) or "off". Managed files reuse the bundleOwners sidecar
// format with "perf:<project>" tokens, so removal is hash-checked and never
// touches user-owned jars. Only one managed renderer is kept per profile.

struct PerfCandidate {
    std::wstring project;
    bool proven = false;
    std::wstring note;
};

struct PerfSelection {
    bool hasCandidate = false;
    PerfCandidate candidate;
    std::wstring gap;
};

PerfSelection SelectPerformanceCandidate(
    const std::wstring& runtimeRoot, const LaunchTarget& target);
std::wstring PerfOwnerToken(const std::wstring& project);

// Synchronous core (runs on a worker thread via StartPerformanceJob).
bool ApplyPerformanceMode(const std::wstring& runtimeRoot, const std::wstring& profileId,
    const std::wstring& mode, std::wstring& status, bool& isError);
void StartPerformanceJob(
    const std::wstring& runtimeRoot, const std::wstring& profileId, const std::wstring& mode);
bool PollPerformanceJob(const std::wstring& profileId, std::wstring& status, bool& isError);
bool PerformanceJobPending(const std::wstring& profileId);

// Launch-time verification and logging only: never downloads, never deletes
// user files, never edits game options.
void LogPerformanceDecision(
    const std::wstring& runtimeRoot, const std::wstring& profileId, const LaunchTarget& target);
// Starts the auto job for a new profile when a proven candidate exists and is
// missing. Returns true when a job was started.
bool MaybeProvisionPerformanceForNewProfile(
    const std::wstring& runtimeRoot, const std::wstring& profileId);
