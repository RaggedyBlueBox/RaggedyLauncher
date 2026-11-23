#pragma once

#include <string>
#include <vector>

#include "profiles.h"

// Launcher-managed controller bundles. Files installed for the "legacy4j" and
// "controlify" bundles live in the profile mods directory alongside user mods.
// Ownership is tracked per file in the existing .raggedy/mod-installs meta
// sidecars ("bundleOwners" comma set, "bundleSha1", "bundleUserBackup") so
// switching bundles only ever removes exact launcher-installed versions and
// never deletes user-owned jars. Shared dependencies (Fabric API, YACL, ...)
// survive as long as another bundle or mod still references them.

// UI cycle order: Auto -> Legacy4J -> Raggedy -> Controlify -> None.
std::wstring NextControllerBundle(const std::wstring& bundle);
const wchar_t* ControllerBundleLabelKey(const std::wstring& bundle);

// True when the profile has at least one installed file owned by the bundle.
bool BundleHasInstalledFiles(const std::wstring& runtimeRoot, const std::wstring& profileId, const std::wstring& bundle);
// Removes files owned by the bundle (hash-checked, refcount-aware, restores
// user backups replaced at install time). Returns removed file count.
int RemoveBundleOwnedFiles(const std::wstring& runtimeRoot, const std::wstring& profileId, const std::wstring& bundle);
// Directory where installers stash user-owned jars a bundle replaces.
std::wstring BundleUserBackupDir(const std::wstring& userModsDir, const std::wstring& bundle);
// True when any bundle claims the file in its ownership sidecar.
bool BundleFileHasOwners(const std::wstring& userModsDir, const std::wstring& fileName);
// Records which user file was replaced so removal can restore it.
void NoteBundleUserBackup(
    const std::wstring& userModsDir, const std::wstring& fileName, const std::wstring& bundle);
// Names of unmanaged (user-installed, untracked) jars that conflict with the
// effective bundle. Empty when there is nothing to warn about.
std::vector<std::wstring> FindBundleConflicts(
    const std::wstring& runtimeRoot, const std::wstring& profileId, const std::wstring& effectiveBundle);
// Pre-launch sync: drops stale managed files of the non-effective bundles and
// launcher-copied Raggedy jars when the effective bundle is not Raggedy. Never
// deletes unmanaged user jars; those are reported through FindBundleConflicts.
int SyncControllerBundleForLaunch(
    const std::wstring& runtimeRoot, const std::wstring& profileId,
    const std::wstring& bundle, const LaunchTarget& target);
// Records bundle ownership on files a bundle install just wrote.
void TagBundleOwnedFiles(
    const std::wstring& userModsDir, const std::vector<std::wstring>& fileNames, const std::wstring& bundle);
// Generic ownership tagging with a caller-chosen token (for example
// "perf:sodium"). Shares the bundleOwners/bundleSha1/bundleUserBackup sidecar
// format and its hash-checked, refcount-aware removal.
void TagManagedFiles(
    const std::wstring& userModsDir, const std::vector<std::wstring>& fileNames, const std::wstring& ownerToken);
int RemoveManagedFiles(
    const std::wstring& runtimeRoot, const std::wstring& profileId, const std::wstring& ownerToken);
bool ManagedFilesPresent(
    const std::wstring& runtimeRoot, const std::wstring& profileId, const std::wstring& ownerToken);
// All ownership tokens on the profile starting with prefix (lowercase compare).
std::vector<std::wstring> ManagedOwnerTokensWithPrefix(
    const std::wstring& runtimeRoot, const std::wstring& profileId, const std::wstring& prefix);
std::wstring ManagedBackupDir(const std::wstring& userModsDir, const std::wstring& ownerToken);
void NoteManagedUserBackup(
    const std::wstring& userModsDir, const std::wstring& fileName, const std::wstring& ownerToken);
bool ReadManagedMetaValue(const std::wstring& userModsDir, const std::wstring& fileName,
    const std::wstring& key, std::wstring& value);
bool WriteManagedMetaValue(const std::wstring& userModsDir, const std::wstring& fileName,
    const std::wstring& key, const std::wstring& value);
bool RemoveManagedMetaValue(const std::wstring& userModsDir, const std::wstring& fileName, const std::wstring& key);
