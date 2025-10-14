#include "CPassBase.h"
#include "json.hpp"
#include <fstream>
#include <sstream>
#include <locale>
#include <codecvt>

using json = nlohmann::json;

void to_json(json& j, const PasswordTemplate& e)
{
    j = json{
        {"icon", e.icon},
        {"url", e.url},
        {"login", e.login},
        {"password", e.password},
        {"description", e.description}
    };
}

void from_json(const json& j, PasswordTemplate& e)
{
    j.at("icon").get_to(e.icon);
    j.at("url").get_to(e.url);
    j.at("login").get_to(e.login);
    j.at("password").get_to(e.password);
    j.at("description").get_to(e.description);
}

bool PasswordDatabase::LoadBase(const std::wstring& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
        return false;

    try
    {
        json j;
        file >> j;

        m_entries.clear();
        for (const auto& entry : j.at("entries"))
        {
            m_entries.emplace_back(entry.get<PasswordTemplate>());
        }
    }
    catch (...)
    {
        return false;
    }

    return true;
}

bool PasswordDatabase::SaveBase(const std::wstring& filePath)
{
    json j;
    j["entries"] = m_entries;

    std::wofstream file(filePath);
    if (!file.is_open())
        return false;

    // Конвертируем строку в wstring
    std::string jsonString = j.dump(4);
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wideJson = converter.from_bytes(jsonString);

    file << wideJson;
    return true;
}

bool PasswordDatabase::CreateBase(const std::wstring& filePath)
{
    m_entries.clear();
    return SaveBase(filePath);
}
