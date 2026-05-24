#define _CRT_SECURE_NO_WARNINGS
#include <string>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable: 4996)

using namespace std;

int main() {
    WSADATA ws;
    SOCKET s;
    sockaddr_in adr;
    HOSTENT* hn;
    char buff[65535];

    if (WSAStartup(0x0202, &ws) != 0) {
        cout << "WSAStartup error" << endl;
        return -1;
    }

    if (INVALID_SOCKET == (s = socket(AF_INET, SOCK_STREAM, 0))) {
        cout << "Socket creation error" << endl;
        return -1;
    }

    if (NULL == (hn = gethostbyname("localhost"))) {
        cout << "Cannot resolve localhost" << endl;
        return -1;
    }

    adr.sin_family = AF_INET;
    ((unsigned long*)&adr.sin_addr)[0] = ((unsigned long**)hn->h_addr_list)[0][0];
    adr.sin_port = htons(8000);

    if (SOCKET_ERROR == connect(s, (sockaddr*)&adr, sizeof(adr))) {
        cout << "Connection failed. Make sure server is running." << endl;
        return -1;
    }

    string userData;
    cout << "Enter data to save on server: ";
    getline(cin, userData);

    string request = "POST / HTTP/1.1\r\n";
    request += "Host: localhost\r\n";
    request += "Content-Length: " + to_string(userData.length() + 5) + "\r\n";
    request += "Content-Type: application/x-www-form-urlencoded\r\n";
    request += "\r\n";
    request += "data=" + userData;

    if (SOCKET_ERROR == send(s, request.c_str(), request.length(), 0)) {
        cout << "Send error" << endl;
        return -1;
    }

    cout << "\n--- Server response ---" << endl;

    int len = 0;
    do {
        if (SOCKET_ERROR == (len = recv(s, (char*)&buff, 65535, 0))) {
            cout << "Receive error" << endl;
            return -1;
        }
        for (int i = 0; i < len; i++) {
            cout << buff[i];
        }
    } while (len != 0);

    cout << "\n-----------------------" << endl;

    if (SOCKET_ERROR == closesocket(s)) {
        cout << "Close socket error" << endl;
        return -1;
    }

    cout << "\nPress Enter to exit...";
    cin.get();
    return 1;
}
