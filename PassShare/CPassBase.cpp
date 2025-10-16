#include <fstream>
#include <sstream>
#include <locale>
#include <codecvt>

#include "CPassBase.h"
#include "json.hpp"

#include "CCryptoManage.h"


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


bool PasswordDatabase::LoadEncrypted(const std::wstring& filePath, const std::wstring& masterPassword)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
        return false;

    // 1. Читаем соль (16 байт)
    std::vector<unsigned char> salt(16);
    file.read(reinterpret_cast<char*>(salt.data()), salt.size());

    // 2. Читаем остаток файла (IV + зашифрованные данные)
    file.seekg(0, std::ios::end);
    size_t totalSize = file.tellg();
    file.seekg(16, std::ios::beg);

    std::vector<unsigned char> encryptedData(totalSize - 16);
    file.read(reinterpret_cast<char*>(encryptedData.data()), encryptedData.size());
    file.close();

    // 3. Генерируем ключ из пароля
    std::vector<unsigned char> key;
    if (!CryptoManager::DeriveKeyFromPassword(masterPassword, salt, key))
        return false;

    // 4. Расшифровываем данные
    std::string decryptedData;
    if (!CryptoManager::DecryptData(encryptedData, key, decryptedData))
    {
        // Очистка ключа перед выходом
        CryptoManager::SecureClear(key);
        return false; // Ошибка расшифровки = неверный пароль
    }

    // 5. Парсим JSON и заполняем m_entries
    try
    {
        json j = json::parse(decryptedData);
        m_entries = j.at("entries").get<std::vector<PasswordTemplate>>(); // ✅ Вот сюда
    }
    catch (...)
    {
        // Очистка ключа перед выходом
        CryptoManager::SecureClear(key);
        return false; // Ошибка парсинга = неверный пароль или повреждённый файл
    }

    // 6. Очищаем ключ
    CryptoManager::SecureClear(key);

    return true;
}

bool PasswordDatabase::SaveEncrypted(const std::wstring& filePath, const std::wstring& masterPassword)
{
    json j;
    j["entries"] = m_entries;
    std::string plaintext = j.dump(4);

    // Генерируем соль
    auto salt = CryptoManager::GenerateSalt(16);
    if (salt.empty())
        return false;

    // Генерируем ключ
    std::vector<unsigned char> key;
    if (!CryptoManager::DeriveKeyFromPassword(masterPassword, salt, key))
        return false;

    // Шифруем
    std::vector<unsigned char> encryptedData;
    if (!CryptoManager::EncryptData(plaintext, key, encryptedData))
    {
        // Очистка ключа перед выходом
        CryptoManager::SecureClear(key);
        return false;
    }

    // Записываем в файл: [salt] + [encrypted_data]
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        // Очистка ключа перед выходом
        CryptoManager::SecureClear(key);
        return false;
    }

    file.write(reinterpret_cast<const char*>(salt.data()), salt.size());
    file.write(reinterpret_cast<const char*>(encryptedData.data()), encryptedData.size());
    file.close();

    // Очищаем ключ
    CryptoManager::SecureClear(key);

    return true;
}

bool PasswordDatabase::CreateEncryptedBase(const std::wstring& filePath, const std::wstring& masterPassword)
{
    m_entries.clear(); // Очищаем текущие записи
    return SaveEncrypted(filePath, masterPassword);
}