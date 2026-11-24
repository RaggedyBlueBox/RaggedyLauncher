#pragma once

#include <string>
#include <vector>

void TagManagedFiles(
    const std::wstring& userModsDir, const std::vector<std::wstring>& fileNames, const std::wstring& ownerToken);
int RemoveManagedFiles(
    const std::wstring& runtimeRoot, const std::wstring& profileId, const std::wstring& ownerToken);
bool ManagedFilesPresent(
    const std::wstring& runtimeRoot, const std::wstring& profileId, const std::wstring& ownerToken);
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
