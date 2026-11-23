#pragma once

#include <string>
#include <vector>

extern const wchar_t kVanillaProfileId[];

struct LaunchTarget {
    std::wstring targetId;
    std::wstring displayName;
    std::wstring minecraftVersion;
    std::wstring loader;
    std::wstring loaderVersion;
    std::wstring javaRuntime;
    std::wstring supportLevel;
    std::wstring notes;
    std::wstring controllerProvider;
};

struct Profile {
    std::wstring id;
    std::wstring name;
    std::wstring minecraftVersion;
    std::wstring loader;
    std::wstring loaderVersion;
    std::wstring targetId;
    bool builtin = false;
    bool controllerModEnabled = true;
    // Controller bundle selection: L"auto" (default), L"legacy4j", L"raggedy",
    // L"controlify" or L"none". "auto" resolves per target to Legacy4J when its
    // build exists for the profile's version, otherwise falls back to the best
    // available bundle for that version (Raggedy, then Controlify, then None).
    // controllerModEnabled is kept for migration only: profiles written before
    // bundles existed map true->raggedy and false->none.
    std::wstring controllerBundle = L"auto";
    // Performance renderer mode: L"auto" (default) installs the proven matrix
    // candidate for the profile target; L"off" removes managed files and
    // installs nothing. Unproven candidates are never auto-installed.
    std::wstring perfRenderer = L"auto";
};

// Canonical controller bundle ids. "auto" is the default for new profiles.
std::wstring DefaultControllerBundle();
bool IsKnownControllerBundle(const std::wstring& bundle);
std::wstring NormalizeControllerBundle(const std::wstring& bundle);
bool ControllerBundleUsesRaggedy(const std::wstring& bundle);
// Static per-target availability: whether the bundle can work on this target
// at all (publisher build exists, bundled controller shipped, loader supported).
// reason receives a short human-readable explanation when it returns false.
bool ControllerBundleSupportedForTarget(
    const std::wstring& bundle, const LaunchTarget& target, std::wstring& reason);
// Effective bundle for launch: resolves "auto" to the first available bundle
// for the target in Legacy4J, Raggedy, Controlify, None order. Any other value
// normalizes to itself when supported, otherwise falls back to "none".
std::wstring ResolveEffectiveControllerBundle(
    const std::wstring& bundle, const LaunchTarget& target);

std::wstring ProfilesRoot(const std::wstring& runtimeRoot);
std::wstring ProfileDir(const std::wstring& runtimeRoot, const std::wstring& id);
std::wstring ProfileGameDir(const std::wstring& runtimeRoot, const std::wstring& id);
std::wstring ProfileModsDir(const std::wstring& runtimeRoot, const std::wstring& id);
std::wstring MakeTargetId(const std::wstring& minecraftVersion, const std::wstring& loader, const std::wstring& loaderVersion);

LaunchTarget DefaultLaunchTarget();
std::wstring TargetShortText(const LaunchTarget& target);
std::wstring TargetProfileText(const LaunchTarget& target);

std::vector<LaunchTarget> LoadVersionCatalog(const std::wstring& runtimeRoot);
LaunchTarget ResolveLaunchTarget(const std::wstring& runtimeRoot, const std::wstring& targetId);
LaunchTarget ResolveProfileTarget(const std::wstring& runtimeRoot, const Profile& profile);

std::vector<Profile> LoadProfiles(const std::wstring& runtimeRoot);
std::wstring GetActiveProfileId(const std::wstring& runtimeRoot);
bool SetActiveProfileId(const std::wstring& runtimeRoot, const std::wstring& id);
Profile GetProfileById(const std::wstring& runtimeRoot, const std::wstring& id);

std::wstring CreateProfile(const std::wstring& runtimeRoot, const std::wstring& name, const LaunchTarget& target);
std::wstring CreateProfile(const std::wstring& runtimeRoot, const std::wstring& name);
std::wstring CreateAutoProfile(const std::wstring& runtimeRoot, const LaunchTarget& target);

bool BackupProfile(const std::wstring& runtimeRoot, const std::wstring& id, std::wstring& backupDir);
bool RestoreProfileBackup(const std::wstring& runtimeRoot, const std::wstring& backupDir, bool removeBackup, std::wstring& restoredName);
bool DeleteProfilePermanent(const std::wstring& runtimeRoot, const std::wstring& id);
bool DeleteProfile(const std::wstring& runtimeRoot, const std::wstring& id);
bool RenameProfile(const std::wstring& runtimeRoot, const std::wstring& id, const std::wstring& newName);
bool SetProfileControllerModEnabled(const std::wstring& runtimeRoot, const std::wstring& id, bool enabled);
bool SetProfileControllerBundle(const std::wstring& runtimeRoot, const std::wstring& id, const std::wstring& bundle);
std::wstring NormalizePerfRenderer(const std::wstring& mode);
bool SetProfilePerfRenderer(const std::wstring& runtimeRoot, const std::wstring& id, const std::wstring& mode);

void EnsureProfilesInitialized(const std::wstring& runtimeRoot);
void EnsureProfileGameDataInitialized(const std::wstring& runtimeRoot, const std::wstring& profileId);
void MigrateLegacyProfileModsForProfile(const std::wstring& runtimeRoot, const std::wstring& profileId);
std::vector<std::wstring> ListProfileMods(const std::wstring& runtimeRoot, const std::wstring& id);
std::wstring ProfileDisplayName(const std::wstring& runtimeRoot, const std::wstring& id);
std::wstring ProfileDisplayTarget(const std::wstring& runtimeRoot, const std::wstring& id);

std::wstring LatestProfileBackup(const std::wstring& runtimeRoot, const std::wstring& kind);
std::wstring ProfileBackupDisplayName(const std::wstring& backupDir);
int ProfileModCount(const std::wstring& runtimeRoot, const std::wstring& id);
