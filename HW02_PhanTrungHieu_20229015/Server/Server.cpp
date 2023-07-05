#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include "streamtcp.h"
#include "service.h"

#pragma comment (lib, "Ws2_32.lib")

#define SERVER_ADDR "127.0.0.1"
#define MAX_CLIENT 128


//echoThread - Thread to receive the message from client and echo
unsigned _stdcall serviceTheard(void *param);

int main(int argc, char *argv[]) {
    //Parameter Validation
	if (argc != 2) {
		printf_s("Must type 2 arguments.\n");
		exit(1);
	}

	//File account.txt in folder "Debug"
	getAccount("account.txt");

	//Initiate Winsock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		printf("Winsock 2.2 is not supported\n");
		return 0;
	}

	//Construct socket
	SOCKET listenSock;
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (listenSock == INVALID_SOCKET) {
		printf("Error %d: Cannot create server socket.", WSAGetLastError());
		return 0;
	}

	//Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	int serverPort = atoi(argv[1]);
	serverAddr.sin_port = htons(serverPort);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);

	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		printf("(Error: %d)Cannot associate a local address with server socket.", WSAGetLastError());
		return 0;
	}

	//Listen request from client
	if (listen(listenSock, MAX_CLIENT)) {
		printf("(Error: %d)Cannot place server socket in state LISTEN.", WSAGetLastError());
		return 0;
	}
	printf("Server started!\n");

	//Communicate with client
	SOCKET connSock;
	sockaddr_in clientAddr;
	char clientIP[INET_ADDRSTRLEN];
	int clientAddrLen = sizeof(clientAddr), clientPort;
	while (1) {
		//Accept request
		connSock = accept(listenSock, (sockaddr *)&clientAddr, &clientAddrLen);
		if (connSock == SOCKET_ERROR) {
			printf("(Error: %d)Cannot permit incoming connection.", WSAGetLastError());
			return 0;
		}
		else {
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
			clientPort = ntohs(clientAddr.sin_port);
			printf("Accept incoming connection from %s:%d\n", clientIP, clientPort);
			Session session(connSock, clientIP, clientPort);
			_beginthreadex(0, 0, serviceTheard, (void *)&session, 0, 0);
		}
	}

	//Close socket
	closesocket(listenSock);

	//Terminate Winsock
	WSACleanup();

	return 0;
}

unsigned _stdcall serviceTheard(void *param) {
	//Create session connect socket
	Session session (*(Session *)param);
	SOCKET connSock = session.connSock;

	char buff[BUFF_SIZE];
	int ret;
	while (1) {
		//Receive request
		ret = recvStream(connSock, buff);
		if (ret == SOCKET_ERROR) {
			printf("Error %d: Cannot receive data.\n", WSAGetLastError());
			break;
		}
		else if (ret == 0) {
			printf("Client disconnects.\n");
			break;
		}
		else {
			int result = handleRequests(session, buff);
			sprintf_s(buff, BUFF_SIZE, "%d", result);

		    //Echo to client
			ret = sendStream(connSock, buff);
			if (ret == SOCKET_ERROR) {
				printf("Error %d: Cannot send data.\n", WSAGetLastError());
				break;
			}
		}
	}

	//Remove session from session list
	sessionList.remove(&session);

	//Close socket
	closesocket(connSock);

	return 0;
}



