#pragma once
#include <windows.h>
#include <string>

// Класс
class LoginDialog
{
public:
    // Методы для показа диалогов
    static bool ShowLogin(HINSTANCE hInstance, HWND parent, std::wstring& outPassword);
    static bool ShowNewPassword(HINSTANCE hInstance, HWND parent, std::wstring& outPassword);

    // Отдельные обработчики для каждого диалога
    static INT_PTR CALLBACK LoginDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK NewPasswordDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
    static void CheckCapsLock(HWND hDlg);
};