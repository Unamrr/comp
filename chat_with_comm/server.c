// Отключаем предупреждения об устаревших функциях (inet_addr и т.д.)
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Подключаем заголовочные файлы для работы с сетью (сокеты в Windows)
#include <winsock2.h>
#include <ws2tcpip.h>

// Для работы с потоками (CreateThread) и критическими секциями
#include <windows.h>

// Для ввода/вывода на экран (cout, cin)
#include <iostream>

// Для работы с типом string
#include <string>

// Для хранения пары "ник" → "сокет" (таблица клиентов)
#include <map>

// Чтобы не писать каждый раз std::cout, std::string и т.д.
using namespace std;

// Подключаем библиотеку с сокетами (Ws2_32.lib)
#pragma comment(lib, "Ws2_32.lib")

// ==================== КОНСТАНТЫ ====================

// Порт, на котором сервер будет слушать подключения
#define PORT 666

// IP-адрес сервера (127.0.0.1 - localhost, для теста на одном компьютере)
// При работе по сети заменить на свой реальный IP
#define SERVERADDR "127.0.0.1"

// ==================== ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ====================

// Счётчик подключённых клиентов (нужен для макроса PRINTNUSERS)
int nclients = 0;

// Таблица (словарь): ник клиента → его сокет (канал связи)
// map - это как список, где можно быстро найти значение по ключу
map<string, SOCKET> clients;

// Критическая секция - защищает общие данные от одновременного изменения
// Несколько потоков не могут войти в неё одновременно
CRITICAL_SECTION cs;

// Макрос для вывода количества пользователей
// \ позволяет написать макрос в несколько строк
#define PRINTNUSERS \
if (nclients) \
    cout << "Users online: " << nclients << endl; \
else \
    cout << "No users online\n";

// ==================== ПРОТОТИП ФУНКЦИИ ====================

// Функция, которая будет выполняться в отдельном потоке для каждого клиента
// DWORD WINAPI - стандартный тип для потоковой функции в Windows
// LPVOID - указатель на любые данные (в данном случае на сокет клиента)
DWORD WINAPI ConToClient(LPVOID client_socket);

// ==================== ГЛАВНАЯ ФУНКЦИЯ (ТОЧКА ВХОДА) ====================

