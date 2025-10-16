#include "CEnterPinDialog.h"
#include "resource.h"
#include <windowsx.h>
#include <strsafe.h>

// Инициализация статической переменной
std::wstring* EnterPinDialog::s_pin = nullptr;

bool EnterPinDialog::Show(HWND parent, std::wstring& outPin)
{
    s_pin = &outPin; // Сохраняем указатель

    INT_PTR result = DialogBoxW(
        GetModuleHandle(NULL),
        MAKEINTRESOURCEW(IDD_DIALOG_LOGIN),
        parent,
        DialogProc);

    s_pin = nullptr; // Сбрасываем
    return result == IDOK;
}

INT_PTR CALLBACK EnterPinDialog::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            wchar_t pin[256] = { 0 };
            GetDlgItemTextW(hDlg, IDD_DIALOG_LOGIN, pin, _countof(pin));

            if (wcslen(pin) == 0)
            {
                MessageBoxW(hDlg, L"Введите PIN-код!", L"Внимание", MB_ICONWARNING);
                return TRUE;
            }

            // Сохраняем введённый PIN
            if (s_pin)
            {
                s_pin->assign(pin);
            }

            EndDialog(hDlg, IDOK);
            return TRUE;
        }
        else if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;

    case WM_CLOSE:
        EndDialog(hDlg, IDCANCEL);
        return TRUE;
    }
    return FALSE;
}