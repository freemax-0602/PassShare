#include "framework.h"
#include "CMainWin.h"
#include "CLoginDialog.h"
#include "resource.h"
#include <windows.h>

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    if (!LoginDialog::Show(hInstance))
        return 0; 

    MainWindow mainWindow(hInstance, nCmdShow);
    if (!mainWindow.Create())
        return 1;

    MSG msg;
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PASSSHARE));

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(mainWindow.GetHwnd(), hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}