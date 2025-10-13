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
    int m_nCmdShow = 0;
    // Поля, которые понадобятся
    PasswordDatabase m_db;
    std::wstring m_filePath;

    LRESULT HandleMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CreatePassBase(HWND hWnd);
    //void OnLoadDatabase(HWND hWnd);
};