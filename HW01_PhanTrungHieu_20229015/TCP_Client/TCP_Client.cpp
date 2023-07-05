#include "stdio.h"
#include "wS2tcpip.h"
#include "streamtcp.h"
#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char* argv[]) {
	//Parameter Validation
	if (argc != 3) {
		printf_s("Must type 3 arguments.\n");
		exit(1);
	}

	//Inittiate Winsock
	WSADATA wsaData;
	WORD WVersion = MAKEWORD(2, 2);
	if (WSAStartup(WVersion, &wsaData)) {
		printf("Winsock 2.2 is not supported\n");
		return 0;
	}

	//Construct socket
	SOCKET client;
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client == INVALID_SOCKET) {
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
	if (connect(client, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		printf("Error %d: Cannot connect server.", WSAGetLastError());
		return 0;
	}
	printf("Connected server!\n");

	//Communicate with server
	char buff[BUFF_SIZE];
	int ret;
	while (1) {
		//Send message
		printf("Send to server: ");
		gets_s(buff, BUFF_SIZE);
		//Exit loop if user enter an empty string
		if (!strlen(buff)) break;

		ret = sendStream(client, buff);
		if (ret == SOCKET_ERROR) {
			printf("Error %d: Cannot send data.\n", WSAGetLastError());
			continue;
		}

		//Receive message
		ret = recvStream(client, buff);
		if (ret == SOCKET_ERROR)
			printf("Error %d: Cannot receive data.\n", WSAGetLastError());
		else if (strlen(buff) > 0) {
			buff[ret] = 0;
			//Failed message
			if (buff[0] == '-')
				printf("Failed: %s\n", buff + 1);
			//Successed message
			else if(buff[0] == '+')
				printf("Successed: %s\n", buff + 1);
		}
	}

	//Close socket
	closesocket(client);

	//Terminate Winsock
	WSACleanup();

	return 0;
}