#include "controller_bundle.h"

#include "i18n.h"
#include "launcher_common.h"
#include "runtime_manager.h"

#include <set>
#include <sstream>

namespace {

// Forward declarations: owner/accounting helpers below are used by the
// backup/tag entry points above them.
bool ReadMetaLine(const std::wstring& userModsDir, const std::wstring& fileName,
    const std::wstring& key, std::wstring& value);
bool WriteMetaLine(const std::wstring& userModsDir, const std::wstring& fileName,
    const std::wstring& key, const std::wstring& value);
bool RemoveMetaLine(const std::wstring& userModsDir, const std::wstring& fileName, const std::wstring& key);
std::set<std::wstring> SplitOwners(const std::wstring& value);
std::wstring JoinOwners(const std::set<std::wstring>& values);

std::wstring BundleMetaDir(const std::wstring& userModsDir) {
    return GetParentDir(userModsDir) + L"\\.raggedy\\mod-installs";
}

std::wstring BundleMetaPath(const std::wstring& userModsDir, const std::wstring& fileName) {
    return BundleMetaDir(userModsDir) + L"\\" + SafeFileName(fileName) + L".meta";
}


bool ReadMetaLine(const std::wstring& userModsDir, const std::wstring& fileName,
    const std::wstring& key, std::wstring& value) {
    std::wstring body;
    if (!ReadTextFile(BundleMetaPath(userModsDir, fileName), body)) return false;
    std::wstringstream ss(body);
    std::wstring line;
    while (std::getline(ss, line)) {
        if (!line.empty() && line.back() == L'\r') line.pop_back();
        const size_t tab = line.find(L'\t');
        if (tab == std::wstring::npos) continue;
        if (line.substr(0, tab) == key) {
            value = line.substr(tab + 1);
            return true;
        }
    }
    return false;
}

bool WriteMetaLine(const std::wstring& userModsDir, const std::wstring& fileName,
    const std::wstring& key, const std::wstring& value) {
    const std::wstring path = BundleMetaPath(userModsDir, fileName);
    std::wstring body;
    ReadTextFile(path, body);
    std::wstringstream out;
    bool replaced = false;
    if (!body.empty()) {
        std::wstringstream ss(body);
        std::wstring line;
        while (std::getline(ss, line)) {
            const bool hasCr = !line.empty() && line.back() == L'\r';
            std::wstring raw = line;
            if (hasCr) line.pop_back();
            const size_t tab = line.find(L'\t');
            if (tab != std::wstring::npos && line.substr(0, tab) == key) {
                if (!replaced) {
                    out << key << L"\t" << value << L"\n";
                    replaced = true;
                }
                continue;
            }
            out << raw << (hasCr ? L"" : L"") << L"\n";
        }
    }
    if (!replaced) out << key << L"\t" << value << L"\n";
    EnsureDirectoryTree(BundleMetaDir(userModsDir));
    return WriteTextFile(path, out.str());
}

bool RemoveMetaLine(const std::wstring& userModsDir, const std::wstring& fileName, const std::wstring& key) {
    const std::wstring path = BundleMetaPath(userModsDir, fileName);
    std::wstring body;
    if (!ReadTextFile(path, body)) return false;
    std::wstringstream out;
    bool removed = false;
    std::wstringstream ss(body);
    std::wstring line;
    while (std::getline(ss, line)) {
        std::wstring check = line;
        if (!check.empty() && check.back() == L'\r') check.pop_back();
        const size_t tab = check.find(L'\t');
        if (tab != std::wstring::npos && check.substr(0, tab) == key) {
            removed = true;
            continue;
        }
        out << line << L"\n";
    }
    if (!removed) return false;
    return WriteTextFile(path, out.str());
}

std::set<std::wstring> SplitOwners(const std::wstring& value) {
    std::set<std::wstring> out;
    size_t start = 0;
    while (start <= value.size()) {
        const size_t comma = value.find(L',', start);
        std::wstring part = value.substr(start, comma == std::wstring::npos ? std::wstring::npos : comma - start);
        if (!part.empty()) out.insert(ToLowerW(part));
        if (comma == std::wstring::npos) break;
        start = comma + 1;
    }
    return out;
}

std::wstring JoinOwners(const std::set<std::wstring>& values) {
    std::wstring out;
    for (const std::wstring& v : values) {
        if (!out.empty()) out += L",";
        out += v;
    }
    return out;
}

bool HasForeignRefs(const std::wstring& requiredBy) {
    // Launcher-managed refcounts ("bundle:" for controller bundles,
    // "perf:" for performance renderers) never block removal on their own;
    // anything else means a user install still needs the file.
    for (const std::wstring& token : SplitOwners(requiredBy)) {
        if (token.rfind(L"bundle:", 0) == 0) continue;
        if (token.rfind(L"perf:", 0) == 0) continue;
        return true;
    }
    return false;
}

}  // namespace

