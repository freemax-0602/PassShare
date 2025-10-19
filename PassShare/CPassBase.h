#pragma once
#include "CPassTemplate.h"
#include <vector>
#include <string>

class PasswordDatabase
{
public:
    //bool SaveBase(const std::wstring& filePath);
    //bool CreateBase(const std::wstring& filePath);

    bool LoadEncrypted(const std::wstring& filePath, const std::wstring& masterPassword);
    bool SaveEncrypted(const std::wstring& filePath, const std::wstring& masterPassword);
    bool CreateEncryptedBase(const std::wstring& filePath, const std::wstring& masterPassword);

    // --- НОВОЕ ---
    // Метод для добавления новой записи (с шифрованием login/password)
    bool AddEntry(const std::wstring& url, const std::wstring& login, const std::wstring& password,
        const std::wstring& description, const std::vector<unsigned char>& key);

    // Методы для получения расшифрованных полей (для копирования, редактирования)
    bool GetDecryptedLogin(int index, const std::vector<unsigned char>& key, std::wstring& decryptedLogin);
    bool GetDecryptedPassword(int index, const std::vector<unsigned char>& key, std::wstring& decryptedPassword);
    // --- КОНЕЦ НОВОГО ---

    std::vector<PasswordTemplate>& GetEntries() { return m_entries; }
    const std::vector<PasswordTemplate>& GetEntries() const { return m_entries; }

private:
    std::vector<PasswordTemplate> m_entries;

    // --- НОВОЕ ---
    // Поле для хранения соли последней операции (для MainWindow)
    std::vector<unsigned char> m_lastUsedSalt;
public:
    const std::vector<unsigned char>& GetLastUsedSalt() const { return m_lastUsedSalt; }

private:
    std::vector<PasswordTemplate> m_entries;
    std::vector<unsigned char> m_lastUsedSalt;
};