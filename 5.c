#include <windows.h>
#include <winsock2.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 666
#define BUFFER_SIZE 1024

using namespace std;

// Критическая секция для защиты списка клиентов
CRITICAL_SECTION csClientList;

struct Client {
    SOCKET socket;
    string name;
};

vector<Client> clients;

// Поиск клиента по имени
SOCKET GetSocketByName(const string& name) {
    EnterCriticalSection(&csClientList);
    for (const auto& client : clients) {
        if (client.name == name) {
            LeaveCriticalSection(&csClientList);
            return client.socket;
        }
    }
    LeaveCriticalSection(&csClientList);
    return INVALID_SOCKET;
}

// Отправить сообщение одному клиенту
void SendToClient(SOCKET sock, const string& msg) {
    send(sock, msg.c_str(), msg.size(), 0);
}

// Отправить сообщение всем клиентам
void SendToAll(const string& msg, SOCKET exclude = INVALID_SOCKET) {
    EnterCriticalSection(&csClientList);
    for (const auto& client : clients) {
        if (client.socket != exclude) {
            SendToClient(client.socket, msg);
        }
    }
    LeaveCriticalSection(&csClientList);
}

// Функция потока для одного клиента
DWORD WINAPI ClientHandler(LPVOID param) {
    SOCKET clientSocket = *(SOCKET*)param;
    char buffer[BUFFER_SIZE];
    int bytesReceived;
    string clientName;

    // Получить имя клиента
    bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        clientName = buffer;
    } else {
        closesocket(clientSocket);
        return 0;
    }

    // Добавить клиента в список
    EnterCriticalSection(&csClientList);
    clients.push_back({clientSocket, clientName});
    int clientCount = clients.size();
    LeaveCriticalSection(&csClientList);

    cout << "[+] " << clientName << " connected. Total clients: " << clientCount << endl;
    SendToAll("[Server] " + clientName + " joined the chat.\n", clientSocket);

    // Обработка сообщений
    while ((bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[bytesReceived] = '\0';
        string msg(buffer);

        // Приватное сообщение: pm:username:text
        if (msg.rfind("pm:", 0) == 0) {
            size_t firstColon = msg.find(':');
            size_t secondColon = msg.find(':', firstColon + 1);
            if (secondColon != string::npos) {
                string targetName = msg.substr(firstColon + 1, secondColon - firstColon - 1);
                string privateMsg = msg.substr(secondColon + 1);

                SOCKET targetSocket = GetSocketByName(targetName);
                if (targetSocket != INVALID_SOCKET) {
                    SendToClient(targetSocket, "[PM from " + clientName + "]: " + privateMsg + "\n");
                    SendToClient(clientSocket, "[PM to " + targetName + "]: " + privateMsg + "\n");
                } else {
                    SendToClient(clientSocket, "[Error] User " + targetName + " not found.\n");
                }
            }
        }
        // Публичное сообщение
        else {
            string publicMsg = "[" + clientName + "]: " + msg;
            cout << publicMsg << endl;
            SendToAll(publicMsg + "\n", clientSocket);
        }
    }

    // Удаление клиента при отключении
    EnterCriticalSection(&csClientList);
    clients.erase(remove_if(clients.begin(), clients.end(),
        [clientSocket](const Client& c) { return c.socket == clientSocket; }),
        clients.end());
    int newCount = clients.size();
    LeaveCriticalSection(&csClientList);

    cout << "[-] " << clientName << " disconnected. Total clients: " << newCount << endl;
    SendToAll("[Server] " + clientName + " left the chat.\n");
    closesocket(clientSocket);
    return 0;
}

// Главный поток сервера
int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "WSAStartup failed\n";
        return -1;
    }

    InitializeCriticalSection(&csClientList);

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        cout << "Socket creation failed\n";
        WSACleanup();
        return -1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "Bind failed\n";
        closesocket(listenSocket);
        WSACleanup();
        return -1;
    }

    if (listen(listenSocket, 10) == SOCKET_ERROR) {
        cout << "Listen failed\n";
        closesocket(listenSocket);
        WSACleanup();
        return -1;
    }

    cout << "Chat server started on port " << PORT << "...\n";

    while (true) {
        SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) continue;

        DWORD threadId;
        CreateThread(NULL, 0, ClientHandler, &clientSocket, 0, &threadId);
    }

    DeleteCriticalSection(&csClientList);
    closesocket(listenSocket);
    WSACleanup();
    return 0;
}#include <winsock2.h>
#include <iostream>
#include <string>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 666
#define SERVER_ADDR "127.0.0.1"

using namespace std;

SOCKET sock;

void ReceiveMessages() {
    char buffer[1024];
    int bytes;
    while ((bytes = recv(sock, buffer, 1024, 0)) > 0) {
        buffer[bytes] = '\0';
        cout << buffer << endl;
    }
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "Connection failed\n";
        return -1;
    }

    cout << "Enter your name: ";
    string name;
    getline(cin, name);
    send(sock, name.c_str(), name.size(), 0);

    thread receiver(ReceiveMessages);
    receiver.detach();

    string input;
    while (true) {
        getline(cin, input);
        if (input == "quit") break;
        send(sock, input.c_str(), input.size(), 0);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