std::wstring BundleUserBackupDir(const std::wstring& userModsDir, const std::wstring& bundle) {
    return GetParentDir(userModsDir) + L"\\.raggedy\\bundle-backups\\" + SafeFileName(ToLowerW(NormalizeControllerBundle(bundle)));
}

bool BundleFileHasOwners(const std::wstring& userModsDir, const std::wstring& fileName) {
    std::wstring owners;
    if (!ReadMetaLine(userModsDir, fileName, L"bundleOwners", owners)) return false;
    return !SplitOwners(owners).empty();
}

std::wstring ManagedBackupDir(const std::wstring& userModsDir, const std::wstring& ownerToken) {
    return GetParentDir(userModsDir) + L"\\.raggedy\\bundle-backups\\" + SafeFileName(ToLowerW(ownerToken));
}

void NoteManagedUserBackup(
    const std::wstring& userModsDir, const std::wstring& fileName, const std::wstring& ownerToken) {
    WriteMetaLine(userModsDir, fileName, L"bundleUserBackup",
        SafeFileName(ToLowerW(ownerToken)) + L"\\" + SafeFileName(fileName));
}

void NoteBundleUserBackup(
    const std::wstring& userModsDir, const std::wstring& fileName, const std::wstring& bundle) {
    NoteManagedUserBackup(userModsDir, fileName, ToLowerW(NormalizeControllerBundle(bundle)));
}

bool ReadManagedMetaValue(const std::wstring& userModsDir, const std::wstring& fileName,
    const std::wstring& key, std::wstring& value) {
    return ReadMetaLine(userModsDir, fileName, key, value);
}

bool WriteManagedMetaValue(const std::wstring& userModsDir, const std::wstring& fileName,
    const std::wstring& key, const std::wstring& value) {
    return WriteMetaLine(userModsDir, fileName, key, value);
}

bool RemoveManagedMetaValue(const std::wstring& userModsDir, const std::wstring& fileName, const std::wstring& key) {
    return RemoveMetaLine(userModsDir, fileName, key);
}

std::wstring NextControllerBundle(const std::wstring& bundle) {
    const std::wstring b = NormalizeControllerBundle(bundle);
    if (b == L"auto") return L"legacy4j";
    if (b == L"legacy4j") return L"raggedy";
    if (b == L"raggedy") return L"controlify";
    if (b == L"controlify") return L"none";
    return L"auto";
}

const wchar_t* ControllerBundleLabelKey(const std::wstring& bundle) {
    const std::wstring b = NormalizeControllerBundle(bundle);
    if (b == L"legacy4j") return L"Legacy4J";
    if (b == L"raggedy") return L"Raggedy";
    if (b == L"controlify") return L"Controlify";
    if (b == L"none") return L"None";
    return L"Auto";
}



void TagManagedFiles(
    const std::wstring& userModsDir, const std::vector<std::wstring>& fileNames, const std::wstring& ownerToken) {
    const std::wstring want = ToLowerW(ownerToken);
    for (const std::wstring& name : fileNames) {
        std::wstring owners;
        ReadMetaLine(userModsDir, name, L"bundleOwners", owners);
        std::set<std::wstring> set = SplitOwners(owners);
        set.insert(want);
        WriteMetaLine(userModsDir, name, L"bundleOwners", JoinOwners(set));
        std::string sha1;
        if (Sha1File(userModsDir + L"\\" + name, &sha1) && !sha1.empty()) {
            WriteMetaLine(userModsDir, name, L"bundleSha1", a2w(sha1.c_str()));
        }
    }
}

