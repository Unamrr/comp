// SERVER TCP - Камень Ножницы Бумага (максимально просто)
#include <iostream>  
#include <winsock2.h> 
#include <windows.h> 
#include <string>
#include <cstdlib>
#include <ctime>
#pragma comment (lib, "Ws2_32.lib")  
using namespace std;
#define SRV_PORT 1234  
#define BUF_SIZE 64  

int main() {
    srand(time(0));
    
    char buff[1024];
    if (WSAStartup(0x0202, (WSADATA*)&buff[0]))
    {
        cout << "Error WSAStartup \n" << WSAGetLastError();
        return -1;
    }
    
    SOCKET s, s_new;
    int from_len;
    char buf[BUF_SIZE] = { 0 };
    sockaddr_in sin, from_sin;
    
    s = socket(AF_INET, SOCK_STREAM, 0);
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(SRV_PORT);
    bind(s, (sockaddr*)&sin, sizeof(sin));
    
    string msg, msg1;
    listen(s, 3);
    
    while (1) {
        from_len = sizeof(from_sin);
        s_new = accept(s, (sockaddr*)&from_sin, &from_len);
        cout << "new connected client! " << endl;
        
        msg = "1-камень 2-ножницы 3-бумага Bye-выход: ";
        
        while (1) {
            send(s_new, (char*)&msg[0], msg.size(), 0);
            from_len = recv(s_new, (char*)buf, BUF_SIZE, 0);
            buf[from_len] = 0;
            msg1 = (string)buf;
            cout << msg1 << endl;
            
            if (msg1 == "Bye") break;
            
            int server = rand() % 3 + 1;
            string res;
            
            if (msg1 == "1") {
                if (server == 1) res = "Ничья (камень)";
                else if (server == 2) res = "Ты выиграл! (камень бьет ножницы)";
                else res = "Ты проиграл! (бумага накрыла камень)";
            }
            else if (msg1 == "2") {
                if (server == 2) res = "Ничья (ножницы)";
                else if (server == 3) res = "Ты выиграл! (ножницы режут бумагу)";
                else res = "Ты проиграл! (камень сломал ножницы)";
            }
            else if (msg1 == "3") {
                if (server == 3) res = "Ничья (бумага)";
                else if (server == 1) res = "Ты выиграл! (бумага накрыла камень)";
                else res = "Ты проиграл! (ножницы режут бумагу)";
            }
            else {
                res = "Введи 1,2 или 3";
            }
            
            msg = res + "\n1-камень 2-ножницы 3-бумага Bye-выход: ";
        }
        
        cout << "client is lost";
        closesocket(s_new);
    }
    
    return 0;
}
// SERVER TCP с игрой Камень-Ножницы-Бумага (работает с вашим клиентом)
#include <iostream>  
#include <winsock2.h> 
#include <windows.h> 
#include <string>
#include <cstdlib>
#include <ctime>
#pragma comment (lib, "Ws2_32.lib")  
using namespace std;
#define SRV_PORT 1234  
#define BUF_SIZE 64  

