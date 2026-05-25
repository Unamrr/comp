//http://ВАШ_IP:8080

// ============================================================
// СЕРВЕР С ХРАНИЛИЩЕМ ДАННЫХ
// ПРИНИМАЕТ СООБЩЕНИЯ ОТ КЛИЕНТОВ И ПОКАЗЫВАЕТ ИХ НА САЙТЕ
// ============================================================

// Отключаем предупреждения Visual Studio о небезопасных функциях
#define _CRT_SECURE_NO_WARNINGS

// Подключение библиотек
#include <iostream>      // Для вывода сообщений в консоль (cout, cerr)
#include <fstream>       // Для работы с файлами (чтение и запись)
#include <string>        // Для работы со строками (string)
#include <cstring>       // Для строковых функций (strstr, strchr)
#include <WinSock2.h>    // Главная библиотека для сокетов в Windows
#include <WS2tcpip.h>    // Для функции getaddrinfo и inet_ntop

// Подключаем библиотеку сокетов
#pragma comment(lib, "Ws2_32.lib")

using namespace std;     // Чтобы не писать std:: перед каждой функцией

// ============================================================
// ГЛАВНАЯ ФУНКЦИЯ
// ============================================================
int main() {
    // ----- ШАГ 1: ИНИЦИАЛИЗАЦИЯ СОКЕТОВ -----
    // Это ОБЯЗАТЕЛЬНЫЙ шаг для Windows. Без него сокеты не работают.
    WSADATA wsaData;     // Структура для хранения информации о версии сокетов
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);  // Запускаем сокеты (просим версию 2.2)
    if (result != 0) {   // Если не 0 - ошибка
        cerr << "WSAStartup failed: " << result << endl;
        return 1;        // Завершаем программу с ошибкой
    }

    // ----- ШАГ 2: НАСТРОЙКА АДРЕСА СЕРВЕРА -----
    // Мы хотим, чтобы сервер слушал ВСЕ сетевые интерфейсы (0.0.0.0)
    // Это значит, что подключаться могут:
    // - с этого же компьютера (localhost)
    // - из локальной сети (Wi-Fi, провод)
    // - из интернета (если настроен проброс портов)
    
    struct addrinfo* addr = NULL;     // Указатель на структуру с адресом (заполнится автоматически)
    struct addrinfo hints;            // Структура с нашими требованиями к адресу
    
    // Очищаем структуру hints (заполняем нулями, чтобы не было "мусора")
    ZeroMemory(&hints, sizeof(hints));
    
    hints.ai_family = AF_INET;        // Используем IPv4
    hints.ai_socktype = SOCK_STREAM;  // Используем TCP (потоковый сокет)
    hints.ai_protocol = IPPROTO_TCP;  // Протокол TCP
    hints.ai_flags = AI_PASSIVE;      // Режим сервера (будем принимать подключения)

    // ВАЖНО: "0.0.0.0" означает "слушать на всех сетевых интерфейсах"
    // Порт 8080 - стандартный порт для HTTP (можно использовать 80, но нужны права администратора)
    result = getaddrinfo("0.0.0.0", "8080", &hints, &addr);
    if (result != 0) {                // Если ошибка
        cerr << "getaddrinfo failed: " << result << endl;
        WSACleanup();                 // Выгружаем сокеты
        return 1;
    }

    // ----- ШАГ 3: СОЗДАНИЕ СОКЕТА -----
    // socket() создаёт сокет и возвращает его дескриптор (номер)
    // Сокет - это как телефонная трубка для обмена данными
    int listen_socket = socket(addr->ai_family,     // AF_INET (IPv4)
                                addr->ai_socktype,   // SOCK_STREAM (TCP)
                                addr->ai_protocol);  // IPPROTO_TCP
    if (listen_socket == INVALID_SOCKET) {          // Если ошибка
        cerr << "Error at socket: " << WSAGetLastError() << endl;
        freeaddrinfo(addr);          // Освобождаем память
        WSACleanup();                // Выгружаем сокеты
        return 1;
    }

    // ----- ШАГ 4: ПРИВЯЗКА СОКЕТА К АДРЕСУ И ПОРТУ (BIND) -----
    // bind() связывает сокет с конкретным адресом и портом
    // Теперь все входящие соединения на порт 8080 будут направляться к нашему сокету
    result = bind(listen_socket, addr->ai_addr, (int)addr->ai_addrlen);
    if (result == SOCKET_ERROR) {    // Если ошибка
        cerr << "bind failed: " << WSAGetLastError() << endl;
        freeaddrinfo(addr);
        closesocket(listen_socket);  // Закрываем сокет
        WSACleanup();
        return 1;
    }

    // Освобождаем память адреса (он больше не нужен, так как bind уже выполнен)
    freeaddrinfo(addr);

    // ----- ШАГ 5: НАЧАЛО ПРОСЛУШИВАНИЯ ПОРТА (LISTEN) -----
    // listen() переводит сокет в режим ожидания входящих подключений
    // SOMAXCONN - максимальный размер очереди подключений (обычно 5-10)
    if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "listen failed: " << WSAGetLastError() << endl;
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    // ----- ПОЛУЧАЕМ И ПОКАЗЫВАЕМ IP-АДРЕСА ДЛЯ ПОДКЛЮЧЕНИЯ -----
    // Это нужно, чтобы пользователи знали, по какому адресу подключаться
    
    // Получаем имя компьютера в сети
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    
    // Выводим информацию о запуске
    cout << "========================================" << endl;
    cout << "SERVER STARTED" << endl;
    cout << "Port: 8080" << endl;
    cout << "Storage file: storage.txt" << endl;
    cout << "========================================" << endl;
    
    // Адрес для доступа с ЭТОГО ЖЕ компьютера
    cout << "Access from THIS computer: http://localhost:8080" << endl;
    
    // Получаем и показываем локальный IP в сети (для Wi-Fi/Ethernet)
    // Этот IP нужно дать друзьям в той же сети
    struct addrinfo* local_addr = NULL;
    struct addrinfo local_hints;
    ZeroMemory(&local_hints, sizeof(local_hints));
    local_hints.ai_family = AF_INET;
    local_hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo(hostname, NULL, &local_hints, &local_addr) == 0) {
        struct sockaddr_in* ipv4 = (struct sockaddr_in*)local_addr->ai_addr;
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(ipv4->sin_addr), ip_str, INET_ADDRSTRLEN);
        cout << "Access from LOCAL NETWORK (give this to friends): http://" << ip_str << ":8080" << endl;
        freeaddrinfo(local_addr);
    }
    
    cout << "========================================" << endl;
    cout << "Waiting for connections..." << endl;

    // ----- ШАГ 6: ПОДГОТОВКА БУФЕРА ДЛЯ ПРИЁМА ДАННЫХ -----
    const int buffer_size = 1024;    // Размер буфера (1024 байта хватит для HTTP запроса)
    char buffer[buffer_size];        // Буфер для хранения данных от клиента

    // ----- ШАГ 7: ОСНОВНОЙ ЦИКЛ СЕРВЕРА (РАБОТАЕТ ВЕЧНО) -----
    // Сервер будет работать, пока его не закроют (Ctrl+C)
    while (true) {
        
        // ----- ПРИНИМАЕМ ПОДКЛЮЧЕНИЕ ОТ КЛИЕНТА (ACCEPT) -----
        // accept() БЛОКИРУЕТСЯ (ждёт), пока какой-нибудь клиент не подключится
        // Когда клиент подключается, accept() возвращает НОВЫЙ сокет для общения с ним
        SOCKET client_socket = accept(listen_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            cerr << "accept failed: " << WSAGetLastError() << endl;
            continue;  // Продолжаем ждать следующих подключений
        }

        // ----- ПОЛУЧАЕМ IP-АДРЕС ПОДКЛЮЧИВШЕГОСЯ КЛИЕНТА -----
        // Это полезно для логирования - видно, кто отправил сообщение
        struct sockaddr_in client_addr;
        int addr_len = sizeof(client_addr);
        getpeername(client_socket, (struct sockaddr*)&client_addr, &addr_len);
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        cout << "[CONNECTED] Client from " << client_ip << endl;

        // ----- ПОЛУЧАЕМ ЗАПРОС ОТ КЛИЕНТА (RECV) -----
        // recv() читает данные из сокета в буфер
        // Возвращает количество полученных байт
        int bytes_received = recv(client_socket, buffer, buffer_size - 1, 0);
        
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';  // Добавляем признак конца строки

            // ----- ОБРАБОТКА POST ЗАПРОСА (СОХРАНЕНИЕ ДАННЫХ) -----
            // strstr() ищет подстроку в строке. Если находит "POST" - значит клиент хочет сохранить данные
            if (strstr(buffer, "POST") != NULL) {
                
                // Ищем тело запроса (оно идёт после пустой строки \r\n\r\n)
                // В HTTP протоколе заголовки отделяются от тела двумя переносами строк
                char* body = strstr(buffer, "\r\n\r\n");
                if (body != NULL) {
                    body += 4;  // Пропускаем символы \r\n\r\n (4 символа)
                    
                    // Ищем "data=" (это имя поля из HTML-формы) и пропускаем 5 символов
                    // "data=" имеет длину 5 символов
                    if (strstr(body, "data=") != NULL) {
                        body = strstr(body, "data=") + 5;
                    }
                    
                    // Удаляем символы перевода строки в конце (оставляем только чистый текст)
                    char* end = body;
                    while (*end != '\0' && *end != '\r' && *end != '\n') {
                        end++;
                    }
                    *end = '\0';  // Ставим конец строки здесь
                    
                    // ========== РАБОТА С ХРАНИЛИЩЕМ (ФАЙЛ) ==========
                    // fopen() открывает файл для дозаписи ("a" = append, добавление в конец)
                    FILE* storage = fopen("storage.txt", "a");
                    if (storage != NULL) {
                        // Записываем в файл: [IP отправителя] сообщение
                        // Это позволяет видеть, кто что написал
                        fprintf(storage, "[%s] %s\n", client_ip, body);
                        // fclose() закрывает файл (ОЧЕНЬ ВАЖНО! Иначе данные могут не сохраниться)
                        fclose(storage);
                        cout << "[SAVED] From " << client_ip << ": " << body << endl;
                    }
                }
            }

            // ----- ЧТЕНИЕ ВСЕХ СОХРАНЁННЫХ ДАННЫХ ИЗ ФАЙЛА -----
            // Это нужно, чтобы показать на сайте все предыдущие сообщения
            string savedItems = "";  // Строка для хранения HTML-списка
            
            // fopen() открывает файл для чтения ("r" = read)
            FILE* readFile = fopen("storage.txt", "r");
            if (readFile != NULL) {
                char line[1024];                     // Буфер для чтения одной строки
                // fgets() читает одну строку из файла
                while (fgets(line, sizeof(line), readFile)) {
                    // Удаляем символ перевода строки \n в конце
                    char* newline = strchr(line, '\n');
                    if (newline != NULL) {
                        *newline = '\0';
                    }
                    // Добавляем строку в HTML-список (теги <li> и </li>)
                    savedItems += "<li>" + string(line) + "</li>";
                }
                fclose(readFile);  // Закрываем файл
            }
            
            // Если файл пустой или не существует, показываем сообщение
            if (savedItems.empty()) {
                savedItems = "<li>No messages yet. Be the first to send!</li>";
            }

            // ----- ФОРМИРОВАНИЕ HTML СТРАНИЦЫ (ОТВЕТ КЛИЕНТУ) -----
            // Это ультрапростая страница: заголовок и список сообщений
            // Браузер отобразит её, когда пользователь перейдёт на http://ваш_IP:8080
            string html = "";
            html += "<html>\n";
            html += "<head>\n";
            html += "<meta charset='UTF-8'>\n";    // Чтобы русские буквы отображались правильно
            html += "<title>Message Board</title>\n";
            html += "<style>\n";                   // Немного стилей для красоты
            html += "body { font-family: Arial; margin: 40px; }\n";
            html += "li { margin: 10px 0; }\n";
            html += "</style>\n";
            html += "</head>\n";
            html += "<body>\n";
            html += "<h1>Messages from everyone</h1>\n";
            html += "<ul>\n";
            html += savedItems;      // Вставляем список сохранённых сообщений
            html += "</ul>\n";
            html += "</body>\n";
            html += "</html>\n";

            // ----- ФОРМИРОВАНИЕ HTTP ОТВЕТА -----
            // HTTP ответ состоит из заголовков и тела (наша HTML страница)
            string response = "";
            response += "HTTP/1.1 200 OK\r\n";           // Статус "Всё хорошо"
            response += "Content-Type: text/html; charset=utf-8\r\n";   // Тип ответа - HTML
            response += "Content-Length: " + to_string(html.length()) + "\r\n";  // Длина страницы
            response += "Connection: close\r\n";         // Закрыть соединение после ответа
            response += "\r\n";                          // Пустая строка - разделитель заголовков и тела
            response += html;                            // Сама HTML страница

            // ----- ОТПРАВКА ОТВЕТА КЛИЕНТУ (SEND) -----
            // send() отправляет данные клиенту по сокету
            send(client_socket, response.c_str(), response.length(), 0);
            
            cout << "[RESPONSE] Sent to " << client_ip << endl;
        }
        
        // ----- ЗАКРЫТИЕ СОЕДИНЕНИЯ С КЛИЕНТОМ -----
        // После отправки ответа закрываем соединение с этим клиентом
        closesocket(client_socket);
        cout << "[DISCONNECTED] Client from " << client_ip << endl;
    }

    // ----- ЗАВЕРШЕНИЕ РАБОТЫ (СЮДА НЕ ДОХОДИТ ИЗ-ЗА БЕСКОНЕЧНОГО ЦИКЛА) -----
    // Если сервер остановят, закроем всё
    closesocket(listen_socket);
    WSACleanup();
    return 0;
}
