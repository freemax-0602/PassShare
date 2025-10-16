#include "CMainWin.h"
#include "resource.h"
#include <windowsx.h>
#include "CPassBase.h"
#include <commctrl.h>
#include "CEditDialog.h"
#include "CCryptoManage.h"
#include "CLoginDialog.h"
#include "CEnterPinDialog.h"

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

void MainWindow::RefreshPasswordList()
{
    // ✅ Отладочное сообщение
    // MessageBoxW(m_hwnd, L"RefreshPasswordList вызван", L"Debug", MB_OK);

    if (!m_hList)
    {
        // MessageBoxW(m_hwnd, L"m_hList == nullptr!", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    // ✅ Удаляем все старые элементы (даже если m_db пустой)
    ListView_DeleteAllItems(m_hList);

    // ✅ Добавляем записи из m_db
    int index = 0;
    for (const auto& entry : m_db.GetEntries())
    {
        LVITEMW item = { 0 };
        item.mask = LVIF_TEXT;
        item.iItem = index;
        item.iSubItem = 0; // Иконка
        item.pszText = const_cast<LPWSTR>(entry.icon.c_str());
        int itemIndex = ListView_InsertItem(m_hList, &item);

        ListView_SetItemText(m_hList, itemIndex, 1, const_cast<LPWSTR>(entry.url.c_str()));
        ListView_SetItemText(m_hList, itemIndex, 2, const_cast<LPWSTR>(entry.login.c_str()));
        ListView_SetItemText(m_hList, itemIndex, 3, const_cast<LPWSTR>(L"••••••••")); // Маскируем пароль
        ListView_SetItemText(m_hList, itemIndex, 4, const_cast<LPWSTR>(entry.description.c_str()));

        index++;
    }
}

void MainWindow::OnSearchTextChanged()
{
    if (!m_isDatabaseLoaded)
        return;

    wchar_t buffer[256];
    GetWindowTextW(m_hSearchEdit, buffer, _countof(buffer));

    std::wstring search = buffer;

    ListView_DeleteAllItems(m_hList);

    int index = 0;
    for (const auto& entry : m_db.GetEntries())
    {
        // Проверяем, содержит ли хотя бы одно поле искомый текст
        if (search.empty() ||
            entry.url.find(search) != std::wstring::npos ||
            entry.login.find(search) != std::wstring::npos ||
            entry.description.find(search) != std::wstring::npos)
        {
            LVITEMW item = { 0 };
            item.mask = LVIF_TEXT;
            item.iItem = index;
            item.iSubItem = 0;
            item.pszText = const_cast<LPWSTR>(entry.icon.c_str());
            ListView_InsertItem(m_hList, &item);

            ListView_SetItemText(m_hList, index, 1, const_cast<LPWSTR>(entry.url.c_str()));
            ListView_SetItemText(m_hList, index, 2, const_cast<LPWSTR>(entry.login.c_str()));
            ListView_SetItemText(m_hList, index, 3, const_cast<LPWSTR>(L"••••••••"));  // Пароль скрыт
            ListView_SetItemText(m_hList, index, 4, const_cast<LPWSTR>(entry.description.c_str()));

            index++;
        }
    }
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
        std::wstring masterPassword;

        if (!LoginDialog::ShowNewPassword(GetModuleHandle(NULL), hWnd, masterPassword))
        {
            return;
        }

        m_filePath = szFile;

        if (m_db.CreateEncryptedBase(szFile, masterPassword))
        {
            if (m_db.CreateEncryptedBase(szFile, masterPassword))
            {
                m_cachedMasterPassword = masterPassword;
                RefreshPasswordList();
                if (m_hList)
                {
                    ShowWindow(m_hList, SW_SHOW);
                }
                MessageBoxW(hWnd, L"Хранилище создано успешно!", L"Успех", MB_OK | MB_ICONINFORMATION);
            }

        }
        else
        {
            MessageBoxW(hWnd, L"Не удалось создать хранилище.", L"Ошибка", MB_OK | MB_ICONERROR);
        }

        // Очищаем пароль
        SecureZeroMemory(const_cast<wchar_t*>(masterPassword.c_str()), masterPassword.length() * sizeof(wchar_t));
    }
}

void MainWindow::OnLoadDatabase(HWND hWnd)
{
    OPENFILENAME ofn = { 0 };
    wchar_t szFile[MAX_PATH] = { 0 };

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    // Уточни фильтр, если используешь .enc.json
    ofn.lpstrFilter = L"Encrypted JSON Files\0*.enc.json\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn))
    {
        std::wstring masterPassword;

        
        // ✅ Используем ShowLogin, так как нужен один PIN без подтверждения
        if (!LoginDialog::ShowLogin(GetModuleHandle(NULL), hWnd, masterPassword))
        {
            return; // Пользователь отменил
        }

        m_filePath = szFile;

        if (m_db.LoadEncrypted(szFile, masterPassword))
        {
            m_cachedMasterPassword = masterPassword; // ✅ Сохраняем
            RefreshPasswordList();
            if (m_hList)
            {
                ShowWindow(m_hList, SW_SHOW); // <--- Показываем
                // Опционально: UpdateWindow(m_hList);
            }
            MessageBoxW(hWnd, L"База загружена успешно!", L"Успех", MB_OK | MB_ICONINFORMATION);
        }
        else
        {
            MessageBoxW(hWnd, L"Неверный пин-код или файл повреждён.", L"Ошибка", MB_OK | MB_ICONERROR);
        }

        // ✅ Очищаем пароль
        SecureZeroMemory(const_cast<wchar_t*>(masterPassword.c_str()), masterPassword.length() * sizeof(wchar_t));
    }
}

