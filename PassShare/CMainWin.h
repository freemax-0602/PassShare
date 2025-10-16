#pragma once
#include <windows.h>
#include "CPassBase.h"

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
    WNDPROC m_oldEditProc = nullptr;
    bool m_isDatabaseLoaded = false;
    int m_nCmdShow = 0;

    PasswordDatabase m_db;
    std::wstring m_filePath;

    LRESULT HandleMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CreatePassBase(HWND hWnd);
    void OnLoadDatabase(HWND hWnd);
    void RefreshPasswordList();
    void OnSearchTextChanged();
    bool CopyPasswordToClipboard(HWND hWnd, const std::wstring& password);

    std::wstring m_cachedMasterPassword;
};