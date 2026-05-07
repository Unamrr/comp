// Отключаем предупреждения об устаревших функциях
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Подключаем заголовки для сокетов, потоков, ввода/вывода, строк
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>
using namespace std;

// Подключаем библиотеку с сокетами
#pragma comment(lib, "Ws2_32.lib")

// ==================== КОНСТАНТЫ ====================

// Порт сервера (должен совпадать с серверным)
#define PORT 666

// IP-адрес сервера (127.0.0.1 - localhost, для теста)
// При работе по сети заменить на реальный IP сервера
#define SERVERADDR "127.0.0.1"

// ==================== ФУНКЦИЯ ПОТОКА ДЛЯ ПРИЁМА СООБЩЕНИЙ ====================
// Эта функция работает в отдельном потоке и постоянно слушает сервер

DWORD WINAPI ReceiveMessages(LPVOID sock) {
    // Получаем сокет из параметра
    SOCKET s = *(SOCKET*)sock;
    
    // Буфер для приёма
    char buff[1024];
    int len;
    
    // Бесконечный цикл приёма сообщений
    while ((len = recv(s, buff, 1024, 0)) > 0) {
        // Добавляем завершающий ноль
        buff[len] = '\0';
        // Выводим полученное сообщение на экран (прямо в чат)
        cout << buff;
    }
    
    return 0;
}

// ==================== ГЛАВНАЯ ФУНКЦИЯ ====================

int main() {
    WSADATA wsaData;
    SOCKET my_sock;
    
    // Приветствие
    cout << "CHAT CLIENT\n";
    cout << "Connecting to server: " << SERVERADDR << ":" << PORT << endl;
    
    // 1. Инициализируем WinSock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        cout << "WSAStartup error\n";
        return -1;
    }
    
    // 2. Создаём сокет
    my_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (my_sock == INVALID_SOCKET) {
        cout << "Socket error\n";
        WSACleanup();
        return -1;
    }
    
    // 3. Настраиваем адрес сервера
    sockaddr_in dest_addr{};
    dest_addr.sin_family = AF_INET;      // IPv4
    dest_addr.sin_port = htons(PORT);    // порт 666
    dest_addr.sin_addr.s_addr = inet_addr(SERVERADDR); // IP сервера
    
    // 4. Подключаемся к серверу
    // connect() - устанавливает соединение с сервером
    if (connect(my_sock, (sockaddr*)&dest_addr, sizeof(dest_addr))) {
        cout << "Connection error to " << SERVERADDR << endl;
        system("pause");  // Ждём нажатия клавиши, чтобы пользователь увидел ошибку
        return -1;
    }
    
    // Подключение успешно
    cout << "Connected to server!\n";
    
    // 5. Запрашиваем ник пользователя
    cout << "Enter your nickname: ";
    string nick;
    getline(cin, nick);  // читаем строку с клавиатуры
    
    // 6. Отправляем ник серверу (сервер добавит нас в таблицу)
    send(my_sock, nick.c_str(), nick.size(), 0);
    
    // 7. Запускаем отдельный поток для приёма сообщений от сервера
    // Пока мы что-то печатаем, этот поток будет в фоне получать и выводить сообщения
    DWORD thID;
    CreateThread(NULL, 0, ReceiveMessages, &my_sock, 0, &thID);
    
    // 8. Выводим подсказки по командам чата
    cout << "\n===== CHAT =====" << endl;
    cout << "  Public message: just type text" << endl;
    cout << "  Private message: @nick message" << endl;
    cout << "  Exit: quit\n" << endl;
    
    // 9. Основной цикл - читаем ввод пользователя и отправляем на сервер
    string msg;
    while (true) {
        // Читаем строку из консоли (то, что пользователь печатает)
        getline(cin, msg);
        
        // Если пользователь ввёл "quit" - выходим из чата
        if (msg == "quit") break;
        
        // Отправляем сообщение серверу
        // Сервер сам определит, публичное оно или приватное (по наличию @)
        send(my_sock, msg.c_str(), msg.size(), 0);
    }
    
    // 10. Завершение работы
    closesocket(my_sock);   // закрываем сокет
    WSACleanup();           // чистим WinSock
    return 0;
}
