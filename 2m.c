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

