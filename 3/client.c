#include <string>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>
#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable: 4996)

using namespace std;

#define request "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n"
#define max_packet_size 65535

int main() {
    WSADATA ws;
    SOCKET s;
    sockaddr_in adr;
    HOSTENT* hn;
    char buff[max_packet_size];

    // Инициализация Winsock
    if (WSAStartup(0x0202, &ws) != 0) {
        cerr << "WSAStartup failed" << endl;
        return -1;
    }

    // Создаём сокет
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        cerr << "Socket creation failed: " << WSAGetLastError() << endl;
        WSACleanup();
        return -1;
    }

    // Получаем адрес
    hn = gethostbyname("localhost");
    if (hn == NULL) {
        cerr << "gethostbyname failed: " << WSAGetLastError() << endl;
        closesocket(s);
        WSACleanup();
        return -1;
    }

    // Заполняем структуру adr
    adr.sin_family = AF_INET;
    adr.sin_port = htons(8000);
    memcpy(&adr.sin_addr, hn->h_addr_list[0], hn->h_length);

    // Устанавливаем соединение
    if (connect(s, (sockaddr*)&adr, sizeof(adr)) == SOCKET_ERROR) {
        cerr << "Connect failed: " << WSAGetLastError() << endl;
        closesocket(s);
        WSACleanup();
        return -1;
    }

    // Посылаем запрос
    if (send(s, request, strlen(request), 0) == SOCKET_ERROR) {
        cerr << "Send failed: " << WSAGetLastError() << endl;
        closesocket(s);
        WSACleanup();
        return -1;
    }

    // Получаем ответ
    int len = 0;
    do {
        len = recv(s, buff, max_packet_size - 1, 0);
        if (len == SOCKET_ERROR) {
            cerr << "Recv failed: " << WSAGetLastError() << endl;
            closesocket(s);
            WSACleanup();
            return -1;
        }
        if (len > 0) {
            buff[len] = '\0';
            cout << buff;
        }
    } while (len > 0);

    // Закрываем соединение
    closesocket(s);
    WSACleanup();

    cout << "\n\nPress Enter to exit...";
    cin.get();
    return 0;
}
