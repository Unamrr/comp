#include <windows.h>
#include <winsock2.h>
#include <iostream>
using namespace std;

#pragma comment(lib, "Ws2_32.lib")
#define PORT 666

CRITICAL_SECTION cs;
SOCKET clients[100];
int clientCount = 0;

DWORD WINAPI ClientHandler(LPVOID sock) {
    SOCKET client = *(SOCKET*)sock;
    char buff[1024];
    int len;

    while ((len = recv(client, buff, 1024, 0)) > 0) {
        buff[len] = '\0';
        
        // Рассылка всем
        EnterCriticalSection(&cs);
        for (int i = 0; i < clientCount; i++) {
            if (clients[i] != client) {
                send(clients[i], buff, len, 0);
            }
        }
        LeaveCriticalSection(&cs);
    }

    // Удаление клиента
    EnterCriticalSection(&cs);
    for (int i = 0; i < clientCount; i++) {
        if (clients[i] == client) {
            for (int j = i; j < clientCount - 1; j++)
                clients[j] = clients[j + 1];
            clientCount--;
            break;
        }
    }
    LeaveCriticalSection(&cs);

    closesocket(client);
    return 0;
}

int main() {
    WSADATA wsa;
    WSAStartup(0x202, &wsa);
    InitializeCriticalSection(&cs);

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = 0;

    bind(s, (sockaddr*)&addr, sizeof(addr));
    listen(s, 100);
    cout << "Server on port " << PORT << endl;

    while (true) {
        SOCKET client = accept(s, NULL, NULL);
        EnterCriticalSection(&cs);
        clients[clientCount++] = client;
        LeaveCriticalSection(&cs);

        DWORD tid;
        CreateThread(NULL, 0, ClientHandler, &client, 0, &tid);
        cout << "Client connected. Total: " << clientCount << endl;
    }

    DeleteCriticalSection(&cs);
    closesocket(s);
    WSACleanup();
    return 0;
}#include <windows.h>
#include <winsock2.h>
#include <iostream>
#include <string>
using namespace std;

#pragma comment(lib, "Ws2_32.lib")
#define PORT 666

DWORD WINAPI Receiver(LPVOID sock) {
    SOCKET s = *(SOCKET*)sock;
    char buff[1024];
    int len;
    while ((len = recv(s, buff, 1024, 0)) > 0) {
        buff[len] = '\0';
        cout << buff << endl;
    }
    return 0;
}

int main() {
    cout << "Server IP: ";
    string ip;
    cin >> ip;

    WSADATA wsa;
    WSAStartup(0x202, &wsa);

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (connect(s, (sockaddr*)&addr, sizeof(addr))) {
        cout << "Connection failed\n";
        return 1;
    }

    cout << "Connected!\n";
    cout << "Enter your name: ";
    string name;
    cin.ignore();
    getline(cin, name);
    send(s, name.c_str(), name.size(), 0);

    DWORD tid;
    CreateThread(NULL, 0, Receiver, &s, 0, &tid);

    string msg;
    while (true) {
        getline(cin, msg);
        if (msg == "quit") break;
        string full = "[" + name + "]: " + msg;
        send(s, full.c_str(), full.size(), 0);
    }

    closesocket(s);
    WSACleanup();
    return 0;
}
