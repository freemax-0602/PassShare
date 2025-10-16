#pragma once
#include <vector>
#include <string>
#include <memory>

class CryptoManager
{
public:
    // Генерирует ключ из пароля и соли
    static bool DeriveKeyFromPassword(const std::wstring& password,
        const std::vector<unsigned char>& salt,
        std::vector<unsigned char>& key);

    // Шифрует данные (plaintext) с помощью ключа
    // Возвращает [IV][зашифрованные_данные]
    static bool EncryptData(const std::string& plaintext,
        const std::vector<unsigned char>& key,
        std::vector<unsigned char>& encryptedData);

    // Расшифровывает данные (в формате [IV][зашифрованные_данные]) с помощью ключа
    static bool DecryptData(const std::vector<unsigned char>& encryptedData,
        const std::vector<unsigned char>& key,
        std::string& decryptedData);

    // Генерирует случайную соль
    static std::vector<unsigned char> GenerateSalt(size_t length = 16);

    // Очищает буфер (для безопасности)
    static void SecureClear(std::vector<unsigned char>& buffer);
    static void SecureClear(std::string& str);
};