static int RemoveOwnedImpl(const std::wstring& runtimeRoot, const std::wstring& profileId,
    const std::wstring& want, const wchar_t* label) {
    const std::wstring modsDir = ProfileModsDir(runtimeRoot, profileId);
    int removed = 0;
    for (const std::wstring& name : ListProfileMods(runtimeRoot, profileId)) {
        std::wstring owners;
        if (!ReadMetaLine(modsDir, name, L"bundleOwners", owners)) continue;
        std::set<std::wstring> set = SplitOwners(owners);
        if (set.count(want) == 0) continue;
        set.erase(want);

        std::wstring requiredBy;
        ReadMetaLine(modsDir, name, L"requiredBy", requiredBy);
        const bool foreign = HasForeignRefs(requiredBy);
        std::wstring recordedSha1;
        ReadMetaLine(modsDir, name, L"bundleSha1", recordedSha1);
        const std::wstring path = modsDir + L"\\" + name;
        const bool matches = recordedSha1.empty() || FileMatchesSha1(path, w2a(recordedSha1.c_str()));
        std::wstring backup;
        ReadMetaLine(modsDir, name, L"bundleUserBackup", backup);

        if (!matches) {
            // The user replaced the file with their own build. Keep it and
            // just drop this bundle's ownership claim.
            WriteLogF(L"%s %s keeps user-modified %s", label, want.c_str(), name.c_str());
            if (set.empty()) {
                RemoveMetaLine(modsDir, name, L"bundleOwners");
                RemoveMetaLine(modsDir, name, L"bundleSha1");
            } else {
                WriteMetaLine(modsDir, name, L"bundleOwners", JoinOwners(set));
            }
            continue;
        }
        if (!set.empty() || foreign) {
            // Still needed by another bundle or mod install. Keep the file.
            WriteMetaLine(modsDir, name, L"bundleOwners", JoinOwners(set));
            if (set.empty()) RemoveMetaLine(modsDir, name, L"bundleSha1");
            continue;
        }
        SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_NORMAL);
        if (!DeleteFileW(path.c_str())) {
            WriteLogF(L"%s %s could not remove %s err=%u", label, want.c_str(), name.c_str(), GetLastError());
            WriteMetaLine(modsDir, name, L"bundleOwners", JoinOwners(set));
            continue;
        }
        ++removed;
        if (!backup.empty()) {
            const std::wstring backupPath = GetParentDir(modsDir) + L"\\.raggedy\\bundle-backups\\" + backup;
            if (GetFileAttributesW(backupPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
                if (MoveFileExW(backupPath.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING)) {
                    WriteLogF(L"%s %s restored user file %s", label, want.c_str(), name.c_str());
                } else {
                    WriteLogF(L"%s %s could not restore user file %s err=%u", label, want.c_str(), name.c_str(), GetLastError());
                }
            }
        }
        DeleteFileW(BundleMetaPath(modsDir, name).c_str());
    }
    if (removed > 0) WriteLogF(L"%s %s removed %d owned file(s)", label, want.c_str(), removed);
    return removed;
}

void TagBundleOwnedFiles(
    const std::wstring& userModsDir, const std::vector<std::wstring>& fileNames, const std::wstring& bundle) {
    TagManagedFiles(userModsDir, fileNames, ToLowerW(NormalizeControllerBundle(bundle)));
}

int RemoveBundleOwnedFiles(const std::wstring& runtimeRoot, const std::wstring& profileId, const std::wstring& bundle) {
    const std::wstring want = ToLowerW(NormalizeControllerBundle(bundle));
    int removed = RemoveOwnedImpl(runtimeRoot, profileId, want, L"Bundle");
    if (want == L"raggedy") {
        // Pre-fork installs tracked the same bundle as "bundle:bandit".
        removed += RemoveOwnedImpl(runtimeRoot, profileId, L"bundle:bandit", L"Bundle");
    }
    return removed;
}

int RemoveManagedFiles(const std::wstring& runtimeRoot, const std::wstring& profileId, const std::wstring& ownerToken) {
    return RemoveOwnedImpl(runtimeRoot, profileId, ToLowerW(ownerToken), L"Perf");
}