int main() {
    srand(time(0));
    
    char buff[1024];
    if (WSAStartup(0x0202, (WSADATA*)&buff[0]))
    {
        cout << "Error WSAStartup \n" << WSAGetLastError();
        return -1;
    }
    
    SOCKET s, s_new;
    int from_len;
    char buf[BUF_SIZE] = { 0 };
    sockaddr_in sin, from_sin;
    
    s = socket(AF_INET, SOCK_STREAM, 0);
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(SRV_PORT);
    bind(s, (sockaddr*)&sin, sizeof(sin));
    
    string msg, msg1;
    listen(s, 3);
    
    cout << "=== ИГРА КАМЕНЬ-НОЖНИЦЫ-БУМАГА ===" << endl;
    
    while (1) {
        from_len = sizeof(from_sin);
        s_new = accept(s, (sockaddr*)&from_sin, &from_len);
        cout << "new connected client! " << endl;
        
        msg = "=== КАМЕНЬ-НОЖНИЦЫ-БУМАГА ===\n1 - камень\n2 - ножницы\n3 - бумага\nBye - выход\nВаш выбор: ";
        
        while (1) {
            // Отправляем приглашение
            send(s_new, (char*)&msg[0], msg.size(), 0);
            
            // Получаем ответ клиента
            from_len = recv(s_new, (char*)buf, BUF_SIZE, 0);
            if (from_len <= 0) break;
            
            buf[from_len] = 0;
            msg1 = (string)buf;
            
            // Удаляем символ новой строки если есть
            if (!msg1.empty() && msg1.back() == '\n') {
                msg1.pop_back();
            }
            
            cout << "Клиент: [" << msg1 << "]" << endl;
            
            // Проверка на выход
            if (msg1 == "Bye") break;
            
            // Сервер случайно выбирает
            int server_choice = rand() % 3 + 1;
            string server_str;
            if (server_choice == 1) server_str = "камень";
            else if (server_choice == 2) server_str = "ножницы";
            else server_str = "бумага";
            
            // Определяем результат
            string result;
            if (msg1 == "1") {
                if (server_choice == 1) result = "Ничья! Сервер тоже камень";
                else if (server_choice == 2) result = "Ты победил! Камень бьет ножницы";
                else result = "Ты проиграл! Бумага накрывает камень";
            }
            else if (msg1 == "2") {
                if (server_choice == 2) result = "Ничья! Сервер тоже ножницы";
                else if (server_choice == 3) result = "Ты победил! Ножницы режут бумагу";
                else result = "Ты проиграл! Камень ломает ножницы";
            }
            else if (msg1 == "3") {
                if (server_choice == 3) result = "Ничья! Сервер тоже бумага";
                else if (server_choice == 1) result = "Ты победил! Бумага накрывает камень";
                else result = "Ты проиграл! Ножницы режут бумагу";
            }
            else {
                result = "Неверный ввод! Введите 1, 2 или 3";
            }
            
            // Формируем ответ
            msg = "\nСервер выбрал: " + server_str + "\n";
            msg += result + "\n";
            msg += "================================\n";
            msg += "1 - камень\n2 - ножницы\n3 - бумага\nBye - выход\nВаш выбор: ";
        }
        
        cout << "client is lost" << endl;
        closesocket(s_new);
    }
    
    closesocket(s);
    WSACleanup();
    return 0;
}

// SERVER TCP с игрой Камень-Ножницы-Бумага
#include <iostream>  
#include <winsock2.h> 
#include <windows.h> 
#include <string>
#include <cstdlib>
#include <ctime>
#pragma comment (lib, "Ws2_32.lib")  
using namespace std;
#define SRV_PORT 1234  
#define BUF_SIZE 64  
const string QUEST = "Who are you?\n";

int main() {
    srand(time(0)); // для случайного выбора сервера
    
    char buff[1024];
    if (WSAStartup(0x0202, (WSADATA*)&buff[0]))
    {
        cout << "Error WSAStartup \n" << WSAGetLastError();
        return -1;
    }
    
    SOCKET s, s_new;
    int from_len;
    char buf[BUF_SIZE] = { 0 };
    sockaddr_in sin, from_sin;
    
    s = socket(AF_INET, SOCK_STREAM, 0);
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(SRV_PORT);
    bind(s, (sockaddr*)&sin, sizeof(sin));
    
    string msg, msg1;
    listen(s, 3);
    
    cout << "=== ИГРА КАМЕНЬ-НОЖНИЦЫ-БУМАГА ===" << endl;
    
    while (1) {
        from_len = sizeof(from_sin);
        s_new = accept(s, (sockaddr*)&from_sin, &from_len);
        cout << "new connected client! " << endl;
        
        msg = "=== ИГРА КАМЕНЬ-НОЖНИЦЫ-БУМАГА ===\n";
        msg += "Введите: 1-камень, 2-ножницы, 3-бумага или Bye для выхода\n";
        msg += "Ваш выбор: ";
        
        while (1) {
            send(s_new, (char*)&msg[0], msg.size(), 0);
            from_len = recv(s_new, (char*)buf, BUF_SIZE, 0);
            buf[from_len] = 0;
            msg1 = (string)buf;
            cout << "Клиент выбрал: " << msg1 << endl;
            
            if (msg1 == "Bye") break;
            
            // Сервер случайно выбирает: 1-камень, 2-ножницы, 3-бумага
            int server_choice = rand() % 3 + 1;
            string server_str;
            if (server_choice == 1) server_str = "камень";
            else if (server_choice == 2) server_str = "ножницы";
            else server_str = "бумага";
            
            // Определяем победителя
            string result;
            if (msg1 == "1") { // клиент - камень
                if (server_choice == 1) result = "Ничья! Сервер тоже выбрал камень";
                else if (server_choice == 2) result = "Ты победил! Камень бьет ножницы";
                else result = "Ты проиграл! Бумага накрывает камень";
            }
            else if (msg1 == "2") { // клиент - ножницы
                if (server_choice == 2) result = "Ничья! Сервер тоже выбрал ножницы";
                else if (server_choice == 3) result = "Ты победил! Ножницы режут бумагу";
                else result = "Ты проиграл! Камень ломает ножницы";
            }
            else if (msg1 == "3") { // клиент - бумага
                if (server_choice == 3) result = "Ничья! Сервер тоже выбрал бумагу";
                else if (server_choice == 1) result = "Ты победил! Бумага накрывает камень";
                else result = "Ты проиграл! Ножницы режут бумагу";
            }
            else {
                result = "Неверный ввод! Введите 1, 2 или 3";
            }
            
            msg = "Сервер выбрал: " + server_str + "\n";
            msg += "Результат: " + result + "\n";
            msg += "----------------------------------------\n";
            msg += "Введите: 1-камень, 2-ножницы, 3-бумага или Bye для выхода\n";
            msg += "Ваш выбор: ";
        }
        
        cout << "client is lost";
        closesocket(s_new);
    }
    
    return 0;
}



