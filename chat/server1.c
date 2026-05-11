// сервер
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <map>
#include <algorithm>
#include <cctype>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")

#define PORT 666
#define SERVERADDR "127.0.0.1"

int nclients = 0;

// clé = nickname en minuscules
map<string, SOCKET> clients;

CRITICAL_SECTION cs;

#define PRINTNUSERS \
if (nclients) \
    cout << "Users online: " << nclients << endl; \
else \
    cout << "No users online\n";

// Convertit un texte en minuscules
string ToLower(const string& str) {
    string result = str;

    transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) {
            return tolower(c);
        });

    return result;
}

DWORD WINAPI ConToClient(LPVOID client_socket);

int main() {
    WSADATA wsaData;
    SOCKET mysocket;

    cout << "CHAT SERVER\n";
    cout << "Server IP: " << SERVERADDR << endl;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        cout << "WSAStartup error\n";
        return -1;
    }

    InitializeCriticalSection(&cs);

    mysocket = socket(AF_INET, SOCK_STREAM, 0);

    if (mysocket == INVALID_SOCKET) {
        cout << "Socket error\n";
        WSACleanup();
        DeleteCriticalSection(&cs);
        return -1;
    }

    sockaddr_in local_addr{};

    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(PORT);
    local_addr.sin_addr.s_addr = inet_addr(SERVERADDR);

    if (bind(mysocket, (sockaddr*)&local_addr, sizeof(local_addr))) {
        cout << "Bind error on IP " << SERVERADDR << endl;

        closesocket(mysocket);
        WSACleanup();
        DeleteCriticalSection(&cs);

        return -1;
    }

    if (listen(mysocket, SOMAXCONN)) {
        cout << "Listen error\n";

        closesocket(mysocket);
        WSACleanup();
        DeleteCriticalSection(&cs);

        return -1;
    }

    cout << "Server started on "
         << SERVERADDR
         << ":"
         << PORT
         << "\n";

    cout << "Waiting connections...\n";

    sockaddr_in client_addr{};
    int client_addr_size = sizeof(client_addr);

    SOCKET client_socket;

    while ((client_socket =
        accept(mysocket,
               (sockaddr*)&client_addr,
               &client_addr_size)) != INVALID_SOCKET)
    {
        SOCKET* pclient = new SOCKET;

        *pclient = client_socket;

        DWORD thID;

        CreateThread(
            NULL,
            0,
            ConToClient,
            pclient,
            0,
            &thID
        );
    }

    closesocket(mysocket);

    WSACleanup();

    DeleteCriticalSection(&cs);

    return 0;
}

DWORD WINAPI ConToClient(LPVOID client_socket) {

    SOCKET my_sock = *(SOCKET*)client_socket;

    delete (SOCKET*)client_socket;

    char buff[1024];

    int len;

    string nick;
    string nickLower;

    bool nickAccepted = false;

    // ===== VERIFICATION DU NICKNAME =====
    while (!nickAccepted) {

        len = recv(my_sock, buff, 1024, 0);

        if (len <= 0) {
            closesocket(my_sock);
            return 0;
        }

        buff[len] = '\0';

        nick = buff;

        // version minuscule
        nickLower = ToLower(nick);

        EnterCriticalSection(&cs);

        // vérifie si déjà pris
        if (clients.find(nickLower) != clients.end()) {

            string errorMsg = "NICKNAME_TAKEN\n";

            send(
                my_sock,
                errorMsg.c_str(),
                errorMsg.size(),
                0
            );

            LeaveCriticalSection(&cs);
        }
        else {

            // stocke en minuscule
            clients[nickLower] = my_sock;

            nclients++;

            cout << "+ "
                 << nick
                 << " connected\n";

            PRINTNUSERS

            string acceptMsg = "NICKNAME_ACCEPTED\n";

            send(
                my_sock,
                acceptMsg.c_str(),
                acceptMsg.size(),
                0
            );

            LeaveCriticalSection(&cs);

            nickAccepted = true;
        }
    }

    // message de connexion
    string welcomeMsg =
        "=== " + nick + " joined the chat ===\n";

    EnterCriticalSection(&cs);

    for (auto& p : clients) {

        if (p.second != my_sock) {

            send(
                p.second,
                welcomeMsg.c_str(),
                welcomeMsg.size(),
                0
            );
        }
    }

    LeaveCriticalSection(&cs);

    // ===== BOUCLE PRINCIPALE =====
    while ((len = recv(my_sock, buff, 1024, 0)) > 0) {

        buff[len] = '\0';

        string msg(buff);

        // ===== MESSAGE PRIVE =====
        if (msg[0] == '@') {

            size_t spacePos = msg.find(' ');

            if (spacePos != string::npos) {

                string targetNick =
                    msg.substr(1, spacePos - 1);

                // conversion minuscule
                string targetNickLower =
                    ToLower(targetNick);

                string privateMsg =
                    "[PM from "
                    + nick
                    + "]: "
                    + msg.substr(spacePos + 1)
                    + "\n";

                EnterCriticalSection(&cs);

                auto it =
                    clients.find(targetNickLower);

                if (it != clients.end()) {

                    send(
                        it->second,
                        privateMsg.c_str(),
                        privateMsg.size(),
                        0
                    );

                    string confirm =
                        "[PM to "
                        + targetNick
                        + "]: "
                        + msg.substr(spacePos + 1)
                        + "\n";

                    send(
                        my_sock,
                        confirm.c_str(),
                        confirm.size(),
                        0
                    );
                }
                else {

                    string error =
                        "User "
                        + targetNick
                        + " not found\n";

                    send(
                        my_sock,
                        error.c_str(),
                        error.size(),
                        0
                    );
                }

                LeaveCriticalSection(&cs);
            }
        }

        // ===== MESSAGE PUBLIC =====
        else {

            string publicMsg =
                "[" + nick + "]: " + msg + "\n";

            cout << publicMsg;

            EnterCriticalSection(&cs);

            for (auto& p : clients) {

                send(
                    p.second,
                    publicMsg.c_str(),
                    publicMsg.size(),
                    0
                );
            }

            LeaveCriticalSection(&cs);
        }
    }

    // ===== DECONNEXION =====
    EnterCriticalSection(&cs);

    clients.erase(nickLower);

    nclients--;

    cout << "- "
         << nick
         << " disconnected\n";

    PRINTNUSERS

    string leaveMsg =
        "=== " + nick + " left the chat ===\n";

    for (auto& p : clients) {

        send(
            p.second,
            leaveMsg.c_str(),
            leaveMsg.size(),
            0
        );
    }

    LeaveCriticalSection(&cs);

    closesocket(my_sock);

    return 0;
}
