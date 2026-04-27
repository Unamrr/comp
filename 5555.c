#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

u_short MY_PORT = 666;

// Macro corrigée 
#define PRINTNUSERS \
if (nclients) \
    cout << "Users online: " << nclients << endl; \
else \
    cout << "No users online\n";

// Variable globale общие переменные
int nclients = 0;

// Prototype прототип
DWORD WINAPI ConToClient(LPVOID client_socket);

int main() {
    WSADATA wsaData;
    SOCKET mysocket;

    cout << "TCP SERVER DEMO\n";

    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        cout << "WSAStartup error\n";
        return -1;
    }

    mysocket = socket(AF_INET, SOCK_STREAM, 0);
    if (mysocket == INVALID_SOCKET) {
        cout << "Socket error\n";
        WSACleanup();
        return -1;
    }

    sockaddr_in local_addr{};
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(MY_PORT);
    local_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(mysocket, (sockaddr*)&local_addr, sizeof(local_addr))) {
        cout << "Bind error\n";
        closesocket(mysocket);
        WSACleanup();
        return -1;
    }

    if (listen(mysocket, SOMAXCONN)) {
        cout << "Listen error\n";
        closesocket(mysocket);
        WSACleanup();
        return -1;
    }

    cout << "Waiting connections...\n";

    sockaddr_in client_addr{};
    int client_addr_size = sizeof(client_addr);
    SOCKET client_socket;

    while ((client_socket = accept(mysocket, (sockaddr*)&client_addr, &client_addr_size)) != INVALID_SOCKET) {

        nclients++;
        cout << "+ Client connected: " << inet_ntoa(client_addr.sin_addr) << endl;
        PRINTNUSERS

            // IMPORTANT : allocation mémoire pour thread выделение памяти для потока
            SOCKET* pclient = new SOCKET;
        *pclient = client_socket;

        DWORD thID;
        CreateThread(NULL, 0, ConToClient, pclient, 0, &thID);
    }

    closesocket(mysocket);
    WSACleanup();
    return 0;
}


//  Fonction THREAD (fait partie du serveur) поток из под сервера
DWORD WINAPI ConToClient(LPVOID client_socket)
{
    SOCKET my_sock = *(SOCKET*)client_socket;
    delete (SOCKET*)client_socket;

    char buff[1024];
    int len;

    char hello[] = "Hello, Student\r\n";
    send(my_sock, hello, sizeof(hello), 0);

    while ((len = recv(my_sock, buff, 1024, 0)) > 0) {
        if (len < 1024)
            buff[len] = '\0';

        cout << "Received: " << buff << endl;
        send(my_sock, buff, len, 0);
    }

    nclients--;
    cout << "- Client disconnected\n";
    PRINTNUSERS

        closesocket(my_sock);
    return 0;
}




#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

#define PORT 666
#define SERVERADDR "127.0.0.1"

int main() {
    WSADATA wsaData;
    SOCKET my_sock;

    cout << "TCP CLIENT DEMO\n";

    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        cout << "WSAStartup error\n";
        return -1;
    }

    my_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (my_sock == INVALID_SOCKET) {
        cout << "Socket error\n";
        return -1;
    }

    sockaddr_in dest_addr{};
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);
    dest_addr.sin_addr.s_addr = inet_addr(SERVERADDR);

    if (connect(my_sock, (sockaddr*)&dest_addr, sizeof(dest_addr))) {
        cout << "Connection error\n";
        return -1;
    }

    cout << "Connected to server\n";
    cout << "Type 'quit' to exit\n";

    char buff[1024];
    int nsize;

    while ((nsize = recv(my_sock, buff, 1024, 0)) > 0) {

        buff[nsize] = '\0';
        cout << "Server: " << buff << endl;

        cout << "You: ";
        string msg;
        getline(cin, msg);

        send(my_sock, msg.c_str(), msg.size(), 0);

        if (msg == "quit")
            break;
    }

    closesocket(my_sock);
    WSACleanup();
    return 0;
}
