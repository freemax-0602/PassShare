#include "CMainWin.h"
#include "resource.h"
#include <windowsx.h>
#include "CPassBase.h"
#include <commctrl.h>
#include "CEditDialog.h"
#include "CCryptoManage.h"
#include "CLoginDialog.h"
#include "CEnterPinDialog.h"
#include <codecvt>
#include "CPassBase.h"

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
    // MessageBoxW(m_hwnd, L"RefreshPasswordList вызван", L"Debug", MB_OK); // Отладка

    if (!m_hList)
    {
        // MessageBoxW(m_hwnd, L"m_hList == nullptr!", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    ListView_DeleteAllItems(m_hList);

    int index = 0;
    for (const auto& entry : m_db.GetEntries())
    {
        LVITEMW item = { 0 };
        item.mask = LVIF_TEXT;
        item.iItem = index;
        item.iSubItem = 0; // url
        item.pszText = const_cast<LPWSTR>(entry.url.c_str());
        int itemIndex = ListView_InsertItem(m_hList, &item);

        // Отображаем открытые поля
        ListView_SetItemText(m_hList, itemIndex, 0, const_cast<LPWSTR>(entry.url.c_str())); // Адрес
        ListView_SetItemText(m_hList, itemIndex, 1, const_cast<LPWSTR>(L"***")); // Логин (маскирован)
        ListView_SetItemText(m_hList, itemIndex, 2, const_cast<LPWSTR>(L"••••••••")); // Пароль (маскирован)
        ListView_SetItemText(m_hList, itemIndex, 3, const_cast<LPWSTR>(entry.description.c_str())); // Описание

        index++;
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
    ofn.lpstrFilter = L"Encrypted JSON Files\0*.enc.json\0All Files\0*.*\0";
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
            return; // Пользователь отменил
        }

        m_filePath = szFile;

        if (m_db.CreateEncryptedBase(szFile, masterPassword))
        {
            // --- НОВОЕ ---
            // Сохраняем соль файла для последующего использования
            m_fileSalt = m_db.GetLastUsedSalt();
            // Сохраняем мастер-пароль для генерации ключа второго уровня
            m_cachedMasterPassword = masterPassword;
            // --- КОНЕЦ НОВОГО ---

            //if (!m_hSearch)
            //{
            //    CreateSearchControl(hWnd); // Предполагается, что этот метод существует
            //}
            RefreshPasswordList();
            MessageBoxW(hWnd, L"Хранилище создано успешно!", L"Успех", MB_OK | MB_ICONINFORMATION);
        }
        else
        {
            MessageBoxW(hWnd, L"Не удалось создать хранилище.", L"Ошибка", MB_OK | MB_ICONERROR);
        }

        // ✅ Очищаем пароль
        SecureZeroMemory(const_cast<wchar_t*>(masterPassword.c_str()), masterPassword.length() * sizeof(wchar_t));
    }
}

//void MainWindow::OnSearchTextChanged()
//{
//    if (!m_isDatabaseLoaded)
//        return;
//
//    wchar_t buffer[256];
//    GetWindowTextW(m_hSearchEdit, buffer, _countof(buffer));
//
//    std::wstring search = buffer;
//
//    ListView_DeleteAllItems(m_hList);
//
//    int index = 0;
//    for (const auto& entry : m_db.GetEntries())
//    {
//        // Проверяем, содержит ли хотя бы одно поле искомый текст
//        if (search.empty() ||
//            entry.url.find(search) != std::wstring::npos ||
//            entry.login.find(search) != std::wstring::npos ||
//            entry.description.find(search) != std::wstring::npos)
//        {
//            LVITEMW item = { 0 };
//            item.mask = LVIF_TEXT;
//            item.iItem = index;
//            item.iSubItem = 0;
//            item.pszText = const_cast<LPWSTR>(entry.icon.c_str());
//            ListView_InsertItem(m_hList, &item);
//
//            ListView_SetItemText(m_hList, index, 1, const_cast<LPWSTR>(entry.url.c_str()));
//            ListView_SetItemText(m_hList, index, 2, const_cast<LPWSTR>(entry.login.c_str()));
//            ListView_SetItemText(m_hList, index, 3, const_cast<LPWSTR>(L"••••••••"));  // Пароль скрыт
//            ListView_SetItemText(m_hList, index, 4, const_cast<LPWSTR>(entry.description.c_str()));
//
//            index++;
//        }
//    }
//}

