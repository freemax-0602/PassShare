#pragma once
#include <windows.h>

// Класс
class LoginDialog
{
public:
    static bool Show(HINSTANCE hInstance, HWND parent = NULL);
    static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
    static bool ValidatePassword(const wchar_t* password);
    static void CheckCapsLock(HWND hDlg);
};