int main() {
    // WSADATA - структура, хранящая информацию о версии WinSock
    WSADATA wsaData;
    
    // mysocket - сокет сервера (через него принимаем подключения)
    SOCKET mysocket;

    // Выводим приветствие и IP сервера в консоль
    cout << "CHAT SERVER\n";
    cout << "Server IP: " << SERVERADDR << endl;

    // ===== ШАГ 1: ИНИЦИАЛИЗАЦИЯ WIN SOCK =====
    // WSAStartup - подготавливает библиотеку для работы с сетью
    // MAKEWORD(2,2) - запрашиваем версию 2.2
    // Если возвращает не 0 - ошибка
    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        cout << "WSAStartup error\n";
        return -1;  // Выход с ошибкой
    }

    // Создаём критическую секцию (готовим её к использованию)
    InitializeCriticalSection(&cs);

    // ===== ШАГ 2: СОЗДАНИЕ СОКЕТА =====
    // socket() - создаёт сокет
    // AF_INET - используем IPv4
    // SOCK_STREAM - используем TCP (надёжная передача с установкой соединения)
    // 0 - автоматический выбор протокола (TCP)
    mysocket = socket(AF_INET, SOCK_STREAM, 0);
    
    // Если сокет не создан (INVALID_SOCKET) - ошибка
    if (mysocket == INVALID_SOCKET) {
        cout << "Socket error\n";
        WSACleanup();             // закрываем WinSock
        DeleteCriticalSection(&cs); // удаляем критическую секцию
        return -1;
    }

    // ===== ШАГ 3: НАСТРОЙКА АДРЕСА СЕРВЕРА =====
    // Структура для хранения адреса сервера (IPv4)
    sockaddr_in local_addr{};
    
    local_addr.sin_family = AF_INET;           // IPv4
    local_addr.sin_port = htons(PORT);         // порт (htons - переводит в нужный формат)
    local_addr.sin_addr.s_addr = inet_addr(SERVERADDR); // IP-адрес

    // ===== ШАГ 4: ПРИВЯЗКА СОКЕТА К АДРЕСУ =====
    // bind() - привязывает сокет к конкретному IP и порту
    // (сокет) https://example.com
    if (bind(mysocket, (sockaddr*)&local_addr, sizeof(local_addr))) {
        cout << "Bind error on IP " << SERVERADDR << endl;
        closesocket(mysocket);    // закрываем сокет
        WSACleanup();             // закрываем WinSock
        DeleteCriticalSection(&cs);
        return -1;
    }

    // ===== ШАГ 5: НАЧАЛО ПРОСЛУШИВАНИЯ =====
    // listen() - начинает слушать входящие подключения
    // SOMAXCONN - максимальный размер очереди (максимально возможный)
    if (listen(mysocket, SOMAXCONN)) {
        cout << "Listen error\n";
        closesocket(mysocket);
        WSACleanup();
        DeleteCriticalSection(&cs);
        return -1;
    }

    // Сообщаем, что сервер запущен и ждёт клиентов
    cout << "Server started on " << SERVERADDR << ":" << PORT << "\n";
    cout << "Waiting connections...\n";

    // ===== ШАГ 6: ОСНОВНОЙ ЦИКЛ - ПРИЁМ КЛИЕНТОВ =====
    // Переменные для хранения информации о подключившемся клиенте
    sockaddr_in client_addr{};      // адрес клиента
    int client_addr_size = sizeof(client_addr); // размер структуры
    SOCKET client_socket;            // сокет для общения с клиентом

    // accept() - ждёт подключения. Когда клиент подключается, возвращает его сокет
    // Цикл работает вечно (пока не закроем программу)
    while ((client_socket = accept(mysocket, (sockaddr*)&client_addr, &client_addr_size)) != INVALID_SOCKET) {
        
        // Выделяем память для копии сокета клиента
        // Нужно, потому что переменная client_socket может измениться в следующей итерации цикла
        SOCKET* pclient = new SOCKET;
        *pclient = client_socket;

        // Создаём новый поток для обслуживания подключившегося клиента
        // Поток будет выполнять функцию ConToClient и получит сокет в параметре
        DWORD thID;
        CreateThread(NULL, 0, ConToClient, pclient, 0, &thID);
    }

    // ===== ЗАВЕРШЕНИЕ (сюда никогда не доходит в обычной работе) =====
    closesocket(mysocket);
    WSACleanup();
    DeleteCriticalSection(&cs);
    return 0;
}

// ==================== ФУНКЦИЯ ПОТОКА ДЛЯ ОДНОГО КЛИЕНТА ====================
// Эта функция выполняется в отдельном потоке для КАЖДОГО подключившегося клиента

