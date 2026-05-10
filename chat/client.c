//клиент
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>
using namespace std;

#pragma comment(lib, "Ws2_32.lib")

#define PORT 666
#define SERVERADDR "127.0.0.1"

// Глобальная переменная для управления состоянием ника
bool nicknameAccepted = false;
string currentNick;

// Функция потока для приёма сообщений от сервера
DWORD WINAPI ReceiveMessages(LPVOID sock) {
    SOCKET s = *(SOCKET*)sock;
    char buff[1024];
    int len;

    while ((len = recv(s, buff, 1024, 0)) > 0) {
        buff[len] = '\0';
        string msg(buff);

        if (msg == "NICKNAME_TAKEN\n") {
            cout << "\n[ERROR] Nickname '" << currentNick << "' is already taken!\n";
            cout << "Please enter a different nickname: ";
            nicknameAccepted = false;
        }
        else if (msg == "NICKNAME_ACCEPTED\n") {
            cout << "Nickname '" << currentNick << "' accepted! Welcome to chat!\n";
            nicknameAccepted = true;
        }
        else {
            cout << msg;
        }
    }
    return 0;
}

int main() {
    WSADATA wsaData;
    SOCKET my_sock;

    cout << "CHAT CLIENT\n";
    cout << "Connecting to server: " << SERVERADDR << ":" << PORT << endl;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        cout << "WSAStartup error\n";
        return -1;
    }

    my_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (my_sock == INVALID_SOCKET) {
        cout << "Socket error\n";
        WSACleanup();
        return -1;
    }

    sockaddr_in dest_addr{};
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);
    dest_addr.sin_addr.s_addr = inet_addr(SERVERADDR);

    if (connect(my_sock, (sockaddr*)&dest_addr, sizeof(dest_addr))) {
        cout << "Connection error to " << SERVERADDR << endl;
        system("pause");
        return -1;
    }

    cout << "Connected to server!\n";

    // Запускаем поток для приёма сообщений
    DWORD thID;
    CreateThread(NULL, 0, ReceiveMessages, &my_sock, 0, &thID);

    // Цикл ввода ника (пока не примут)
    nicknameAccepted = false;
    while (!nicknameAccepted) {
        cout << "Enter your nickname: ";
        getline(cin, currentNick);
        send(my_sock, currentNick.c_str(), currentNick.size(), 0);

        // Небольшая задержка, чтобы сервер успел ответить
        Sleep(100);
    }

    cout << "\n===== CHAT =====" << endl;
    cout << "  Public message: just type text" << endl;
    cout << "  Private message: @nick message" << endl;
    cout << "  Exit: quit\n" << endl;

    string msg;
    while (true) {
        getline(cin, msg);
        if (msg == "quit") break;
        send(my_sock, msg.c_str(), msg.size(), 0);
    }

    closesocket(my_sock);
    WSACleanup();
    return 0;
}
