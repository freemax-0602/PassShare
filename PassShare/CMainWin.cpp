#include "CMainWin.h"
#include "resource.h"
#include <windowsx.h>
#include "CPassBase.h"

MainWindow::MainWindow(HINSTANCE hInstance, int nCmdShow)
    : m_hInstance(hInstance), m_nCmdShow(nCmdShow)
{
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void MainWindow::CreatePassBase(HWND hWnd)
{
    // Открываем диалог сохранения файла
    OPENFILENAME ofn = { 0 };
    wchar_t szFile[MAX_PATH] = { 0 };

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"JSON Files\0*.json\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn))
    {
        // Путь к файлу в szFile
        PasswordDatabase db;
        if (db.CreateNew(szFile))
        {
            MessageBoxW(hWnd, L"Хранилище создано успешно!", L"Успех", MB_OK | MB_ICONINFORMATION);
        }
        else
        {
            MessageBoxW(hWnd, L"Не удалось создать хранилище.", L"Ошибка", MB_OK | MB_ICONERROR);
        }
    }
}

bool MainWindow::Create()
{
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = m_hInstance;
    wcex.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_PASSSHARE));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PASSSHARE);
    wcex.lpszClassName = L"PassShareMainWindow";
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    if (!RegisterClassExW(&wcex))
        return false;

    m_hwnd = CreateWindowW(wcex.lpszClassName, L"PassShare", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, m_hInstance, nullptr);

    if (!m_hwnd)
        return false;

    ShowWindow(m_hwnd, m_nCmdShow);
    UpdateWindow(m_hwnd);

    return true;
}

LRESULT CALLBACK MainWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case ID_ADD_BASE_BTN:
            CreatePassBase(hWnd);
            break;
        case IDM_ABOUT:
            DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code here...
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}