// SERVER TCP 

#include <iostream>  
#include <winsock2.h> 
#include <windows.h> 
#include <string> 
#pragma comment (lib, "Ws2_32.lib")  
using namespace std;
#define SRV_PORT 1234  
#define BUF_SIZE 64  
const string QUEST = "Who are you?\n";
int main() {
	char buff[1024];
	if (WSAStartup(0x0202, (WSADATA*)&buff[0]))
	{
		cout << "Error WSAStartup \n" << WSAGetLastError();   // Ошибка!
		return -1;
	}
	SOCKET s, s_new;
	int from_len;
	char buf[BUF_SIZE] = { 0 };
	sockaddr_in sin, from_sin;
	s = socket(AF_INET, SOCK_STREAM, 0);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = 0;
	sin.sin_port = htons(SRV_PORT);
	bind(s, (sockaddr*)&sin, sizeof(sin));
	string msg, msg1;
	listen(s, 3);
	while (1) {
		from_len = sizeof(from_sin);
		s_new = accept(s, (sockaddr*)&from_sin, &from_len);
		cout << "new connected client! " << endl;
		msg = QUEST;
		while (1) {
			send(s_new, (char*)&msg[0], msg.size(), 0);
			from_len = recv(s_new, (char*)buf, BUF_SIZE, 0);
			buf[from_len] = 0;
			msg1 = (string)buf;
			cout << msg1 << endl;;
			if (msg1 == "Bye") break;
			getline(cin, msg);
		}
		cout << "client is lost";
		closesocket(s_new);
	}
	return 0;
}

// CLIENT TCP
#include <iostream>  
#define _WINSOCK_DEPRECATED_NO_WARNINGS  
// подавление предупреждений библиотеки winsock2
#include <winsock2.h> 
#include <string>
#include <windows.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma warning(disable: 4996)  // подавление предупреждения 4996 
using namespace std;
#define SRV_HOST "localhost"  
#define SRV_PORT 1234 
#define CLNT_PORT 1235  
#define BUF_SIZE 64  
char TXT_ANSW[] = "I am your student\n";
int main() {
	char buff[1024];
	if (WSAStartup(0x0202, (WSADATA*)&buff[0])) {
		cout << "Error WSAStartup \n" << WSAGetLastError();  // Ошибка!
		return -1;
	}
	SOCKET s;
	int from_len;   char buf[BUF_SIZE] = { 0 };    hostent* hp;
	sockaddr_in clnt_sin, srv_sin;
	s = socket(AF_INET, SOCK_STREAM, 0);
	clnt_sin.sin_family = AF_INET;
	clnt_sin.sin_addr.s_addr = 0;
	clnt_sin.sin_port = htons(CLNT_PORT);
	bind(s, (sockaddr*)&clnt_sin, sizeof(clnt_sin));
	hp = gethostbyname(SRV_HOST);
	srv_sin.sin_port = htons(SRV_PORT);
	srv_sin.sin_family = AF_INET;
	((unsigned long*)&srv_sin.sin_addr)[0] =
		((unsigned long**)hp->h_addr_list)[0][0];
	connect(s, (sockaddr*)&srv_sin, sizeof(srv_sin));
	string mst;
	do {
		from_len = recv(s, (char*)&buf, BUF_SIZE, 0);
		buf[from_len] = 0;
		cout << buf << endl;
		//send (s, (char *)&TXT_ANSW, sizeof(TXT_ANSW),0); 
		getline(cin, mst);
		int msg_size = mst.size();
		send(s, (char*)&mst[0], msg_size, 0);
	} while (mst != "Bye");
	cout << "exit to infinity" << endl;
	cin.get();    closesocket(s);
	return 0;
}

