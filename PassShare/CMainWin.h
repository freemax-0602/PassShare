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
    HWND m_hEdit = NULL;
    int m_iEditItem = -1;
    int m_iEditSubItem = -1;
    WNDPROC m_oldEditProc = nullptr;

    int m_nCmdShow = 0;
    // Поля, которые понадобятся
    PasswordDatabase m_db;
    std::wstring m_filePath;

    LRESULT HandleMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK EditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void CreatePassBase(HWND hWnd);
    //void OnLoadDatabase(HWND hWnd);
   // void OnAddEntry(HWND hWnd);
    void OnEditLabel(HWND hWnd, int iItem, int iSubItem);
    void EndEditLabel(bool save);
};