#pragma once
#include "CPassBase.h"
#include <windows.h>

class EditEntryDialog
{
public:
    static bool Show(HWND parent, PasswordTemplate& entry, bool isEdit = false);
    static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
    static PasswordTemplate* s_pEntry;  // Указатель на запись
    static bool s_isEdit;            // Режим редактирования
};