void MainWindow::CreatePassBase(HWND hWnd)
{
    OPENFILENAME ofn = { 0 };
    wchar_t szFile[MAX_PATH] = { 0 };

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Encrypted JSON Files\0*.enc.json\0All Files\0*.*\0";
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
            return; // Пользователь отменил
        }

        m_filePath = szFile;

        if (m_db.CreateEncryptedBase(szFile, masterPassword))
        {
            // --- НОВОЕ ---
            // Сохраняем соль файла для последующего использования
            m_fileSalt = m_db.GetLastUsedSalt();
            // Сохраняем мастер-пароль для генерации ключа второго уровня
            m_cachedMasterPassword = masterPassword;
            // --- КОНЕЦ НОВОГО ---

            RefreshPasswordList();
            MessageBoxW(hWnd, L"Хранилище создано успешно!", L"Успех", MB_OK | MB_ICONINFORMATION);
        }
        else
        {
            MessageBoxW(hWnd, L"Не удалось создать хранилище.", L"Ошибка", MB_OK | MB_ICONERROR);
        }

        // ✅ Очищаем пароль
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
            // --- НОВОЕ ---
            // Сохраняем соль файла для последующего использования
            m_fileSalt = m_db.GetLastUsedSalt();
            // Сохраняем мастер-пароль для генерации ключа второго уровня
            m_cachedMasterPassword = masterPassword;
            // --- КОНЕЦ НОВОГО ---

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

bool MainWindow::CopyPasswordToClipboard(HWND hWnd, int itemIndex) // <-- Новая сигнатура
{
    // 1. Проверка корректности индекса (проводится внутри GetDecryptedPasswordForItem)
    // if (itemIndex < 0 || itemIndex >= static_cast<int>(m_db.GetEntries().size())) ...

    // 2. Проверка наличия кэшированного мастер-пароля (проводится внутри GetDecryptedPasswordForItem)
    // if (m_cachedMasterPassword.empty()) ...

    std::wstring decryptedPassword;

    // 3. Получаем расшифрованный пароль
    if (!GetDecryptedPasswordForItem(itemIndex, decryptedPassword)) // <-- Используем новый метод
    {
        MessageBoxW(hWnd, L"Не удалось расшифровать пароль. Проверьте PIN.", L"Ошибка", MB_OK | MB_ICONERROR);
        return false;
    }

    // 4. Копируем в буфер обмена (используем старую логику)
    if (!OpenClipboard(hWnd))
        return false;

    EmptyClipboard();

    size_t len = (decryptedPassword.length() + 1) * sizeof(wchar_t);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
    if (!hMem)
    {
        CloseClipboard();
        // Очищаем временный пароль на случай, если GlobalAlloc упал по памяти
        SecureZeroMemory(const_cast<wchar_t*>(decryptedPassword.c_str()), decryptedPassword.length() * sizeof(wchar_t));
        return false;
    }

    void* pMem = GlobalLock(hMem);
    if (!pMem)
    {
        GlobalFree(hMem);
        CloseClipboard();
        // Очищаем временный пароль
        SecureZeroMemory(const_cast<wchar_t*>(decryptedPassword.c_str()), decryptedPassword.length() * sizeof(wchar_t));
        return false;
    }

    wcscpy_s(static_cast<wchar_t*>(pMem), decryptedPassword.length() + 1, decryptedPassword.c_str());
    GlobalUnlock(hMem);

    bool success = SetClipboardData(CF_UNICODETEXT, hMem) != NULL;

    if (!success)
    {
        GlobalFree(hMem);
    }

    CloseClipboard();

    // 5. Очищаем временный пароль сразу после копирования
    SecureZeroMemory(const_cast<wchar_t*>(decryptedPassword.c_str()), decryptedPassword.length() * sizeof(wchar_t));
    // decryptedPassword.clear(); // Не обязательно, выход из функции уничтожит его

    // 6. Возвращаем результат
    if (success)
    {
        // Опционально: сообщение пользователю
        // MessageBoxW(hWnd, L"Пароль скопирован!", L"Успех", MB_OK | MB_ICONINFORMATION);
    }
    else
    {
        MessageBoxW(hWnd, L"Не удалось скопировать пароль в буфер обмена.", L"Ошибка", MB_OK | MB_ICONERROR);
    }

    return success;
}

