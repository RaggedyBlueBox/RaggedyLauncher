#include "performance_bundle.h"

#include "controller_bundle.h"
#include "i18n.h"
#include "launcher_common.h"
#include "mods_browser.h"
#include "runtime_manager.h"

#include <winrt/base.h>
#include <winrt/Windows.Data.Json.h>
#include <winrt/Windows.Foundation.h>

#include <mutex>
#include <set>
#include <thread>

namespace {

std::wstring PerfMatrixPath(const std::wstring& runtimeRoot) {
    const std::wstring packageDir = GetExecutableDir();
    const std::wstring candidates[] = {
        packageDir + L"\\runtime\\performance-mods.json",
        runtimeRoot + L"\\runtime\\performance-mods.json",
    };
    for (const std::wstring& path : candidates) {
        WIN32_FILE_ATTRIBUTE_DATA data = {};
        if (GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &data)) return path;
    }
    return candidates[0];
}

bool JarNameMatchesProject(const std::wstring& fileName, const std::wstring& project) {
    const std::wstring lower = ToLowerW(fileName);
    if (lower.size() <= 4 || lower.compare(lower.size() - 4, 4, L".jar") != 0) return false;
    return lower.rfind(ToLowerW(project), 0) == 0;
}

std::vector<std::wstring> FindPerfConflicts(
    const std::wstring& runtimeRoot, const std::wstring& profileId, const std::wstring& project) {
    const std::wstring modsDir = ProfileModsDir(runtimeRoot, profileId);
    static const wchar_t* kRendererPrefixes[] = { L"sodium", L"embeddium", L"rubidium", L"nothirium" };
    std::vector<std::wstring> conflicts;
    for (const std::wstring& name : ListProfileMods(runtimeRoot, profileId)) {
        std::wstring owners;
        if (ReadManagedMetaValue(modsDir, name, L"bundleOwners", owners) && !owners.empty()) continue;
        const std::wstring lower = ToLowerW(name);
        const bool isRenderer = [lower] {
            for (const wchar_t* prefix : kRendererPrefixes) {
                if (lower.rfind(prefix, 0) == 0 && lower.size() > 4 &&
                    lower.compare(lower.size() - 4, 4, L".jar") == 0) {
                    return true;
                }
            }
            return false;
        }();
        if (!isRenderer) continue;
        if (!project.empty() && JarNameMatchesProject(name, project)) continue;
        conflicts.push_back(name);
    }
    return conflicts;
}

}  // namespace

std::wstring PerfOwnerToken(const std::wstring& project) {
    return L"perf:" + ToLowerW(project);
}

PerfSelection SelectPerformanceCandidate(
    const std::wstring& runtimeRoot, const LaunchTarget& target) {
    PerfSelection selection;
    std::wstring body;
    if (!ReadTextFile(PerfMatrixPath(runtimeRoot), body)) {
        selection.gap = L"Performance matrix is missing from the install.";
        return selection;
    }
    try {
        using namespace winrt::Windows::Data::Json;
        JsonObject root = JsonObject::Parse(winrt::hstring(body.c_str()));
        if (!root.HasKey(L"targets")) return selection;
        JsonArray targets = root.GetNamedArray(L"targets");
        const std::wstring loader = ToLowerW(target.loader);
        for (uint32_t i = 0; i < targets.Size(); ++i) {
            if (targets.GetAt(i).ValueType() != JsonValueType::Object) continue;
            JsonObject entry = targets.GetAt(i).GetObject();
            std::wstring mc, ld;
            if (entry.HasKey(L"minecraftVersion")) mc = entry.GetNamedString(L"minecraftVersion").c_str();
            if (entry.HasKey(L"loader")) ld = ToLowerW(std::wstring(entry.GetNamedString(L"loader").c_str()));
            if (mc != target.minecraftVersion || ld != loader) continue;
            if (entry.HasKey(L"candidates") &&
                entry.GetNamedValue(L"candidates").ValueType() == JsonValueType::Array) {
                JsonArray candidates = entry.GetNamedArray(L"candidates");
                if (candidates.Size() > 0 && candidates.GetAt(0).ValueType() == JsonValueType::Object) {
                    JsonObject candidate = candidates.GetAt(0).GetObject();
                    selection.hasCandidate = true;
                    if (candidate.HasKey(L"project")) {
                        selection.candidate.project = candidate.GetNamedString(L"project").c_str();
                    }
                    if (candidate.HasKey(L"proven")) {
                        selection.candidate.proven =
                            candidate.GetNamedValue(L"proven").ValueType() == JsonValueType::Boolean &&
                            candidate.GetNamedBoolean(L"proven");
                    }
                    if (candidate.HasKey(L"note")) {
                        selection.candidate.note = candidate.GetNamedString(L"note").c_str();
                    }
                    return selection;
                }
            }
            if (entry.HasKey(L"gap")) selection.gap = entry.GetNamedString(L"gap").c_str();
            return selection;
        }
        selection.gap = L"This version and loader has no performance matrix entry.";
    } catch (...) {
        selection.gap = L"Performance matrix could not be read.";
    }
    return selection;
}

