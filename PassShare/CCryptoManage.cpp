#include "CCryptoManage.h"
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <windows.h>
#include <string>
#include <locale>
#include <codecvt>

// --- Генерация ключа ---
bool CryptoManager::DeriveKeyFromPassword(const std::wstring& password,
    const std::vector<unsigned char>& salt,
    std::vector<unsigned char>& key)
{
    // Конвертируем wstring в UTF-8
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::string passwordUtf8 = converter.to_bytes(password);

    key.resize(32); // 256 бит = 32 байта

    int result = PKCS5_PBKDF2_HMAC(passwordUtf8.c_str(), passwordUtf8.length(),
        salt.data(), salt.size(),
        10000, // количество итераций
        EVP_sha256(), // хеш-функция
        32, // длина ключа
        key.data());

    return result == 1;
}

// --- Шифрование ---
bool CryptoManager::EncryptData(const std::string& plaintext,
    const std::vector<unsigned char>& key,
    std::vector<unsigned char>& encryptedData)
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), NULL) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    // Генерация случайного IV
    std::vector<unsigned char> iv(AES_BLOCK_SIZE);
    if (RAND_bytes(iv.data(), AES_BLOCK_SIZE) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    // Устанавливаем IV
    if (EVP_EncryptInit_ex(ctx, NULL, NULL, NULL, iv.data()) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    std::vector<unsigned char> ciphertext(plaintext.length() + AES_BLOCK_SIZE);
    int len = 0;
    int ciphertext_len = 0;

    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, (const unsigned char*)plaintext.c_str(), plaintext.length()) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    // Возвращаем [IV][ciphertext]
    encryptedData.resize(iv.size() + ciphertext_len);
    memcpy(encryptedData.data(), iv.data(), iv.size());
    memcpy(encryptedData.data() + iv.size(), ciphertext.data(), ciphertext_len);

    return true;
}

// --- Расшифровка ---
bool CryptoManager::DecryptData(const std::vector<unsigned char>& encryptedData,
    const std::vector<unsigned char>& key,
    std::string& decryptedData)
{
    if (encryptedData.size() < AES_BLOCK_SIZE)
        return false;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;

    // Извлекаем IV
    std::vector<unsigned char> iv(encryptedData.begin(), encryptedData.begin() + AES_BLOCK_SIZE);
    // Остальные данные - зашифрованные
    std::vector<unsigned char> ciphertext(encryptedData.begin() + AES_BLOCK_SIZE, encryptedData.end());

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), iv.data()) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    std::vector<unsigned char> plaintext(ciphertext.size() + AES_BLOCK_SIZE);
    int len = 0;
    int plaintext_len = 0;

    if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size()) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return false; // Ошибка расшифровки = неверный ключ
    }
    plaintext_len = len;

    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return false; // Ошибка расшифровки = неверный ключ
    }
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    decryptedData = std::string(reinterpret_cast<char*>(plaintext.data()), plaintext_len);
    return true;
}

// --- Генерация соли ---
std::vector<unsigned char> CryptoManager::GenerateSalt(size_t length)
{
    std::vector<unsigned char> salt(length);
    if (RAND_bytes(salt.data(), length) != 1)
        return {}; // возвращаем пустой вектор при ошибке
    return salt;
}

// --- Очистка памяти ---
void CryptoManager::SecureClear(std::vector<unsigned char>& buffer)
{
    if (!buffer.empty())
    {
        SecureZeroMemory(buffer.data(), buffer.size());
        buffer.clear();
    }
}

void CryptoManager::SecureClear(std::string& str)
{
    if (!str.empty())
    {
        SecureZeroMemory(const_cast<char*>(str.data()), str.size());
        str.clear();
    }
}