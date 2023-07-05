#include <iostream>
#include <ws2tcpip.h>
#include <winsock2.h>
#include "prefix.h"
#include "streamtcp.h"

#pragma comment(lib, "Ws2_32.lib")

/*
Function service: Let user choose service
Param[out] request: A pointer to request
Return: None
*/
void service(char *request);

int main(int argc, char *argv[]) {
	//Parameter Validation
	if (argc != 3) {
		printf_s("Must type 3 arguments.\n");
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
	SOCKET connSock;
	connSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connSock == INVALID_SOCKET) {
		printf("Error %d: Cannot create server socket.", WSAGetLastError());
		return 0;
	}

	//Specify server address
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	char *serverIp = argv[1];
	int serverPort = atoi(argv[2]);
	serverAddr.sin_port = htons(serverPort);
	inet_pton(AF_INET, serverIp, &serverAddr.sin_addr);

	//Request to connect server
	if (connect(connSock, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		printf("\nError: %d", WSAGetLastError());
		closesocket(connSock);
		return 0;
	}

	//Communicate with server
	char buff[BUFF_SIZE];
	int ret;
	while (1) {
		//Send message
		service(buff);
		ret = sendStream(connSock, buff);
		if (ret == SOCKET_ERROR) {
			printf("Error %d: Cannot send data.\n", WSAGetLastError());
			continue;
		}

		//Receive message
		ret = recvStream(connSock, buff);
		if (ret == SOCKET_ERROR)
			printf("Error %d: Cannot receive data.\n", WSAGetLastError());
		else {
			std::cout << messageResponse(buff) << "\n";
		}
	}

	//Close socket
	closesocket(connSock);

	//Terminate Winsock
	WSACleanup();

	return 0;
}

void service(char *request) {
	printf("\n\nChoose service:\n");
	printf("1.Login\n");
	printf("2.Post message\n");
	printf("3.Logout\n\n");

	int choice;
	char temp[BUFF_SIZE];

	while (1) {
		printf("Please choose one mode:");
		scanf_s("%d", &choice);

		//Clear input buffer
		int c;
		while ((c = getchar()) != '\n' && c != EOF);

		switch (choice) {
		//Login service
		case 1: {
			printf("Enter username:");
			gets_s(temp, BUFF_SIZE);
			sprintf_s(request, BUFF_SIZE, "USER %s", temp);
			return;
		}
		//Post message service
		case 2: {
			printf("Enter message:");
			gets_s(temp, BUFF_SIZE);
			sprintf_s(request, BUFF_SIZE, "POST %s", temp);
			return;
		}
		//Logout service
		case 3: {
			strcpy_s(request, BUFF_SIZE, "BYE");
			return;
		}
		//Invalid input
		default: {
			printf("Invalid input.Choose again\n");
			continue;
		}
		}
	}
}
