#include "stdio.h"
#include "winSock2.h"
#include "wS2tcpip.h"
#pragma comment(lib, "Ws2_32.lib")
#define BUFF_SIZE 2048
#define SERVER_ADDR "127.0.0.1"

/*
Function lookupIP: Resolve a domain name
Param [in]hostName: Null-terminated string contains a host name
Param [out]ipAddress: Null-terminated string contains ip addresses of host
Return: None
*/
void lookupIP(const char *hostName, char *ipAddress);

int main(int argc, char *argv[]) {
	//Parameter Validation
	if (argc != 2) {
		printf_s("Must type 2 arguments.\n");
		exit(1);
	}

	// Initialize Winsock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	WSAStartup(wVersion, &wsaData);
	if (WSAStartup(wVersion, &wsaData)) {
		printf("Version is not supported\n");
		return 0;
	}

	// Construct socket
	SOCKET server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (server == INVALID_SOCKET) {
		printf("Error %d: Cannot create server socket.", WSAGetLastError());
		return 0;
	}

	//Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	int serverPort = atoi(argv[1]);
	serverAddr.sin_port = htons(serverPort);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);
	if (bind(server, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		printf("Error %d: Cannot bind this address.", WSAGetLastError());
		return 0;
	}
	printf("Server started!\n");

	// Communicate with client
	sockaddr_in clientAddr;
	char buff[BUFF_SIZE], ipFound[BUFF_SIZE], clientIP[INET_ADDRSTRLEN];
	int ret, clientAddrLen = sizeof(clientAddr), clientPort;

	while (1) {
		//Receive message
		ret = recvfrom(server, buff, BUFF_SIZE, 0, (sockaddr *)&clientAddr, &clientAddrLen);
		if (ret == SOCKET_ERROR)
			printf("Error : %d", WSAGetLastError());
		else if (strlen(buff) > 0) {
			buff[ret] = 0;
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
			clientPort = ntohs(clientAddr.sin_port);
			printf("Receive from client[%s:%d]: %s\n", clientIP, clientPort, buff);

			//Resolving domain name
			lookupIP(buff, ipFound);

			//Send to client
			ret = sendto(server, ipFound, strlen(ipFound), 0, (sockaddr *)&clientAddr, sizeof(clientAddr));
			if (ret == SOCKET_ERROR)
				printf("Error %d: Cannot send data.", WSAGetLastError());
		}
	} //End while

	  //Close socket
	closesocket(server);

	//Terminate Winsock
	WSACleanup();

	return 0;
}
void lookupIP(const char *hostName, char *ipAddress) {
	int rc;
	addrinfo *result = nullptr; //Pointer to the linked-list
								//Containing information about the host
	addrinfo *ptr = nullptr; //Pointer to the linked-list
	addrinfo hints; //Optional pointer to a struct addrinfo
	sockaddr_in *address;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; //Only focus on IPv4 address

	//Get the address info
	char ipStr[INET_ADDRSTRLEN];
	rc = getaddrinfo(hostName, NULL, &hints, &result);
	if (!rc) {
		// Get each address and append to the result, a " " between two address
		sprintf_s(ipAddress, BUFF_SIZE, "+");
		for (ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
			address = (sockaddr_in *)ptr->ai_addr;
			inet_ntop(AF_INET, &address->sin_addr, ipStr, sizeof(ipStr));
			strcat_s(ipAddress, BUFF_SIZE, ipStr);
			strcat_s(ipAddress, BUFF_SIZE, " ");
		}
	}
	else {
		sprintf_s(ipAddress, BUFF_SIZE, "-Not found information\n");
	}

	//Free linked-list
	freeaddrinfo(result);
}
