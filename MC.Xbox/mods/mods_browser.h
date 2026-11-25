#pragma once

#include <functional>
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

using DefaultPackProgress = std::function<void(const std::wstring& status, double fraction)>;

bool EnsureDefaultModpackInstalled(const std::wstring& runtimeRoot, const std::wstring& profileId,
    const LaunchTarget& target, std::wstring& error, const DefaultPackProgress& progress = nullptr);
std::vector<LaunchTarget> SupportedPackTargets(
    const std::wstring& runtimeRoot, const std::wstring& profileId);
std::wstring DefaultPackTitle(const std::wstring& profileId);
bool InstallManagedModrinthProject(const std::wstring& runtimeRoot, const std::wstring& profileId,
    const std::wstring& rootSlug, const std::wstring& ownerToken, const LaunchTarget& target,
    std::vector<std::wstring>& installedOut, std::wstring& error);
