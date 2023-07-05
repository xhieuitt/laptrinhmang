#include "stdio.h"
#include "wS2tcpip.h"
#include "streamtcp.h"
#define SERVER_ADDR "127.0.0.1"
#pragma comment(lib, "Ws2_32.lib")

/*
Function sumStr: Caculates the sum of all digits in a string
Param[in] str: A null-terminated string
Return: The sum of all digits.
        -1 if the string has non-digit characters
*/
int sumStr(const char *str);

int main(int argc, char* argv[]) {
	//Parameter Validation
	if (argc != 2) {
		printf_s("Must type 2 arguments.\n");
		exit(1);
	}

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
	if (listen(listenSock, 10)) {
		printf("(Error: %d)Cannot place server socket in state LISTEN.", WSAGetLastError());
		return 0;
	}
	printf("Server started!\n");

	//Communicate with client
	sockaddr_in clientAddr;
	char buff[BUFF_SIZE], res[BUFF_SIZE], clientIP[INET_ADDRSTRLEN];
	int ret, clientAddrLen = sizeof(clientAddr), clientPort;
	SOCKET connSock;

	while (1) {//Communicate to many client
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
		}

		while (1) {
			//Receive message from client
			ret = recvStream(connSock, buff);
			if (ret == SOCKET_ERROR) {
				printf("Error %d: Cannot receive data.\n", WSAGetLastError());
				break;
			}
			else if (!ret) {
				printf("Client disconnects.\n");
				break;
			}
			else {
				printf("Receive from client[%s:%d]: %s\n", clientIP, clientPort, buff);
				//Send result to client
				ret = sumStr(buff);
				if (ret == -1)
					sprintf_s(res, BUFF_SIZE, "-String contains non-number character");
				else
					sprintf_s(res, BUFF_SIZE, "+The result is %d", ret);
				ret = sendStream(connSock, res);
				if (ret == SOCKET_ERROR) {
					printf("Error %d: Cannot send data.\n", WSAGetLastError());
					break;
				}
			}
		} //end communicating

		  //Close socket
		closesocket(connSock);
	}
	closesocket(listenSock);

	//Terminate Winsock
	WSACleanup();

	return 0;
}
int sumStr(const char *str) {
	int sum = 0;
	for (size_t i = 0; i < strlen(str); i++) {
		if (!isdigit(str[i])) //Check if char is number
			return -1;
		sum += str[i] - '0';
	}
	return sum;
}