bool MainWindow::CopyPasswordToClipboard(HWND hWnd, const std::wstring& password)
{
    if (!OpenClipboard(hWnd))
        return false;

    EmptyClipboard();

    size_t len = (password.length() + 1) * sizeof(wchar_t);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
    if (!hMem)
    {
        CloseClipboard();
        return false;
    }

    void* pMem = GlobalLock(hMem);
    if (!pMem)
    {
        GlobalFree(hMem);
        CloseClipboard();
        return false;
    }

    wcscpy_s(static_cast<wchar_t*>(pMem), password.length() + 1, password.c_str());
    GlobalUnlock(hMem);

    bool success = SetClipboardData(CF_UNICODETEXT, hMem) != NULL;

    if (!success)
    {
        GlobalFree(hMem);
    }

    CloseClipboard();

    if (!success)
    {
        GlobalFree(hMem);
    }

    return success;
}


//===========================================================================================================================
// ========================================== MAIN WINDOW ===================================================================
// ==========================================================================================================================
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
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, m_hInstance, this);

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
        // Поле поиска
        HWND hSearch = CreateWindowW(L"EDIT", NULL,
            ES_LEFT | WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP,
            10, 10, 400, 25,
            hWnd, (HMENU)IDC_SEARCH_EDIT, m_hInstance, NULL);

        if (!hSearch)
        {
            MessageBoxW(hWnd, L"Не удалось создать поле поиска", L"Ошибка", MB_OK | MB_ICONERROR);
            return -1;
        }

        m_hSearchEdit = hSearch;
        
        // Устанавливаем placeholder текст
        SendMessageW(m_hSearchEdit, EM_SETCUEBANNER, FALSE, (LPARAM)L"Найти...");

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
        ShowWindow(m_hList, SW_HIDE);

        // Создаём ImageList с размером 32x32
        HIMAGELIST hImageList = ImageList_Create(32, 32, ILC_COLOR32, 0, 0);
        ListView_SetImageList(hList, hImageList, LVSIL_SMALL);  // Привязываем

        ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

        // Добавляем колонки
        LVCOLUMNW col = { 0 };
        col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

        col.cx = 60;
        col.pszText = const_cast<LPWSTR>(L"Значок");
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

            MoveWindow(m_hSearchEdit, 10, 10, width - 20, 25, TRUE);
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
    case WM_NOTIFY:
    {
        NMHDR* pnmh = reinterpret_cast<NMHDR*>(lParam);
        if (pnmh->hwndFrom == m_hList && pnmh->code == NM_DBLCLK)
        {
            NMITEMACTIVATE* pia = reinterpret_cast<NMITEMACTIVATE*>(lParam);
            int index = pia->iItem;

            if (index >= 0 && index < static_cast<int>(m_db.GetEntries().size()))
            {
                // Редактирование
                PasswordTemplate& entry = m_db.GetEntries()[index];
                if (EditEntryDialog::Show(m_hwnd, entry, true))
                {
                    RefreshPasswordList();
                    // ✅ Сохраняем с кэшированным паролем
                    if (!m_filePath.empty() && !m_cachedMasterPassword.empty())
                    {
                        m_db.SaveEncrypted(m_filePath, m_cachedMasterPassword);
                    }
                }
            }
            else
            {
                // Добавление
                PasswordTemplate newEntry = {};
                if (EditEntryDialog::Show(m_hwnd, newEntry, false))
                {
                    m_db.GetEntries().push_back(newEntry);
                    RefreshPasswordList();
                    // ✅ Сохраняем с кэшированным паролем
                    if (!m_filePath.empty() && !m_cachedMasterPassword.empty())
                    {
                        m_db.SaveEncrypted(m_filePath, m_cachedMasterPassword);
                    }
                }
            }
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
        case ID_LOAD_BASE_BTN:
            OnLoadDatabase(hWnd);
            break;
        case IDC_SEARCH_EDIT:
            if (HIWORD(wParam) == EN_CHANGE)
            {
                OnSearchTextChanged();
            }
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
        // ✅ Очищаем кэшированный пароль при закрытии
        if (!m_cachedMasterPassword.empty())
        {
            SecureZeroMemory(const_cast<wchar_t*>(m_cachedMasterPassword.c_str()), m_cachedMasterPassword.length() * sizeof(wchar_t));
            m_cachedMasterPassword.clear();
        }
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}