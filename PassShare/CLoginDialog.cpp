#include "CLoginDialog.h"
#include "resource.h"
#include <windowsx.h>
#include <strsafe.h>

bool LoginDialog::ShowLogin(HINSTANCE hInstance, HWND parent, std::wstring& outPassword)
{
    // Передаём указатель на строку для возврата пароля
    std::wstring* passwordPtr = &outPassword;

    INT_PTR result = DialogBoxParamW(
        hInstance,
        MAKEINTRESOURCEW(IDD_DIALOG_LOGIN), // Используем диалог авторизации
        parent,
        LoginDialogProc, // Используем соответствующий обработчик
        reinterpret_cast<LPARAM>(passwordPtr));

    return result == IDOK;
}

bool LoginDialog::ShowNewPassword(HINSTANCE hInstance, HWND parent, std::wstring& outPassword)
{
    // Передаём указатель на строку для возврата пароля
    std::wstring* passwordPtr = &outPassword;

    INT_PTR result = DialogBoxParamW(
        hInstance,
        MAKEINTRESOURCEW(IDD_NEW_PASSSHARE_DIALOG), // Используем диалог создания пароля
        parent,
        NewPasswordDialogProc, // Используем соответствующий обработчик
        reinterpret_cast<LPARAM>(passwordPtr));

    return result == IDOK;
}

INT_PTR CALLBACK LoginDialog::LoginDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static std::wstring* s_password = nullptr; // Указатель на строку для возврата пароля

    switch (message)
    {
    case WM_INITDIALOG:
    {
        // Сохраняем указатель на строку пароля
        s_password = reinterpret_cast<std::wstring*>(lParam);
        return TRUE;
    }

    case WM_COMMAND:
        // Обработка Caps Lock
        if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == IDC_PASSWORD_EDIT)
        {
            CheckCapsLock(hDlg);
            return TRUE;
        }

        if (LOWORD(wParam) == IDC_LOGIN_BUTTON) // ID кнопки для этого диалога
        {
            wchar_t password[256] = { 0 };
            // ✅ Правильный ID поля ввода для этого диалога
            GetDlgItemTextW(hDlg, IDC_CONFIRM_LOGIN_PASSWORD_EDIT, password, _countof(password));

            if (wcslen(password) == 0)
            {
                MessageBoxW(hDlg, L"Введите пин-код!", L"Внимание", MB_ICONWARNING);
                return TRUE;
            }

            // Сохраняем введённый пароль
            if (s_password)
            {
                s_password->assign(password);
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

INT_PTR CALLBACK LoginDialog::NewPasswordDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static std::wstring* s_password = nullptr; // Указатель на строку для возврата пароля

    switch (message)
    {
    case WM_INITDIALOG:
    {
        // Сохраняем указатель на строку пароля
        s_password = reinterpret_cast<std::wstring*>(lParam);
        return TRUE;
    }

    case WM_COMMAND:
        // Обработка Caps Lock
        if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == IDC_PASSWORD_EDIT)
        {
            CheckCapsLock(hDlg);
            return TRUE;
        }

        if (LOWORD(wParam) == IDC_LOGIN_NEW_BUTTON) // ID кнопки для этого диалога
        {
            wchar_t password[256] = { 0 };
            // ✅ Правильный ID основного поля ввода
            GetDlgItemTextW(hDlg, IDC_PASSWORD_EDIT, password, _countof(password));

            if (wcslen(password) == 0)
            {
                MessageBoxW(hDlg, L"Введите пин-код!", L"Внимание", MB_ICONWARNING);
                return TRUE;
            }

            // ✅ Проверка подтверждения (только для этого диалога)
            wchar_t confirmPassword[256] = { 0 };
            // ✅ Правильный ID поля подтверждения
            GetDlgItemTextW(hDlg, IDC_CONFIRM_PASSWORD_EDIT, confirmPassword, _countof(confirmPassword));

            if (wcscmp(password, confirmPassword) != 0)
            {
                MessageBoxW(hDlg, L"Пароли не совпадают!", L"Ошибка", MB_ICONERROR);
                return TRUE;
            }

            // Сохраняем введённый пароль
            if (s_password)
            {
                s_password->assign(password);
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

//INT_PTR CALLBACK LoginDialog::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
//{
//    static DialogData* s_data = nullptr;
//    switch (message)
//    {
//    case WM_INITDIALOG:
//    {
//        s_data = reinterpret_cast<DialogData*>(lParam); // ✅ Сохраняем указатель
//
//        if (s_data && s_data->isNewPasswordMode)
//        {
//            SetWindowTextW(hDlg, L"Введите мастер-пароль для нового хранилища");
//        }
//
//        return TRUE;
//    }
//
//    case WM_COMMAND:
//        
//        if (s_data)
//        {
//            if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == IDC_PASSWORD_EDIT)
//            {
//                CheckCapsLock(hDlg);
//                return TRUE;
//            }
//
//            if (LOWORD(wParam) == IDC_LOGIN_BUTTON || LOWORD(wParam) == IDC_LOGIN_NEW_BUTTON)
//            {
//                wchar_t password[256] = { 0 };
//                GetDlgItemTextW(hDlg, IDC_PASSWORD_EDIT, password, _countof(password));
//
//                if (wcslen(password) == 0)
//                {
//                    MessageBoxW(hDlg, L"Введите пин-код!", L"Внимание", MB_ICONWARNING);
//                    return TRUE;
//                }
//
//                if (s_data->isNewPasswordMode)
//                {
//                    wchar_t confirmPassword[256] = { 0 };
//                    GetDlgItemTextW(hDlg, IDC_CONFIRM_PASSWORD_EDIT, confirmPassword, _countof(confirmPassword));
//
//                    if (wcscmp(password, confirmPassword) != 0)
//                    {
//                        MessageBoxW(hDlg, L"Пароли не совпадают!", L"Ошибка", MB_ICONERROR);
//                        return TRUE;
//                    }
//                }
//
//                if (s_data->password)
//                {
//                    s_data->password->assign(password);
//                }
//
//                EndDialog(hDlg, IDOK);
//                return TRUE;
//            }
//            else if (LOWORD(wParam) == IDCANCEL)
//            {
//                EndDialog(hDlg, IDCANCEL);
//                return TRUE;
//            }
//        }
//        break;
//
//    case WM_CLOSE:
//        EndDialog(hDlg, IDCANCEL);
//        return TRUE;
//    }
//    return FALSE;
//}
//
//bool LoginDialog::ShowNewPassword(HINSTANCE hInstance, HWND parent, std::wstring& outPassword)
//{
//    DialogData data;
//    data.password = &outPassword;
//    data.isNewPasswordMode = true; // ✅ Устанавливаем режим
//
//    INT_PTR result = DialogBoxParamW(hInstance, MAKEINTRESOURCEW(IDD_NEW_PASSSHARE_DIALOG), parent, DialogProc, reinterpret_cast<LPARAM>(&data));
//    return result == IDOK;
//}
//
//bool LoginDialog::ShowPassword(HINSTANCE hInstance, HWND parent, std::wstring& outPassword)
//{
//    DialogData data;
//    data.password = &outPassword;
//    data.isNewPasswordMode = false; 
//
//    INT_PTR result = DialogBoxParamW(hInstance, MAKEINTRESOURCEW(IDD_DIALOG_LOGIN), parent, DialogProc, reinterpret_cast<LPARAM>(&data));
//    return result == IDOK;
//}

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