bool ApplyPerformanceMode(const std::wstring& runtimeRoot, const std::wstring& profileId,
    const std::wstring& mode, std::wstring& status, bool& isError) {
    isError = false;
    const Profile profile = GetProfileById(runtimeRoot, profileId);
    if (profile.builtin || profileId == kVanillaProfileId) {
        status = Tr(L"Vanilla is read-only. Create a profile to change performance mods.");
        isError = true;
        return false;
    }
    const std::wstring want = ToLowerW(mode) == L"off" ? L"off" : L"auto";
    const LaunchTarget target = ResolveProfileTarget(runtimeRoot, profile);
    const PerfSelection selection = SelectPerformanceCandidate(runtimeRoot, target);
    int removed = 0;
    for (const std::wstring& token : ManagedOwnerTokensWithPrefix(runtimeRoot, profileId, L"perf:")) {
        if (want == L"auto" && selection.hasCandidate && token == PerfOwnerToken(selection.candidate.project)) {
            continue;
        }
        removed += RemoveManagedFiles(runtimeRoot, profileId, token);
    }
    if (want == L"off") {
        if (!SetProfilePerfRenderer(runtimeRoot, profileId, want)) {
            status = Tr(L"Could not save performance setting");
            isError = true;
            return false;
        }
        status = removed > 0
            ? Tr(L"Performance renderer off; managed files removed, your mods were left in place.")
            : Tr(L"Performance renderer off.");
        return true;
    }
    // want == auto
    if (!selection.hasCandidate) {
        SetProfilePerfRenderer(runtimeRoot, profileId, want);
        status = Tr(L"No performance renderer for this target: ") +
            (selection.gap.empty() ? Tr(L"the game runs without one.") : selection.gap);
        return true;
    }
    const std::wstring token = PerfOwnerToken(selection.candidate.project);
    if (!selection.candidate.proven) {
        SetProfilePerfRenderer(runtimeRoot, profileId, want);
        status = TrFormat(L"{0} matches this target but is untested on Xbox, so it was not auto-installed. Install it from Mods if you want to try it.",
            { selection.candidate.project });
        return true;
    }
    if (ManagedFilesPresent(runtimeRoot, profileId, token)) {
        SetProfilePerfRenderer(runtimeRoot, profileId, want);
        status = TrFormat(L"Performance renderer {0} is already installed.", { selection.candidate.project });
        return true;
    }
    std::vector<std::wstring> installed;
    std::wstring error;
    if (!InstallManagedModrinthProject(runtimeRoot, profileId, selection.candidate.project, token,
            target, installed, error)) {
        status = error.empty() ? Tr(L"Performance renderer install failed.") : error;
        isError = true;
        return false;
    }
    if (!SetProfilePerfRenderer(runtimeRoot, profileId, want)) {
        status = Tr(L"Could not save performance setting");
        isError = true;
        return false;
    }
    status = TrFormat(L"Performance renderer {0} installed ({1} file(s)).",
        { selection.candidate.project, std::to_wstring(installed.size()) });
    const std::vector<std::wstring> conflicts = FindPerfConflicts(runtimeRoot, profileId, selection.candidate.project);
    if (!conflicts.empty()) {
        status += L" " + TrFormat(L"Unmanaged {0} is also installed; keep only one renderer to avoid conflicts.",
            { conflicts.front() });
    }
    return true;
}

