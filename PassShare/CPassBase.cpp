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

bool PasswordDatabase::Load(const std::wstring& filePath)
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

bool PasswordDatabase::Save(const std::wstring& filePath)
{
    json j;
    m_entries.clear();
    j["entries"] = json::array();

    std::string jsonString = j.dump(4);  // строка в UTF-8

    // Конвертируем в wstring (UTF-16)
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wideJson = converter.from_bytes(jsonString);

    std::wofstream file(filePath);
    if (!file.is_open())
        return false;

    file << wideJson;  // теперь можно записать
    return true;
}

bool PasswordDatabase::CreateNew(const std::wstring& filePath)
{
    m_entries.clear();
    return Save(filePath);
}