std::vector<std::wstring> ManagedOwnerTokensWithPrefix(
    const std::wstring& runtimeRoot, const std::wstring& profileId, const std::wstring& prefix) {
    const std::wstring modsDir = ProfileModsDir(runtimeRoot, profileId);
    const std::wstring want = ToLowerW(prefix);
    std::set<std::wstring> out;
    for (const std::wstring& name : ListProfileMods(runtimeRoot, profileId)) {
        std::wstring owners;
        if (!ReadMetaLine(modsDir, name, L"bundleOwners", owners)) continue;
        for (const std::wstring& token : SplitOwners(owners)) {
            if (token.rfind(want, 0) == 0) out.insert(token);
        }
    }
    return std::vector<std::wstring>(out.begin(), out.end());
}

bool ManagedFilesPresent(const std::wstring& runtimeRoot, const std::wstring& profileId, const std::wstring& ownerToken) {
    const std::wstring modsDir = ProfileModsDir(runtimeRoot, profileId);
    const std::wstring want = ToLowerW(ownerToken);
    for (const std::wstring& name : ListProfileMods(runtimeRoot, profileId)) {
        std::wstring owners;
        if (!ReadMetaLine(modsDir, name, L"bundleOwners", owners)) continue;
        if (SplitOwners(owners).count(want) == 0) continue;
        if (GetFileAttributesW((modsDir + L"\\" + name).c_str()) == INVALID_FILE_ATTRIBUTES) continue;
        return true;
    }
    return false;
}

std::vector<std::wstring> FindBundleConflicts(
    const std::wstring& runtimeRoot, const std::wstring& profileId, const std::wstring& effectiveBundle) {
    const std::wstring modsDir = ProfileModsDir(runtimeRoot, profileId);
    const std::wstring effective = NormalizeControllerBundle(effectiveBundle);
    std::vector<std::wstring> conflicts;
    for (const std::wstring& name : ListProfileMods(runtimeRoot, profileId)) {
        std::wstring owners;
        ReadMetaLine(modsDir, name, L"bundleOwners", owners);
        if (!SplitOwners(owners).empty()) continue;  // launcher-managed, not a user conflict
        const bool isControlify = IsControlifyJarFileName(name);
        const bool isLegacy4J = IsLegacy4JJarFileName(name);
        if (effective == L"raggedy" && (isControlify || isLegacy4J)) conflicts.push_back(name);
        if (effective == L"legacy4j" && isControlify) conflicts.push_back(name);
        if (effective == L"controlify" && isLegacy4J) conflicts.push_back(name);
    }
    return conflicts;
}

int SyncControllerBundleForLaunch(
    const std::wstring& runtimeRoot, const std::wstring& profileId,
    const std::wstring& bundle, const LaunchTarget& target) {
    const std::wstring effective = ResolveEffectiveControllerBundle(bundle, target);
    const std::wstring userModsDir = ProfileModsDir(runtimeRoot, profileId);
    int removed = 0;
    if (effective != L"raggedy") {
        removed += DeleteRaggedyControllerModsFromDir(userModsDir);
    }
    static const wchar_t* kManaged[] = { L"legacy4j", L"controlify" };
    for (const wchar_t* owned : kManaged) {
        if (effective != owned) removed += RemoveBundleOwnedFiles(runtimeRoot, profileId, owned);
    }
    const std::vector<std::wstring> conflicts = FindBundleConflicts(runtimeRoot, profileId, effective);
    for (const std::wstring& name : conflicts) {
        WriteLogF(L"Unmanaged controller mod %s conflicts with the %s bundle; left in place for the user to remove",
            name.c_str(), effective.c_str());
    }
    if (removed > 0) {
        WriteLogF(L"Bundle sync removed %d stale file(s) for effective bundle %s", removed, effective.c_str());
    }
    return removed;
}

bool BundleHasInstalledFiles(const std::wstring& runtimeRoot, const std::wstring& profileId, const std::wstring& bundle) {
    return ManagedFilesPresent(runtimeRoot, profileId, ToLowerW(NormalizeControllerBundle(bundle)));
}
