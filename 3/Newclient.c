// ============================================================
// КЛИЕНТ ДЛЯ ОТПРАВКИ СООБЩЕНИЙ НА СЕРВЕР
// ПОДКЛЮЧАЕТСЯ К СЕРВЕРУ И ОТПРАВЛЯЕТ СООБЩЕНИЯ
// ============================================================

// Отключаем предупреждения Visual Studio о небезопасных функциях
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Подключение библиотек
#include <iostream>      // Для ввода/вывода (cout, cin, getline)
#include <string>        // Для работы со строками (string)
#include <winsock2.h>    // Главная библиотека для сокетов в Windows

// Подключаем библиотеку сокетов
#pragma comment(lib, "Ws2_32.lib")
// Отключаем предупреждение 4996 (устаревшие функции)
#pragma warning(disable: 4996)

using namespace std;     // Чтобы не писать std:: перед каждой функцией

// ============================================================
// ГЛАВНАЯ ФУНКЦИЯ
// ============================================================
int main() {
    // ----- ШАГ 1: ИНИЦИАЛИЗАЦИЯ СОКЕТОВ -----
    // Это ОБЯЗАТЕЛЬНЫЙ шаг для Windows. Без него сокеты не работают.
    WSADATA ws;          // Структура для хранения информации о версии сокетов
    // WSAStartup() запускает библиотеку сокетов. MAKEWORD(2,2) - просим версию 2.2
    if (WSAStartup(MAKEWORD(2, 2), &ws) != 0) {
        cout << "WSAStartup failed" << endl;
        return -1;       // Завершаем программу с ошибкой
    }

    // ============================================================
    // ВАЖНО: ЗДЕСЬ НУЖНО ВСТАВИТЬ IP-АДРЕС КОМПЬЮТЕРА С СЕРВЕРОМ
    // ============================================================
    // Спросите у подруги, у которой запущен сервер, её IP-адрес
    // Он будет выглядеть как 192.168.x.x или 10.0.x.x
    // Узнать его можно командой ipconfig в консоли
    string server_ip = "192.168.1.100";   // ВСТАВЬТЕ IP-АДРЕС СЕРВЕРА СЮДА!
    
    // Выводим информацию о подключении
    cout << "========================================" << endl;
    cout << "MESSENGER CLIENT" << endl;
    cout << "========================================" << endl;
    cout << "Connecting to " << server_ip << ":8080..." << endl;

    // ----- ШАГ 2: СОЗДАНИЕ СОКЕТА -----
    // socket() создаёт сокет для общения с сервером
    // AF_INET - IPv4, SOCK_STREAM - TCP, 0 - протокол по умолчанию
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        cout << "Socket creation failed" << endl;
        WSACleanup();    // Выгружаем сокеты
        return -1;
    }

    // ----- ШАГ 3: НАСТРОЙКА АДРЕСА СЕРВЕРА -----
    // Создаём структуру с адресом сервера, к которому будем подключаться
    sockaddr_in server_addr;                    // Структура для хранения адреса
    server_addr.sin_family = AF_INET;           // Семейство адресов (IPv4)
    // inet_addr() преобразует строку с IP-адресом в числовой формат
    server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
    server_addr.sin_port = htons(8080);         // Порт сервера (8080)
    // htons() переводит число в нужный порядок байт (big-endian)

    // ----- ШАГ 4: ПОДКЛЮЧЕНИЕ К СЕРВЕРУ (CONNECT) -----
    // connect() устанавливает соединение с сервером
    // Если сервер не запущен или IP неверный - будет ошибка
    if (connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        cout << "Connection failed. Is server running?" << endl;
        closesocket(client_socket);  // Закрываем сокет
        WSACleanup();                // Выгружаем сокеты
        return -1;
    }
    
    // Если дошли сюда - подключение успешно!
    cout << "Connected to server!" << endl;
    cout << "Type 'exit' to quit" << endl;
    cout << "========================================" << endl;

    // ----- ШАГ 5: ОСНОВНОЙ ЦИКЛ ОТПРАВКИ СООБЩЕНИЙ -----
    // Клиент работает в бесконечном цикле, пока пользователь не введёт "exit"
    while (true) {
        // Запрашиваем сообщение у пользователя
        string userInput;
        cout << "\nYour message: ";
        getline(cin, userInput);   // getline() читает всю строку с пробелами
        
        // Проверяем, хочет ли пользователь выйти
        if (userInput == "exit") {
            cout << "Goodbye!" << endl;
            break;  // Выходим из цикла
        }
        
        // Если сообщение пустое - не отправляем, просим ввести что-то
        if (userInput.empty()) {
            cout << "[WARNING] Empty message, not sent" << endl;
            continue;  // Пропускаем итерацию цикла, идём к следующему вводу
        }

        // ----- ШАГ 6: ФОРМИРОВАНИЕ HTTP POST ЗАПРОСА -----
        // HTTP запрос должен быть в правильном формате, чтобы сервер его понял
        string request = "";
        request += "POST / HTTP/1.1\r\n";                    // Метод POST, корневой ресурс, версия HTTP
        request += "Host: " + server_ip + "\r\n";            // Хост сервера
        request += "Content-Length: " + to_string(userInput.length() + 5) + "\r\n";  // Длина данных (+5 для "data=")
        request += "Content-Type: application/x-www-form-urlencoded\r\n";  // Тип данных (как в HTML-форме)
        request += "\r\n";                                   // Пустая строка - разделитель между заголовками и телом
        request += "data=" + userInput;                      // Тело запроса (сами данные с префиксом "data=")

        // ----- ШАГ 7: ОТПРАВКА ЗАПРОСА НА СЕРВЕР (SEND) -----
        // send() отправляет данные на сервер
        // Первый аргумент - сокет, второй - данные, третий - длина, четвёртый - флаги (0 = стандарт)
        int bytes_sent = send(client_socket, request.c_str(), request.length(), 0);
        if (bytes_sent == SOCKET_ERROR) {
            cout << "Send failed. Connection may be lost." << endl;
            break;  // Выходим из цикла при ошибке
        }

        // ----- ШАГ 8: ПОЛУЧЕНИЕ ОТВЕТА ОТ СЕРВЕРА (RECV) -----
        // После отправки запроса ждём ответ от сервера
        char buffer[4096] = {0};    // Буфер для ответа (4KB, занулён)
        // recv() получает данные от сервера
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';  // Добавляем признак конца строки
            
            // Проверяем статус ответа
            // strstr() ищет подстроку в строке
            if (strstr(buffer, "200 OK") != NULL) {
                cout << "[OK] Message delivered!" << endl;
            } else {
                cout << "[INFO] Server responded, but status unknown" << endl;
            }
        } else {
            // Если bytes_received == 0, сервер закрыл соединение
            cout << "Server disconnected" << endl;
            break;
        }
    }

    // ----- ШАГ 9: ЗАКРЫТИЕ СОКЕТА И ОЧИСТКА -----
    // closesocket() закрывает сокет (освобождает ресурсы)
    closesocket(client_socket);
    // WSACleanup() выгружает библиотеку сокетов
    WSACleanup();
    
    return 0;  // Успешное завершение программы
}
