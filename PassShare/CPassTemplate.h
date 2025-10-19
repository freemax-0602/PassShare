#pragma once
#include <string>
#include <vector> // <-- Добавлено для std::vector<unsigned char>

// Структура для хранения одной записи пароля
struct PasswordTemplate
{
    // std::wstring icon; // Удалено, как в вашем коде
    std::wstring url;                                    // Открытое поле (для поиска и отображения)
    std::vector<unsigned char> encrypted_login;          // Зашифрованное поле (бинарные данные)
    std::vector<unsigned char> encrypted_password;       // Зашифрованное поле (бинарные данные)
    std::wstring description;                            // Открытое поле
    // ВАЖНО: EditEntryDialog будет работать с временными std::wstring login/password
    // Эти поля НЕ входят в сериализацию JSON.
    // Для совместимости с EditEntryDialog, можно добавить их как несериализуемые поля,
    // но лучше изменить логику в MainWindow.
    // Пока оставим структуру как есть, MainWindow будет управлять этим.
};