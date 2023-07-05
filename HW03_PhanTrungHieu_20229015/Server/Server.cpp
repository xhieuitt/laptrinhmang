#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <process.h>
#include "streamtcp.h"
#include "service.h"

#pragma comment (lib, "Ws2_32.lib")

#define SERVER_ADDR "127.0.0.1"
#define MAX_CLIENT 128

std::vector<Session> sessionList; //Critical resource
CONDITION_VARIABLE cv; //Use by main thread
CRITICAL_SECTION cs;
bool copyDone; //Check if the child thread has finished copying session list from "sessionList"(critical resource)

//workerThread : child thread
unsigned _stdcall workerThread(void *param);

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

	std::vector<HANDLE> childTheards; //List of child threads is working
    
	SOCKET connSock;
	sockaddr_in clientAddr;
	fd_set readfds, initfds; //Use initfds to initiate readfds at the begining of every loop step
	int ret, nEvents, clientAddrLen, clientPort;
	char clientIP[INET_ADDRSTRLEN];
	char buff[BUFF_SIZE];

	InitializeConditionVariable(&cv);
	InitializeCriticalSection(&cs);
	FD_ZERO(&initfds);
	FD_SET(listenSock, &initfds);

	//Communicate with clients
	while (1) {
		readfds = initfds;		/* Structure assignment */
		nEvents = select(0, &readfds, 0, 0, 0);
		if (nEvents < 0) {
			printf("\nError! Cannot poll sockets: %d", WSAGetLastError());
			break;
		}

		//New client connection
		if (FD_ISSET(listenSock, &readfds)) {
			clientAddrLen = sizeof(clientAddr);
			if ((connSock = accept(listenSock, (sockaddr *)&clientAddr, &clientAddrLen)) < 0) {
				printf("\nError! Cannot accept new connection: %d", WSAGetLastError());
				break;
			}
			else {
				inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
				clientPort = ntohs(clientAddr.sin_port);
				printf("You got a connection from %s:%d\n", clientIP, clientPort); 

				//Add session to "sessionList" and add socket to initfds
				EnterCriticalSection(&cs);
				sessionList.emplace_back(connSock, clientIP, clientPort);
				FD_SET(sessionList.back().connSock, &initfds);
				LeaveCriticalSection(&cs);

				// If number of socket = FD_SETSIZE, move "sessionList" to "workerTheard"
				if (sessionList.size() == FD_SETSIZE) {
					childTheards.push_back((HANDLE)_beginthreadex(nullptr, 0, workerThread, nullptr, 0, 0));

					//While the child thread has not finished copying "sessionList", the main thread waited 
					while (copyDone == false)
						SleepConditionVariableCS(&cv, &cs, INFINITE);
					copyDone = false;

					//Clear "sessionList" when the child thread has finished copying
                    EnterCriticalSection(&cs);
					sessionList.clear();
					LeaveCriticalSection(&cs);

					//Clear initfds, continue listen for new connection
					FD_ZERO(&initfds);
					FD_SET(listenSock, &initfds);
					continue;
				}
				if (--nEvents <= 0)
					continue; //No more event
			}
		}

		//Receive data from clients
		EnterCriticalSection(&cs);
		std::vector<Session>::iterator ptr = sessionList.begin();
		while (ptr != sessionList.end() && nEvents > 0) {
			SOCKET clientSock = ptr->connSock;

			if (FD_ISSET(clientSock, &readfds)) {
				--nEvents;
				//Recieve request
				ret = recvStream(clientSock, buff);
				if (ret <= 0) {
					if (ret == 0)
						printf("Client disconnect\n");
					else
						printf("Error %d: Cannot recieve data.\n", WSAGetLastError());

					FD_CLR(clientSock, &initfds);
					closesocket(clientSock);
					ptr = sessionList.erase(ptr);
					continue;
				}
				else if (ret > 0) {
					int result = handleRequests(*ptr, buff);
					sprintf_s(buff, BUFF_SIZE, "%d", result);

					//Send to client
					ret = sendStream(clientSock, buff);
					if (ret == SOCKET_ERROR) {
						printf("Error %d: Cannot send data.\n", WSAGetLastError());
						break;
					}
				}
			}
			ptr++;
		}
		LeaveCriticalSection(&cs);
	}//End while
	closesocket(listenSock);

	//Wait for all child thread done
	WaitForMultipleObjects(childTheards.size(), childTheards.data(), TRUE, INFINITE);

	//Terminate Winsock
	WSACleanup();

	return 0;
}

unsigned _stdcall workerThread(void *param) {
	//Session list in child thread
	std::vector<Session> sessionListInThread (sessionList); //Child thread copying session list from "sessionList"
	copyDone = true;
	WakeConditionVariable(&cv); //Run the main thread when the child thread finished copying "sessionList"

	fd_set readfs, initfds;
	int nEvents, ret;
	char buff[BUFF_SIZE];

	FD_ZERO(&initfds);
	for (Session &s : sessionListInThread) {
		FD_SET(s.connSock, &initfds);
	}

	while (1) {
		readfs = initfds;              /* Structure assignment */
		nEvents = select(0, &readfs, 0, 0, 0);
		if (nEvents < 0) {
			printf("%d: Error! Cannot poll sockets: %d\n", GetCurrentThreadId(), WSAGetLastError());
			break;
		}

		//Receive data from clients
		std::vector<Session>::iterator ptr = sessionListInThread.begin();
		while(ptr != sessionListInThread.end() && nEvents > 0) {
			SOCKET clientSock = ptr->connSock;

			if (FD_ISSET(clientSock, &readfs)) {
				--nEvents;
				//Receive request
				ret = recvStream(clientSock, buff);
				if (ret <= 0) {
					if (ret == 0)
						printf("Client disconnect\n");
					else
						printf("Error %d: Cannot recieve data.\n", WSAGetLastError());

					FD_CLR(clientSock, &initfds);
					closesocket(clientSock);
					ptr = sessionListInThread.erase(ptr);
					continue;
				}
				else if (ret > 0) {
					int result = handleRequests(*ptr, buff);
					sprintf_s(buff, BUFF_SIZE, "%d", result);

					//Send to client
					ret = sendStream(clientSock, buff);
					if (ret == SOCKET_ERROR) {
						printf("Error %d: Cannot send data.\n", WSAGetLastError());
						break;
					}
				}
			}
			ptr++;
		}

		//If all socket disconnect, end while
		if (sessionListInThread.empty())
			break;
	}//End while
    return 0;
}

