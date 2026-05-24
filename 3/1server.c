#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <sstream>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

int main() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cerr << "WSAStartup failed: " << result << "\n";
        return result;
    }

    struct addrinfo* addr = NULL;
    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    result = getaddrinfo("127.0.0.1", "8000", &hints, &addr);
    if (result != 0) {
        cerr << "getaddrinfo failed: " << result << "\n";
        WSACleanup();
        return 1;
    }

    int listen_socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (listen_socket == INVALID_SOCKET) {
        cerr << "Error at socket: " << WSAGetLastError() << "\n";
        freeaddrinfo(addr);
        WSACleanup();
        return 1;
    }

    result = bind(listen_socket, addr->ai_addr, (int)addr->ai_addrlen);
    if (result == SOCKET_ERROR) {
        cerr << "bind failed with error: " << WSAGetLastError() << "\n";
        freeaddrinfo(addr);
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "listen failed with error: " << WSAGetLastError() << "\n";
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    cerr << "Server started on port 8000" << endl;
    cerr << "Open browser: http://localhost:8000" << endl;
    cerr << "Data will be saved to storage.txt" << endl;

    const int max_client_buffer_size = 1024;
    char buf[max_client_buffer_size];
    int client_socket = INVALID_SOCKET;

    for (;;) {
        client_socket = accept(listen_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            cerr << "accept failed: " << WSAGetLastError() << "\n";
            closesocket(listen_socket);
            WSACleanup();
            return 1;
        }

        result = recv(client_socket, buf, max_client_buffer_size, 0);
        stringstream response;
        stringstream response_body;

        if (result == SOCKET_ERROR) {
            cerr << "recv failed: " << result << "\n";
            closesocket(client_socket);
        }
        else if (result == 0) {
            cerr << "connection closed...\n";
        }
        else if (result > 0) {
            buf[result] = '\0';

            // HANDLE POST REQUEST - SAVE DATA TO FILE
            if (strstr(buf, "POST") != NULL) {
                char* body = strstr(buf, "\r\n\r\n");
                if (body != NULL) {
                    body += 4;

                    if (strstr(body, "data=") != NULL) {
                        body = strstr(body, "data=") + 5;
                    }

                    char* end = body;
                    while (*end != '\0' && *end != '\r' && *end != '\n') {
                        end++;
                    }
                    *end = '\0';

                    // SAVE TO FILE STORAGE
                    FILE* file = fopen("storage.txt", "a");
                    if (file != NULL) {
                        fprintf(file, "%s\n", body);
                        fclose(file);
                        cerr << "Saved to storage: " << body << endl;
                    }
                }
            }

            // READ ALL SAVED DATA FROM FILE
            string savedItems = "";
            FILE* readFile = fopen("storage.txt", "r");
            if (readFile != NULL) {
                char line[1024];
                while (fgets(line, sizeof(line), readFile)) {
                    char* newline = strchr(line, '\n');
                    if (newline != NULL) {
                        *newline = '\0';
                    }
                    savedItems += "<li>" + string(line) + "</li>";
                }
                fclose(readFile);
            }
            if (savedItems.empty()) {
                savedItems = "<li>No data yet</li>";
            }

            // GENERATE HTML PAGE WITH FORM (ENGLISH VERSION)
            response_body << "<!DOCTYPE html>\n"
                << "<html>\n"
                << "<head>\n"
                << "    <title>Data Storage</title>\n"
                << "    <style>\n"
                << "        body { font-family: Arial; margin: 40px; background: #f0f0f0; }\n"
                << "        .container { max-width: 600px; margin: auto; background: white; padding: 20px; border-radius: 10px; }\n"
                << "        input { width: 70%; padding: 10px; margin: 10px 0; }\n"
                << "        button { padding: 10px 20px; background: #007bff; color: white; border: none; cursor: pointer; }\n"
                << "        ul { background: #e9ecef; padding: 20px; border-radius: 5px; }\n"
                << "        li { margin: 10px 0; }\n"
                << "        h1, h2 { color: #333; }\n"
                << "    </style>\n"
                << "</head>\n"
                << "<body>\n"
                << "    <div class='container'>\n"
                << "        <h1>My Data Storage</h1>\n"
                << "        <form method='POST'>\n"
                << "            <input type='text' name='data' placeholder='Enter text to save...' required>\n"
                << "            <button type='submit'>Save</button>\n"
                << "        </form>\n"
                << "        <h2>Saved data:</h2>\n"
                << "        <ul>\n"
                << savedItems
                << "        </ul>\n"
                << "    </div>\n"
                << "</body>\n"
                << "</html>\n";

            response << "HTTP/1.1 200 OK\r\n"
                << "Content-Type: text/html\r\n"
                << "Content-Length: " << response_body.str().length() << "\r\n"
                << "Connection: close\r\n"
                << "\r\n"
                << response_body.str();

            result = send(client_socket, response.str().c_str(), response.str().length(), 0);
            if (result == SOCKET_ERROR) {
                cerr << "send failed: " << WSAGetLastError() << "\n";
            }
            closesocket(client_socket);
        }
    }

    closesocket(listen_socket);
    freeaddrinfo(addr);
    WSACleanup();
    return 0;
}
