#include "scpi_parser.h"
#include "usbd_cdc_if.h"


extern const TScpiCommand ScpiCommand[];


void SCPI_Send(const char *s)
{
    size_t len = strlen(s);

    // Если строка уже заканчивается \r или \n — шлём как есть
    if (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r')) {
        while (CDC_Transmit_FS((uint8_t*)s, (uint16_t)len) == USBD_BUSY) {
            // ждём
        }
        return;
    }

    // Иначе добавляем \r\n во временный буфер
    char buf[64];
    if (len > sizeof(buf) - 3)        // защита от переполнения
        len = sizeof(buf) - 3;

    memcpy(buf, s, len);
    buf[len++] = '\r';
    buf[len++] = '\n';
    buf[len]   = '\0';                // не обязательно для передачи, но удобно

    while (CDC_Transmit_FS((uint8_t*)buf, (uint16_t)len) == USBD_BUSY) {
        // ждём
    }
}


// Вспомогательное: в верхний регистр
static void str_toupper(char *s)
{
    while (*s) {
        if (*s >= 'a' && *s <= 'z')
            *s -= 'a' - 'A';  // разница между маленькой и большой буквой
        s++;
    }
}

// парсим
void SCPI_ProcessLine(char *line)
{
    // 1. Обрезаем пробелы в начале/конце
    char *p = line;
    while (*p == ' ' || *p == '\t') p++;
    char *start = p;

    char *end = start;
    while (*end) end++;
    while (end > start && (end[-1] == ' ' || end[-1] == '\t')) {
        end--;
    }
    *end = '\0';

    if (*start == '\0') return; // пустая строка

    // 2. Проверяем, запрос это или команда
    bool is_query = false;
    if (end > start && end[-1] == '?') {
        is_query = true;
        end[-1] = '\0'; // убираем '?'
        // ещё раз обрежем хвост на всякий случай
        while (end > start && (end[-1] == ' ' || end[-1] == '\t')) {
            end--;
            *end = '\0';
        }
    }

    // 3. Убираем ведущий двоеточие (:LED -> LED)
    if (*start == ':') start++;

    // 4. Делим на "команда" и "аргументы"
    char *args = start;
    while (*args && *args != ' ' && *args != '\t') {
        args++;
    }

    if (*args) {              // если есть пробел
        *args = '\0';         // отделяем команду
        args++;               // указываем на аргументы
        while (*args == ' ' || *args == '\t') args++; // пропускаем лишние пробелы
    } else {
        args = NULL;          // аргументов нет
    }

    // 5. Нормализуем имя команды (верхний регистр)
    str_toupper(start);

    // 6. Поиск в таблице
    const TScpiCommand *cmd = ScpiCommand;
    while (cmd->name) {
        if (strcmp(cmd->name, start) == 0) {
            cmd->handler(is_query, args);
            return;
        }
        cmd++;
    }

    // 7. Если не нашли
    SCPI_Send("ERR:UNKNOWN_COMMAND");
}
