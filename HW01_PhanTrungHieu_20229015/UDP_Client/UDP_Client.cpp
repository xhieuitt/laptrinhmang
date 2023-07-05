#include "stdio.h"
#include "winSock2.h"
#include "wS2tcpip.h"
#pragma comment(lib, "Ws2_32.lib")
#define BUFF_SIZE 2048

/*
Function splitStr: Split a string with delimiter and print
Param [in]str: Null-terminated string
Return: None
*/
void splitStr(char *str);

int main(int argc, char *argv[]) {
	//Parameter Validation
	if (argc != 3) {
		printf_s("Must type 3 arguments.\n");
		exit(1);
	}

	//Inittiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		printf("Version is not supported.\n");
		return 0;
	}
	//Construct socket 
	SOCKET client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (client == INVALID_SOCKET) {
		printf("Error %d: Cannot create server socket.", WSAGetLastError());
		return 0;
	}
	printf("Client started!\n");

	//Specify server address
	char *serverIp = argv[1];
	int serverPort = atoi(argv[2]);
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	inet_pton(AF_INET, serverIp, &serverAddr.sin_addr);

	//Communicate with server
	char buff[BUFF_SIZE];
	int ret, serverAddrLen = sizeof(serverAddr);

	while (1) {
		//Send message
		printf("Send to server: ");
		gets_s(buff, BUFF_SIZE);
		//Exit loop if user enter an empty string
		if (!strlen(buff)) break;
		ret = sendto(client, buff, strlen(buff), 0, (sockaddr *)&serverAddr, serverAddrLen);
		if (ret == SOCKET_ERROR)
			printf("Error %d: Cannot send mesage.\n", WSAGetLastError());

		//Receive message
		ret = recvfrom(client, buff, BUFF_SIZE, 0, (sockaddr *)&serverAddr, &serverAddrLen);
		if (ret == SOCKET_ERROR) {
			printf("Error %d: Cannot receive message.\n", WSAGetLastError());
		}
		else if (strlen(buff) > 0) {
			buff[ret] = 0;
			//Failed message
			if (buff[0] == '-') {
				printf("%s", buff + 1);
			}
			//Successed message
			else if (buff[0] == '+') {
				printf("IP Addresses:\n");
				splitStr(buff + 1);
			}
		}
	} //End while

	  // Close socket
	closesocket(client);

	// Terminate Winsock
	WSACleanup();
}
void splitStr(char *str) {
	char *token = nullptr;
	char *next_token = nullptr;
	const char *delim = " ";

	//Split result into one/many addresses
	token = strtok_s(str, delim, &next_token);
	while (token != nullptr) {
		printf("%s\n", token);
		token = strtok_s(nullptr, delim, &next_token);
	}
}