namespace {
struct PerfJobState {
    std::mutex mutex;
    bool running = false;
    bool ready = false;
    bool error = false;
    std::wstring profileId;
    std::wstring status;
};
PerfJobState g_perfJob;
}  // namespace

void StartPerformanceJob(
    const std::wstring& runtimeRoot, const std::wstring& profileId, const std::wstring& mode) {
    std::lock_guard<std::mutex> lk(g_perfJob.mutex);
    if (g_perfJob.running) return;
    g_perfJob.running = true;
    g_perfJob.ready = false;
    g_perfJob.profileId = profileId;
    const std::wstring rootCopy = runtimeRoot;
    const std::wstring profileCopy = profileId;
    const std::wstring modeCopy = mode;
    std::thread([rootCopy, profileCopy, modeCopy]() {
        std::wstring status;
        bool isError = false;
        ApplyPerformanceMode(rootCopy, profileCopy, modeCopy, status, isError);
        std::lock_guard<std::mutex> inner(g_perfJob.mutex);
        g_perfJob.status = status;
        g_perfJob.error = isError;
        g_perfJob.ready = true;
        g_perfJob.running = false;
    }).detach();
}

bool PollPerformanceJob(const std::wstring& profileId, std::wstring& status, bool& isError) {
    std::lock_guard<std::mutex> lk(g_perfJob.mutex);
    if (g_perfJob.running || !g_perfJob.ready) return false;
    if (g_perfJob.profileId != profileId) return false;
    g_perfJob.ready = false;
    status = g_perfJob.status;
    isError = g_perfJob.error;
    return true;
}

bool PerformanceJobPending(const std::wstring& profileId) {
    std::lock_guard<std::mutex> lk(g_perfJob.mutex);
    return (g_perfJob.running || g_perfJob.ready) && g_perfJob.profileId == profileId;
}

bool MaybeProvisionPerformanceForNewProfile(
    const std::wstring& runtimeRoot, const std::wstring& profileId) {
    const Profile profile = GetProfileById(runtimeRoot, profileId);
    if (profile.builtin || profileId == kVanillaProfileId) return false;
    if (NormalizePerfRenderer(profile.perfRenderer) != L"auto") return false;
    const LaunchTarget target = ResolveProfileTarget(runtimeRoot, profile);
    const PerfSelection selection = SelectPerformanceCandidate(runtimeRoot, target);
    if (!selection.hasCandidate || !selection.candidate.proven) return false;
    if (ManagedFilesPresent(runtimeRoot, profileId, PerfOwnerToken(selection.candidate.project))) {
        return false;
    }
    StartPerformanceJob(runtimeRoot, profileId, L"auto");
    return true;
}

void LogPerformanceDecision(
    const std::wstring& runtimeRoot, const std::wstring& profileId, const LaunchTarget& target) {
    const Profile profile = GetProfileById(runtimeRoot, profileId);
    const PerfSelection selection = SelectPerformanceCandidate(runtimeRoot, target);
    std::wstring detail;
    if (profile.perfRenderer == L"off") {
        detail = L"mode=off";
    } else if (!selection.hasCandidate) {
        detail = L"mode=auto no-candidate gap=" +
            (selection.gap.empty() ? L"unknown" : selection.gap);
    } else {
        const std::wstring token = PerfOwnerToken(selection.candidate.project);
        const bool installed = ManagedFilesPresent(runtimeRoot, profileId, token);
        detail = L"mode=auto candidate=" + selection.candidate.project +
            L" proven=" + std::wstring(selection.candidate.proven ? L"yes" : L"no") +
            L" installed=" + std::wstring(installed ? L"yes" : L"no");
        if (!selection.candidate.proven) {
            detail += L" (not auto-installed: untested on Xbox)";
        } else if (!installed) {
            detail += L" (missing: enable it in Settings while online; the game runs without it)";
        }
    }
    WriteLogF(L"performance renderer %s target=%s", detail.c_str(), target.targetId.c_str());
    const std::vector<std::wstring> conflicts =
        FindPerfConflicts(runtimeRoot, profileId, selection.hasCandidate ? selection.candidate.project : L"");
    for (const std::wstring& name : conflicts) {
        WriteLogF(L"Unmanaged renderer mod %s present alongside managed selection; left in place for the user to resolve",
            name.c_str());
    }
}
