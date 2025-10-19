#pragma once
#include <windows.h>
#include <vector>
#include "CPassBase.h" // Обновлённая структура
#include "CPassTemplate.h"

class MainWindow
{
public:
    MainWindow(HINSTANCE hInstance, int nCmdShow);
    HWND GetHwnd() const { return m_hwnd; }
    bool Create();
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    HWND m_hwnd = NULL;
    HINSTANCE m_hInstance = NULL;
    HWND m_hList = NULL;
    HWND m_hSearchEdit = NULL;
    // WNDPROC m_oldEditProc = nullptr; // Удалено, не используется
    bool m_isDatabaseLoaded = false; // Удалено, не используется напрямую
    int m_nCmdShow = 0;

    PasswordDatabase m_db;
    std::wstring m_filePath;

    // --- НОВОЕ ---
    // Кэшируем мастер-пароль и соль файла для второго уровня шифрования
    std::wstring m_cachedMasterPassword;
    std::vector<unsigned char> m_fileSalt; // <-- Новое поле
    // --- КОНЕЦ НОВОГО ---

    LRESULT HandleMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CreatePassBase(HWND hWnd);
    void OnLoadDatabase(HWND hWnd);
    void RefreshPasswordList();
    void OnSearchTextChanged();
    // bool CopyPasswordToClipboard(HWND hWnd, const std::wstring& password); // Старая сигнатура
    // --- НОВОЕ ---
    bool CopyPasswordToClipboard(HWND hWnd, int itemIndex); // Новая сигнатура: принимает индекс
    bool GetDecryptedPasswordForItem(int itemIndex, std::wstring& decryptedPassword); // Вспомогательный метод
    // --- КОНЕЦ НОВОГО ---
};