DWORD WINAPI ConToClient(LPVOID client_socket) {
    // Извлекаем сокет клиента из параметра
    SOCKET my_sock = *(SOCKET*)client_socket;
    
    // Удаляем временную память, которую выделили в main
    delete (SOCKET*)client_socket;

    // Буфер для приёма данных (1024 байта)
    char buff[1024];
    // Длина принятого сообщения
    int len;

    // ===== 1. ПОЛУЧАЕМ НИК ОТ КЛИЕНТА =====
    // recv() - принимает данные от сокета
    len = recv(my_sock, buff, 1024, 0);
    
    // Если ничего не получили или ошибка - отключаем клиента
    if (len <= 0) {
        closesocket(my_sock);
        return 0;
    }
    
    // Ставим завершающий ноль в конце строки (чтобы работать как со string)
    buff[len] = '\0';
    // Сохраняем ник в переменную типа string
    string nick(buff);

    // ===== 2. ДОБАВЛЯЕМ КЛИЕНТА В ОБЩИЙ СПИСОК =====
    // Входим в критическую секцию - запрещаем другим потокам менять общие данные
    EnterCriticalSection(&cs);
    
    // Добавляем пару "ник → сокет" в таблицу
    clients[nick] = my_sock;
    // Увеличиваем счётчик клиентов
    nclients++;
    
    // Выводим в консоль сервера информацию о подключении
    cout << "+ " << nick << " connected\n";
    // Выводим количество пользователей онлайн (макрос)
    PRINTNUSERS
    
    // Выходим из критической секции - разрешаем другим потокам работать
    LeaveCriticalSection(&cs);

    // ===== 3. ОПОВЕЩАЕМ ВСЕХ ОСТАЛЬНЫХ КЛИЕНТОВ О НОВОМ ПОЛЬЗОВАТЕЛЕ =====
    string welcomeMsg = "=== " + nick + " joined the chat ===\n";
    
    EnterCriticalSection(&cs);
    
    // Проходим по всем клиентам в таблице
    for (auto& p : clients) {
        // Если это НЕ сам новый клиент (отправляем всем, кроме него)
        if (p.second != my_sock) {
            // send() - отправляем сообщение клиенту
            send(p.second, welcomeMsg.c_str(), welcomeMsg.size(), 0);
        }
    }
    
    LeaveCriticalSection(&cs);

    // ===== 4. ОСНОВНОЙ ЦИКЛ ПРИЁМА СООБЩЕНИЙ ОТ ЭТОГО КЛИЕНТА =====
    // Пока клиент присылает данные (len > 0), обрабатываем их
    while ((len = recv(my_sock, buff, 1024, 0)) > 0) {
        // Добавляем завершающий ноль
        buff[len] = '\0';
        // Преобразуем в string для удобной работы
        string msg(buff);

        // ===== ОПРЕДЕЛЯЕМ ТИП СООБЩЕНИЯ =====
        // Если первый символ сообщения - @, то это ПРИВАТНОЕ сообщение
        if (msg[0] == '@') {
            
            // Ищем пробел (разделитель между ником получателя и текстом)
            // Пример: "@Маша Привет" → пробел после "Маша"
            size_t spacePos = msg.find(' ');
            
            // Если пробел найден (не равна npos - нет позиции)
            if (spacePos != string::npos) {
                // Вырезаем ник получателя (от 1 символа до пробела, без @)
                // msg.substr(начало, длина)
                string targetNick = msg.substr(1, spacePos - 1);
                
                // Вырезаем текст сообщения (от пробела+1 до конца)
                string privateMsg = "[PM from " + nick + "]: " + msg.substr(spacePos + 1) + "\n";
                
                // Ищем получателя в таблице
                EnterCriticalSection(&cs);
                
                // auto it = clients.find(targetNick) - ищем по ключу
                // it != clients.end() - если нашли (не конец таблицы)
                auto it = clients.find(targetNick);
                if (it != clients.end()) {
                    // Отправляем сообщение получателю
                    send(it->second, privateMsg.c_str(), privateMsg.size(), 0);
                    
                    // Отправляем отправителю подтверждение (чтобы он знал, что сообщение ушло)
                    string confirm = "[PM to " + targetNick + "]: " + msg.substr(spacePos + 1) + "\n";
                    send(my_sock, confirm.c_str(), confirm.size(), 0);
                } else {
                    // Если пользователь с таким ником не найден - отправляем ошибку
                    string error = "User " + targetNick + " not found\n";
                    send(my_sock, error.c_str(), error.size(), 0);
                }
                
                LeaveCriticalSection(&cs);
            }
        } 
        // Иначе (первый символ не @) - это ПУБЛИЧНОЕ сообщение
        else {
            // Формируем сообщение с ником отправителя
            // Пример: "[Диана]: Всем привет!"
            string publicMsg = "[" + nick + "]: " + msg + "\n";
            
            // Выводим это сообщение в консоль сервера (для лога)
            cout << publicMsg;
            
            // Отправляем сообщение ВСЕМ клиентам
            EnterCriticalSection(&cs);
            
            // Проходим по всей таблице клиентов
            for (auto& p : clients) {
                // отправляем каждому
                send(p.second, publicMsg.c_str(), publicMsg.size(), 0);
            }
            
            LeaveCriticalSection(&cs);
        }
    }

    // ===== 5. КЛИЕНТ ОТКЛЮЧИЛСЯ (вышли из цикла) =====
    
    // Удаляем его из таблицы
    EnterCriticalSection(&cs);
    
    // erase() - удаляет элемент из map по ключу (нику)
    clients.erase(nick);
    // Уменьшаем счётчик
    nclients--;
    
    cout << "- " << nick << " disconnected\n";
    PRINTNUSERS
    
    // Оповещаем всех остальных, что этот клиент покинул чат
    string leaveMsg = "=== " + nick + " left the chat ===\n";
    for (auto& p : clients) {
        send(p.second, leaveMsg.c_str(), leaveMsg.size(), 0);
    }
    
    LeaveCriticalSection(&cs);
    
    // Закрываем сокет клиента
    closesocket(my_sock);
    
    // Завершаем поток (возвращаем 0 - успешно)
    return 0;
}
