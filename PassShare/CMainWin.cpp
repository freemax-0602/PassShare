#include "CMainWin.h"
#include "resource.h"
#include <windowsx.h>
#include "CPassBase.h"
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

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
        m_filePath = szFile;
        m_db.CreateBase(szFile);

        if (m_db.CreateBase(szFile))
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

    // Вот тут важно: передаём `this` в lpCreateParams
    m_hwnd = CreateWindowW(wcex.lpszClassName, L"PassShare", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, m_hInstance, this); // ✅

    if (!m_hwnd)
        return false;

    ShowWindow(m_hwnd, m_nCmdShow);
    UpdateWindow(m_hwnd);

    return true;
}

LRESULT CALLBACK MainWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    MainWindow* pThis = nullptr;

    if (message == WM_NCCREATE)
    {
        pThis = static_cast<MainWindow*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    }
    else
    {
        pThis = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    if (pThis)
    {
        return pThis->HandleMessages(hWnd, message, wParam, lParam);
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT MainWindow::HandleMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        HWND hList = CreateWindowW(WC_LISTVIEWW, NULL,
            LVS_REPORT | WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_SINGLESEL,
            10, 50, 600, 400,
            hWnd, (HMENU)IDC_PASSWORD_LIST, m_hInstance, NULL);

        if (!hList)
        {
            MessageBoxW(hWnd, L"Не удалось создать ListView", L"Ошибка", MB_OK | MB_ICONERROR);
            return -1;
        }

        m_hList = hList;

        // Создаём ImageList с размером 32x32
        HIMAGELIST hImageList = ImageList_Create(32, 32, ILC_COLOR32, 0, 0);
        ListView_SetImageList(hList, hImageList, LVSIL_SMALL);  // Привязываем

        ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

        // Добавляем колонки
        LVCOLUMNW col = { 0 };
        col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

        col.cx = 60;
        col.pszText = const_cast<LPWSTR>(L"Иконка");
        ListView_InsertColumn(hList, 0, &col);

        col.cx = 150;
        col.pszText = const_cast<LPWSTR>(L"Адрес");
        ListView_InsertColumn(hList, 1, &col);

        col.cx = 150;
        col.pszText = const_cast<LPWSTR>(L"Логин");
        ListView_InsertColumn(hList, 2, &col);

        col.cx = 150;
        col.pszText = const_cast<LPWSTR>(L"Пароль");
        ListView_InsertColumn(hList, 3, &col);

        col.cx = 0;
        col.pszText = const_cast<LPWSTR>(L"Описание");
        ListView_InsertColumn(hList, 4, &col);

        break;
    }
    case WM_SIZE:
    {
        if (m_hList)
        {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);

            MoveWindow(m_hList, 10, 50, width - 20, height - 60, TRUE);

            RECT rc;
            GetClientRect(m_hList, &rc);
            int totalWidth = rc.right - rc.left;

            // Устанавливаем ширину колонок
            ListView_SetColumnWidth(m_hList, 0, 60);
            ListView_SetColumnWidth(m_hList, 1, 150);
            ListView_SetColumnWidth(m_hList, 2, 150);
            ListView_SetColumnWidth(m_hList, 3, 150);
            ListView_SetColumnWidth(m_hList, 4, totalWidth - (60 + 150 + 150 + 150));  // Описание — остаток
        }
        break;
    }
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