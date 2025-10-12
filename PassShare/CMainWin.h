#pragma once
#include <windows.h>

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
    int m_nCmdShow = 0;
};