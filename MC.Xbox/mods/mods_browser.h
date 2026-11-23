#pragma once

#include <string>
#include <vector>

#include <windows.ui.core.h>

#include "auth_ui_state.h"
#include "mods_page.h"
#include "profiles.h"

class AuthScreenRenderer;

int PurgeBlockedModsFromDir(const std::wstring& runtimeRoot, const std::wstring& modsDir);
bool IsBlockedModFileName(const std::wstring& fileName);

void ShowModsPage(
    ABI::Windows::UI::Core::ICoreWindow* window,
    AuthScreenRenderer* renderer,
    AuthUiState& state,
    const std::wstring& runtimeRoot);

// Applies a controller bundle to a profile on a worker thread started by
// StartControllerBundleJob; PollControllerBundleJob collects the result.
// ApplyControllerBundle is the synchronous core (also used by tests/tools).
bool ApplyControllerBundle(const std::wstring& runtimeRoot, const std::wstring& profileId,
    const std::wstring& bundle, std::wstring& chosenOut, std::wstring& status, bool& isError);
void StartControllerBundleJob(
    const std::wstring& runtimeRoot, const std::wstring& profileId, const std::wstring& bundle);
bool InstallManagedModrinthProject(const std::wstring& runtimeRoot, const std::wstring& profileId,
    const std::wstring& rootSlug, const std::wstring& ownerToken, const LaunchTarget& target,
    std::vector<std::wstring>& installedOut, std::wstring& error);
bool PollControllerBundleJob(const std::wstring& profileId, std::wstring& status, bool& isError, std::wstring& chosen);
bool ControllerBundleJobPending(const std::wstring& profileId);