bool MainWindow::GetDecryptedPasswordForItem(int itemIndex, std::wstring& decryptedPassword)
{
    // 1. Проверка корректности индекса
    if (itemIndex < 0 || itemIndex >= static_cast<int>(m_db.GetEntries().size()))
    {
        return false;
    }

    // 2. Проверка наличия кэшированного мастер-пароля
    if (m_cachedMasterPassword.empty())
    {
        // Логика: если пароль не кэширован, нужно запросить его.
        // Однако, в текущей архитектуре он кэшируется при загрузке/создании.
        // Если он пуст, это может означать проблему.
        // MessageBoxW(m_hwnd, L"Мастер-пароль не доступен. Перезагрузите базу.", L"Ошибка", MB_OK | MB_ICONERROR);
        return false;
    }

    // 3. Проверка наличия соли файла
    if (m_fileSalt.empty())
    {
        // Это критическая ошибка - соль должна быть после загрузки
        // MessageBoxW(m_hwnd, L"Соль файла отсутствует.", L"Ошибка", MB_OK | MB_ICONERROR);
        return false;
    }

    // 4. Генерация ключа для второго уровня шифрования
    // Используем ТУ ЖЕ соль, что и для шифрования файла, и ТОТ ЖЕ мастер-пароль.
    std::vector<unsigned char> decryptionKey;
    if (!CryptoManager::DeriveKeyFromPassword(m_cachedMasterPassword, m_fileSalt, decryptionKey))
    {
        // Ошибка генерации ключа
        // MessageBoxW(m_hwnd, L"Не удалось сгенерировать ключ для расшифровки.", L"Ошибка", MB_OK | MB_ICONERROR);
        return false; // Ошибка генерации ключа
    }

    // 5. Вызов метода базы данных для расшифровки
    bool result = m_db.GetDecryptedPassword(itemIndex, decryptionKey, decryptedPassword);

    // 6. Очистка ключа сразу после использования (важно для безопасности)
    CryptoManager::SecureClear(decryptionKey);

    // 7. Возврат результата
    // result будет true, если расшифровка прошла успешно, и false - если ошибка
    return result;
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
        // ... (все предыдущие case: WM_CREATE, WM_SIZE, WM_COMMAND для меню и т.д.) ...

    case WM_NOTIFY:
    {
        NMHDR* pnmh = reinterpret_cast<NMHDR*>(lParam);
        if (pnmh->hwndFrom == m_hList && pnmh->code == NM_DBLCLK)
        {
            NMITEMACTIVATE* pia = reinterpret_cast<NMITEMACTIVATE*>(lParam);
            int index = pia->iItem;

            // --- Проверяем, кликнули ли по колонке "Пароль" (индекс 3) ---
            if (pia->iSubItem == 3) // Предполагаем: 0=Значок, 1=Адрес, 2=Логин, 3=Пароль, 4=Описание
            {
                if (index >= 0 && index < static_cast<int>(m_db.GetEntries().size()))
                {
                    // --- КОПИРОВАНИЕ ПАРОЛЯ ---
                    // Вызываем новый метод, который сам запрашивает PIN и расшифровывает
                    if (!CopyPasswordToClipboard(hWnd, index)) // <-- Используем индекс
                    {
                        // Сообщение об ошибке уже внутри CopyPasswordToClipboard
                    }
                    else
                    {
                        // Опционально: сообщение об успехе
                        // MessageBoxW(hWnd, L"Пароль скопирован!", L"Успех", MB_OK | MB_ICONINFORMATION);
                    }
                }
                break; // Обработали клик по паролю, выходим
            }
            // --- КОНЕЦ КОПИРОВАНИЯ ПАРОЛЯ ---

            // --- РЕДАКТИРОВАНИЕ ИЛИ ДОБАВЛЕНИЕ ---
            if (index >= 0 && index < static_cast<int>(m_db.GetEntries().size()))
            {
                // --- РЕДАКТИРОВАНИЕ ---
                // 1. Проверяем наличие мастер-пароля и соли
                if (m_cachedMasterPassword.empty() || m_fileSalt.empty())
                {
                    MessageBoxW(hWnd, L"Невозможно редактировать запись: мастер-пароль или соль недоступны. Перезагрузите базу.", L"Ошибка", MB_OK | MB_ICONERROR);
                    break;
                }

                // 2. Генерируем ключ
                std::vector<unsigned char> editKey;
                if (!CryptoManager::DeriveKeyFromPassword(m_cachedMasterPassword, m_fileSalt, editKey))
                {
                    MessageBoxW(hWnd, L"Ошибка генерации ключа для редактирования.", L"Ошибка", MB_OK | MB_ICONERROR);
                    CryptoManager::SecureClear(editKey);
                    break;
                }

                // 3. Расшифровываем login и password
                const auto& originalEntry = m_db.GetEntries()[index];
                std::string decryptedLoginUtf8, decryptedPasswordUtf8;
                if (!CryptoManager::DecryptString(originalEntry.encrypted_login, editKey, decryptedLoginUtf8) ||
                    !CryptoManager::DecryptString(originalEntry.encrypted_password, editKey, decryptedPasswordUtf8))
                {
                    MessageBoxW(hWnd, L"Ошибка расшифровки данных для редактирования. Неверный PIN или повреждены данные.", L"Ошибка", MB_OK | MB_ICONERROR);
                    CryptoManager::SecureClear(editKey);
                    break;
                }
                CryptoManager::SecureClear(editKey); // Ключ больше не нужен

                // 4. Конвертируем в wstring
                std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                std::wstring decryptedLogin = converter.from_bytes(decryptedLoginUtf8);
                std::wstring decryptedPassword = converter.from_bytes(decryptedPasswordUtf8);

                // 5. Создаем ВРЕМЕННУЮ структуру с открытыми данными для диалога
                PasswordTemplate tempEntryForDialog; // <-- Временная структура
                tempEntryForDialog.url = originalEntry.url;
                tempEntryForDialog.login = decryptedLogin; // <-- Открытый логин
                tempEntryForDialog.password = decryptedPassword; // <-- Открытый пароль
                tempEntryForDialog.description = originalEntry.description;
                // Поля encrypted_* игнорируются EditEntryDialog'ом

                // 6. Показываем диалог с ВРЕМЕННОЙ структурой
                if (EditEntryDialog::Show(m_hwnd, tempEntryForDialog, true)) // true = редактирование
                {
                    // 7. Пользователь нажал OK, tempEntryForDialog содержит НОВЫЕ открытые данные
                    // Нужно зашифровать их и обновить оригинал в m_db

                    std::vector<unsigned char> updateKey;
                    if (!CryptoManager::DeriveKeyFromPassword(m_cachedMasterPassword, m_fileSalt, updateKey))
                    {
                        MessageBoxW(hWnd, L"Ошибка генерации ключа для обновления записи.", L"Ошибка", MB_OK | MB_ICONERROR);
                        CryptoManager::SecureClear(updateKey);
                        // Очищаем временные данные
                        SecureZeroMemory(const_cast<wchar_t*>(decryptedLogin.c_str()), decryptedLogin.length() * sizeof(wchar_t));
                        SecureZeroMemory(const_cast<wchar_t*>(decryptedPassword.c_str()), decryptedPassword.length() * sizeof(wchar_t));
                        SecureZeroMemory(const_cast<wchar_t*>(tempEntryForDialog.login.c_str()), tempEntryForDialog.login.length() * sizeof(wchar_t));
                        SecureZeroMemory(const_cast<wchar_t*>(tempEntryForDialog.password.c_str()), tempEntryForDialog.password.length() * sizeof(wchar_t));
                        break;
                    }

                    // 8. Конвертируем новые открытые данные в UTF-8
                    std::string newLoginUtf8 = converter.to_bytes(tempEntryForDialog.login);
                    std::string newPasswordUtf8 = converter.to_bytes(tempEntryForDialog.password);

                    // 9. Шифруем
                    std::vector<unsigned char> newEncryptedLogin, newEncryptedPassword;
                    if (!CryptoManager::EncryptString(newLoginUtf8, updateKey, newEncryptedLogin) ||
                        !CryptoManager::EncryptString(newPasswordUtf8, updateKey, newEncryptedPassword))
                    {
                        MessageBoxW(hWnd, L"Ошибка шифрования обновленных данных.", L"Ошибка", MB_OK | MB_ICONERROR);
                        CryptoManager::SecureClear(updateKey);
                        // Очищаем временные данные
                        SecureZeroMemory(const_cast<wchar_t*>(decryptedLogin.c_str()), decryptedLogin.length() * sizeof(wchar_t));
                        SecureZeroMemory(const_cast<wchar_t*>(decryptedPassword.c_str()), decryptedPassword.length() * sizeof(wchar_t));
                        SecureZeroMemory(const_cast<wchar_t*>(tempEntryForDialog.login.c_str()), tempEntryForDialog.login.length() * sizeof(wchar_t));
                        SecureZeroMemory(const_cast<wchar_t*>(tempEntryForDialog.password.c_str()), tempEntryForDialog.password.length() * sizeof(wchar_t));
                        break;
                    }

                    // 10. Обновляем оригинал в m_db
                    auto& entryToUpdate = m_db.GetEntries()[index];
                    entryToUpdate.url = tempEntryForDialog.url;
                    entryToUpdate.description = tempEntryForDialog.description;
                    entryToUpdate.encrypted_login = newEncryptedLogin;
                    entryToUpdate.encrypted_password = newEncryptedPassword;

                    RefreshPasswordList();

                    // 11. Сохраняем изменения
                    if (!m_filePath.empty())
                    {
                        if (!m_db.SaveEncrypted(m_filePath, m_cachedMasterPassword))
                        {
                            MessageBoxW(hWnd, L"Ошибка сохранения базы после редактирования записи.", L"Ошибка", MB_OK | MB_ICONERROR);
                        }
                        else
                        {
                            // Обновляем соль
                            m_fileSalt = m_db.GetLastUsedSalt();
                        }
                    }
                    CryptoManager::SecureClear(updateKey);
                    // Очищаем временные данные
                    SecureZeroMemory(const_cast<wchar_t*>(decryptedLogin.c_str()), decryptedLogin.length() * sizeof(wchar_t));
                    SecureZeroMemory(const_cast<wchar_t*>(decryptedPassword.c_str()), decryptedPassword.length() * sizeof(wchar_t));
                    SecureZeroMemory(const_cast<wchar_t*>(tempEntryForDialog.login.c_str()), tempEntryForDialog.login.length() * sizeof(wchar_t));
                    SecureZeroMemory(const_cast<wchar_t*>(tempEntryForDialog.password.c_str()), tempEntryForDialog.password.length() * sizeof(wchar_t));
                }
                else
                {
                    // Пользователь отменил, очищаем временные данные
                    SecureZeroMemory(const_cast<wchar_t*>(decryptedLogin.c_str()), decryptedLogin.length() * sizeof(wchar_t));
                    SecureZeroMemory(const_cast<wchar_t*>(decryptedPassword.c_str()), decryptedPassword.length() * sizeof(wchar_t));
                }
            }
            else
            {
                // --- ДОБАВЛЕНИЕ ---
                PasswordTemplate newEntry = {};
                if (EditEntryDialog::Show(m_hwnd, newEntry, false)) // false = добавление
                {
                    // newEntry содержит открытые login/password
                    // Нужно зашифровать их и добавить в m_db
                    if (m_cachedMasterPassword.empty() || m_fileSalt.empty())
                    {
                        MessageBoxW(hWnd, L"Невозможно добавить запись: мастер-пароль или соль недоступны.", L"Ошибка", MB_OK | MB_ICONERROR);
                        break;
                    }

                    std::vector<unsigned char> addKey;
                    if (!CryptoManager::DeriveKeyFromPassword(m_cachedMasterPassword, m_fileSalt, addKey))
                    {
                        MessageBoxW(hWnd, L"Ошибка генерации ключа для новой записи.", L"Ошибка", MB_OK | MB_ICONERROR);
                        CryptoManager::SecureClear(addKey);
                        break;
                    }

                    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                    std::string newLoginUtf8 = converter.to_bytes(newEntry.login);
                    std::string newPasswordUtf8 = converter.to_bytes(newEntry.password);

                    std::vector<unsigned char> newEncryptedLogin, newEncryptedPassword;
                    if (!CryptoManager::EncryptString(newLoginUtf8, addKey, newEncryptedLogin) ||
                        !CryptoManager::EncryptString(newPasswordUtf8, addKey, newEncryptedPassword))
                    {
                        MessageBoxW(hWnd, L"Ошибка шифрования новых данных.", L"Ошибка", MB_OK | MB_ICONERROR);
                        CryptoManager::SecureClear(addKey);
                        break;
                    }

                    PasswordTemplate newEncryptedEntry;
                    newEncryptedEntry.url = newEntry.url;
                    newEncryptedEntry.description = newEntry.description;
                    newEncryptedEntry.encrypted_login = newEncryptedLogin;
                    newEncryptedEntry.encrypted_password = newEncryptedPassword;

                    m_db.GetEntries().push_back(newEncryptedEntry); // Добавляем в вектор

                    RefreshPasswordList();
                    if (!m_filePath.empty())
                    {
                        if (!m_db.SaveEncrypted(m_filePath, m_cachedMasterPassword))
                        {
                            MessageBoxW(hWnd, L"Не удалось сохранить базу после добавления записи.", L"Ошибка", MB_OK | MB_ICONERROR);
                        }
                        else
                        {
                            // Обновляем соль
                            m_fileSalt = m_db.GetLastUsedSalt();
                        }
                    }
                    CryptoManager::SecureClear(addKey);
                }
            }
            // --- КОНЕЦ РЕДАКТИРОВАНИЯ/ДОБАВЛЕНИЯ ---
        }
        break;
    }

    // ... (все остальные case) ...
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}