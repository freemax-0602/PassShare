#include "CLoginDialog.h"
#include "resource.h"
#include <windowsx.h>
#include <strsafe.h>

bool LoginDialog::Show(HINSTANCE hInstance, HWND parent)
{
    INT_PTR result = DialogBoxParamW(hInstance, MAKEINTRESOURCEW(IDD_DIALOG_LOGIN), parent, DialogProc, 0);
    return result == IDOK;
}

INT_PTR CALLBACK LoginDialog::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
        
        if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == IDC_PASSWORD_EDIT)
        {
            CheckCapsLock(hDlg);
            return TRUE;
        }

        if (LOWORD(wParam) == IDC_LOGIN_BUTTON)
        {
            wchar_t password[256] = { 0 };
            GetDlgItemTextW(hDlg, IDC_PASSWORD_EDIT, password, _countof(password));

            if (wcslen(password) == 0)
            {
                MessageBoxW(hDlg, L"Введите пин-код!", L"Внимание", MB_ICONWARNING);
                return TRUE;
            }

            if (ValidatePassword(password))
            {
                EndDialog(hDlg, IDOK);
            }
            else
            {
                MessageBoxW(hDlg, L"Неверный пин-код!", L"Ошибка", MB_ICONERROR);
            }
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

bool LoginDialog::ValidatePassword(const wchar_t* password)
{
    // Пока хардкод, позже будет шифрование !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    return wcscmp(password, L"12345") == 0;
}

void LoginDialog::CheckCapsLock(HWND hDlg)
{
    HWND hLabel = GetDlgItem(hDlg, IDC_CAPSLOCK_WARNING);
    if (hLabel == NULL)
        return;

    if (GetKeyState(VK_CAPITAL) & 0x0001)
    {
        ShowWindow(hLabel, SW_SHOW);
    }
    else
    {
        ShowWindow(hLabel, SW_HIDE);
    }
}