// www-client.cpp - МОДИФИЦИРОВАННАЯ ВЕРСИЯ
#define _CRT_SECURE_NO_WARNINGS
#include <string>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>
#pragma comment (lib,"Ws2_32.lib")
#pragma warning(disable: 4996)

using namespace std;

int main() {
    setlocale(LC_ALL, "ru");
    WSADATA ws;
    SOCKET s;
    sockaddr_in adr;
    HOSTENT* hn;
    char buff[65535];

    // init
    if (WSAStartup(0x0202, &ws) != 0) { return -1; }

    // создаём сокет
    if (INVALID_SOCKET == (s = socket(AF_INET, SOCK_STREAM, 0))) {
        return -1;
    }

    // получаем адрес
    if (NULL == (hn = gethostbyname("localhost"))) {
        return -1;
    }

    // заполняем структуру адреса
    adr.sin_family = AF_INET;
    ((unsigned long*)&adr.sin_addr)[0] = ((unsigned long**)hn->h_addr_list)[0][0];
    adr.sin_port = htons(8000);

    // устанавливаем соединение
    if (SOCKET_ERROR == connect(s, (sockaddr*)&adr, sizeof(adr))) {
        return -1;
    }

    // ===== НОВЫЙ КОД: СПРАШИВАЕМ, ЧТО ОТПРАВИТЬ =====
    string userData;
    cout << "Введите данные для сохранения на сервере: ";
    getline(cin, userData);

    // Формируем POST-запрос с данными
    string request = "POST / HTTP/1.1\r\n";
    request += "Host: localhost\r\n";
    request += "Content-Length: " + to_string(userData.length()) + "\r\n";
    request += "\r\n";
    request += userData;
    // ===== КОНЕЦ НОВОГО КОДА =====

    // посылаем запрос серверу
    if (SOCKET_ERROR == send(s, request.c_str(), request.length(), 0)) {
        return -1;
    }

    // ждём ответа
    int len = 0;
    do {
        if (SOCKET_ERROR == (len = recv(s, (char*)&buff, 65535, 0))) {
            return -1;
        }
        for (int i = 0; i < len; i++) cout << buff[i];
    } while (len != 0);

    // закрываем соединение
    if (SOCKET_ERROR == closesocket(s)) {
        return -1;
    }

    cin.get();
    return 1;
}
