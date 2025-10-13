#pragma once
#include "CPassTemplate.h"
#include <vector>
#include <string>

class PasswordDatabase
{
public:
    bool Load(const std::wstring& filePath);
    bool Save(const std::wstring& filePath);
    bool CreateNew(const std::wstring& filePath);

    std::vector<PasswordTemplate>& GetEntries() { return m_entries; }
    const std::vector<PasswordTemplate>& GetEntries() const { return m_entries; }

private:
    std::vector<PasswordTemplate> m_entries;
};