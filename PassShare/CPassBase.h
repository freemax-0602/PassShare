#pragma once
#include "CPassTemplate.h"
#include <vector>
#include <string>

class PasswordDatabase
{
public:
    bool LoadBase(const std::wstring& filePath);
    bool SaveBase(const std::wstring& filePath);
    bool CreateBase(const std::wstring& filePath);

    std::vector<PasswordTemplate>& GetEntries() { return m_entries; }
    const std::vector<PasswordTemplate>& GetEntries() const { return m_entries; }

private:
    std::vector<PasswordTemplate> m_entries;
};