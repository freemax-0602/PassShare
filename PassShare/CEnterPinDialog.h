#pragma once
#include <windows.h>
#include <string>

class EnterPinDialog
{
public:
    // Показывает диалог и возвращает введённый PIN
    static bool Show(HWND parent, std::wstring& outPin);

    static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
    static std::wstring* s_pin; // Указатель на строку для возврата PIN
};