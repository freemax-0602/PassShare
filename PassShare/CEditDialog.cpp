#include "CEditDialog.h"
#include "resource.h"
#include <windowsx.h>

PasswordTemplate* EditEntryDialog::s_pEntry = nullptr;
bool EditEntryDialog::s_isEdit = false;

bool EditEntryDialog::Show(HWND parent, PasswordTemplate& entry, bool isEdit)
{
    s_pEntry = &entry;
    s_isEdit = isEdit;

    INT_PTR result = DialogBoxParamW(
        GetModuleHandle(NULL),
        MAKEINTRESOURCEW(IDD_DIALOG_ENTRY),
        parent,
        DialogProc,
        0);

    return result == IDOK;
}

INT_PTR CALLBACK EditEntryDialog::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
    {
        if (s_pEntry)
        {
            //SetDlgItemTextW(hDlg, IDC_EDIT_ICON, s_pEntry->icon.c_str());
            SetDlgItemTextW(hDlg, IDC_EDIT_URL, s_pEntry->url.c_str());
            SetDlgItemTextW(hDlg, IDC_EDIT_LOGIN, s_pEntry->login.c_str());
            SetDlgItemTextW(hDlg, IDC_EDIT_PASS, s_pEntry->password.c_str());
            SetDlgItemTextW(hDlg, IDC_EDIT_DESCRIPTION, s_pEntry->description.c_str());
        }
        return TRUE;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            if (s_pEntry)
            {
                wchar_t buffer[256];

                //GetDlgItemTextW(hDlg, IDC_EDIT_ICON, buffer, _countof(buffer));
                //s_pEntry->icon = buffer;

                GetDlgItemTextW(hDlg, IDC_EDIT_URL, buffer, _countof(buffer));
                s_pEntry->url = buffer;

                GetDlgItemTextW(hDlg, IDC_EDIT_LOGIN, buffer, _countof(buffer));
                s_pEntry->login = buffer;

                //GetDlgItemTextW(hDlg, IDC_EDIT_PASSWORD, buffer, _countof(buffer));
                s_pEntry->password = buffer;

                //GetDlgItemTextW(hDlg, IDC_EDIT_DESCRIPTION, buffer, _countof(buffer));
                s_pEntry->description = buffer;
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