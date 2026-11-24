#include "controller_bundle.h"

#include "launcher_common.h"
#include "profiles.h"
#include "runtime_manager.h"

#include <set>
#include <sstream>

namespace {

bool ReadMetaLine(const std::wstring& userModsDir, const std::wstring& fileName,
    const std::wstring& key, std::wstring& value);
bool WriteMetaLine(const std::wstring& userModsDir, const std::wstring& fileName,
    const std::wstring& key, const std::wstring& value);
bool RemoveMetaLine(const std::wstring& userModsDir, const std::wstring& fileName, const std::wstring& key);
std::set<std::wstring> SplitOwners(const std::wstring& value);
std::wstring JoinOwners(const std::set<std::wstring>& values);

std::wstring ManagedMetaDir(const std::wstring& userModsDir) {
    return GetParentDir(userModsDir) + L"\\.raggedy\\mod-installs";
}

std::wstring ManagedMetaPath(const std::wstring& userModsDir, const std::wstring& fileName) {
    return ManagedMetaDir(userModsDir) + L"\\" + SafeFileName(fileName) + L".meta";
}

bool ReadMetaLine(const std::wstring& userModsDir, const std::wstring& fileName,
    const std::wstring& key, std::wstring& value) {
    std::wstring body;
    if (!ReadTextFile(ManagedMetaPath(userModsDir, fileName), body)) return false;
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
    const std::wstring path = ManagedMetaPath(userModsDir, fileName);
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
    EnsureDirectoryTree(ManagedMetaDir(userModsDir));
    return WriteTextFile(path, out.str());
}

bool RemoveMetaLine(const std::wstring& userModsDir, const std::wstring& fileName, const std::wstring& key) {
    const std::wstring path = ManagedMetaPath(userModsDir, fileName);
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
    for (const std::wstring& token : SplitOwners(requiredBy)) {
        if (token.rfind(L"bundle:", 0) == 0) continue;
        if (token.rfind(L"perf:", 0) == 0) continue;
        return true;
    }
    return false;
}

int RemoveOwnedImpl(const std::wstring& runtimeRoot, const std::wstring& profileId,
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
        DeleteFileW(ManagedMetaPath(modsDir, name).c_str());
    }
    if (removed > 0) WriteLogF(L"%s %s removed %d owned file(s)", label, want.c_str(), removed);
    return removed;
}

}

std::wstring ManagedBackupDir(const std::wstring& userModsDir, const std::wstring& ownerToken) {
    return GetParentDir(userModsDir) + L"\\.raggedy\\bundle-backups\\" + SafeFileName(ToLowerW(ownerToken));
}

void NoteManagedUserBackup(
    const std::wstring& userModsDir, const std::wstring& fileName, const std::wstring& ownerToken) {
    WriteMetaLine(userModsDir, fileName, L"bundleUserBackup",
        SafeFileName(ToLowerW(ownerToken)) + L"\\" + SafeFileName(fileName));
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
