#include <fstream>
#include <sstream>
#include <locale>
#include <codecvt>
#include <vector>

#include "CPassBase.h"
#include "json.hpp"
#include "CCryptoManage.h"


using json = nlohmann::json;

bool PasswordDatabase::LoadEncrypted(const std::wstring& filePath, const std::wstring& masterPassword)
{
    m_lastUsedSalt.clear(); // <-- НОВОЕ: Очищаем перед использованием

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
        return false;

    // 1. Читаем соль (16 байт)
    std::vector<unsigned char> salt(16);
    file.read(reinterpret_cast<char*>(salt.data()), salt.size());

    // <-- НОВОЕ: Сохраняем прочитанную соль
    m_lastUsedSalt = salt;
    // --->

    // 2. Читаем остаток файла (IV + зашифрованные данные)
    file.seekg(0, std::ios::end);
    size_t totalSize = file.tellg();
    file.seekg(16, std::ios::beg);

    std::vector<unsigned char> encryptedData(totalSize - 16);
    file.read(reinterpret_cast<char*>(encryptedData.data()), encryptedData.size());
    file.close();

    // 3. Генерируем ключ из пароля и СОХРАНЁННОЙ СОЛИ
    std::vector<unsigned char> key;
    if (!CryptoManager::DeriveKeyFromPassword(masterPassword, salt, key)) // <-- Используем salt
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
        auto j_entries = j.at("entries").get<std::vector<json>>();

        m_entries.clear();
        for (const auto& j_entry : j_entries)
        {
            PasswordTemplate entry;
            entry.url = j_entry.at("url").get<std::wstring>();
            entry.description = j_entry.at("description").get<std::wstring>();

            // Получаем бинарные зашифрованные данные из JSON как binary_t
            entry.encrypted_login = j_entry.at("encrypted_login").get_binary();
            entry.encrypted_password = j_entry.at("encrypted_password").get_binary();

            m_entries.push_back(entry);
        }
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
    m_lastUsedSalt.clear(); // <-- НОВОЕ: Очищаем перед использованием

    // Генерируем НОВУЮ соль
    auto salt = CryptoManager::GenerateSalt(16);
    if (salt.empty())
        return false;

    // <-- НОВОЕ: Сохраняем сгенерированную соль
    m_lastUsedSalt = salt;
    // --->

    // Генерируем ключ из пароля и СОХРАНЁННОЙ СОЛИ
    std::vector<unsigned char> key;
    if (!CryptoManager::DeriveKeyFromPassword(masterPassword, salt, key)) // <-- Используем salt
        return false;

    // 1. Подготавливаем JSON с зашифрованными данными
    json j;
    json j_entries = json::array();

    for (const auto& entry : m_entries)
    {
        json j_entry;
        j_entry["url"] = entry.url;
        j_entry["description"] = entry.description;

        // Сохраняем зашифрованные данные как binary_t
        j_entry["encrypted_login"] = json::binary(entry.encrypted_login);
        j_entry["encrypted_password"] = json::binary(entry.encrypted_password);

        j_entries.push_back(j_entry);
    }
    j["entries"] = j_entries;

    std::string plaintext = j.dump(4);

    // 2. Шифруем весь JSON
    std::vector<unsigned char> encryptedData;
    if (!CryptoManager::EncryptData(plaintext, key, encryptedData))
    {
        // Очистка ключа перед выходом
        CryptoManager::SecureClear(key);
        return false;
    }

    // 3. Записываем в файл: [salt] + [encrypted_data]
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        // Очистка ключа перед выходом
        CryptoManager::SecureClear(key);
        return false;
    }

    file.write(reinterpret_cast<const char*>(salt.data()), salt.size()); // <-- Используем salt
    file.write(reinterpret_cast<const char*>(encryptedData.data()), encryptedData.size());
    file.close();

    // 4. Очищаем ключ
    CryptoManager::SecureClear(key);

    return true;
}

bool PasswordDatabase::CreateEncryptedBase(const std::wstring& filePath, const std::wstring& masterPassword)
{
    m_entries.clear(); // Очищаем текущие записи
    // m_lastUsedSalt будет установлена внутри SaveEncrypted
    return SaveEncrypted(filePath, masterPassword);
}

// --- НОВОЕ: Реализация новых методов ---
bool PasswordDatabase::AddEntry(const std::wstring& url, const std::wstring& login, const std::wstring& password,
    const std::wstring& description, const std::vector<unsigned char>& key)
{
    PasswordTemplate newEntry;
    newEntry.url = url;
    newEntry.description = description;

    // Конвертируем wstring в UTF-8
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::string loginUtf8 = converter.to_bytes(login);
    std::string passwordUtf8 = converter.to_bytes(password);

    // Шифруем
    if (!CryptoManager::EncryptString(loginUtf8, key, newEntry.encrypted_login) ||
        !CryptoManager::EncryptString(passwordUtf8, key, newEntry.encrypted_password))
    {
        return false; // Ошибка шифрования
    }

    m_entries.push_back(newEntry);
    return true;
}

bool PasswordDatabase::GetDecryptedLogin(int index, const std::vector<unsigned char>& key, std::wstring& decryptedLogin)
{
    if (index < 0 || index >= static_cast<int>(m_entries.size()))
        return false;

    const auto& entry = m_entries[index];
    std::string decryptedLoginUtf8;

    if (!CryptoManager::DecryptString(entry.encrypted_login, key, decryptedLoginUtf8))
    {
        return false; // Ошибка расшифровки
    }

    // Конвертируем из UTF-8 обратно в wstring
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    decryptedLogin = converter.from_bytes(decryptedLoginUtf8);
    return true;
}

bool PasswordDatabase::GetDecryptedPassword(int index, const std::vector<unsigned char>& key, std::wstring& decryptedPassword)
{
    if (index < 0 || index >= static_cast<int>(m_entries.size()))
        return false;

    const auto& entry = m_entries[index];
    std::string decryptedPasswordUtf8;

    if (!CryptoManager::DecryptString(entry.encrypted_password, key, decryptedPasswordUtf8))
    {
        return false; // Ошибка расшифровки
    }

    // Конвертируем из UTF-8 обратно в wstring
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    decryptedPassword = converter.from_bytes(decryptedPasswordUtf8);
    return true;
}

const std::vector<unsigned char>& PasswordDatabase::GetLastUsedSalt() const
{
    return m_lastUsedSalt; // Просто возвращает ссылку на приватное поле
}