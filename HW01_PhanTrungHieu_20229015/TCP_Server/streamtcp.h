#include <winsock2.h>
#include <stdint.h>
#define BUFF_SIZE 1024

/*
Function sendStream: Send message in stream
Param[in] socket: A descriptor identifying a connected socket.
Param[in] buffer: A pointer to a buffer containing the data to be transmitted.
Return: If no error occurs, return the number of bytes message sent.
        Otherwise, send a value of SOCKET_ERROR
*/
int sendStream(SOCKET socket, char *buffer) {
	unsigned char bytes[4]; //The length of message
	int ret;
	uint32_t messLen, buffLen;
	uint32_t byteSend = 0; //The number of bytes message sent

	buffLen = strlen(buffer);
	messLen = htonl(buffLen);
	memcpy_s(bytes, 4 * sizeof(unsigned char), &messLen, 4);

	//Send the length of the message
	ret = send(socket, (char *)bytes, 4, 0);
	if (ret == SOCKET_ERROR)
		return ret;
	byteSend += ret;

	//Send the entire message
	while (buffLen) {
		ret = send(socket, buffer, buffLen, 0);
		if (ret == SOCKET_ERROR)
			return ret;

		//Left bytes to send
		buffLen -= ret;
		buffer += ret;
		byteSend += ret;
	}
	return byteSend;
}

/*
Function recvStream: Receive message in stream
Param[in] socket: The descriptor that identifies a connected socket.
Param[out] buffer: A pointer to the buffer to receive the incoming data.
Return: If no error occurs, recvStream returns the number of bytes message received.
        If the connection has been gracefully closed, the return value is zero.
        Otherwise, a value of SOCKET_ERROR is returned
*/
int recvStream(SOCKET socket, char* buffer) {
	sprintf_s(buffer, BUFF_SIZE, "");
	unsigned char bytes[4]; //The length of message
	char partMess[BUFF_SIZE]; //Recieve part of message

	int ret;
	uint32_t buffLen, messLen;
	uint32_t byteRecv = 0; //The number of bytes message received

						   //Receive the length of message
	ret = recv(socket, (char *)bytes, 4, MSG_WAITALL);
	if (ret < 1) //= 0 if disconnect or = SOCKET_ERROR
		return ret;
	byteRecv += ret;
	memcpy_s(&messLen, sizeof(uint32_t), bytes, 4);
	buffLen = ntohl(messLen);

	//Recive the entire message
	while (buffLen) {
		ret = recv(socket, partMess, buffLen, 0);
		if (ret < 1)
			return ret;
		partMess[ret] = 0;
		strcat_s(buffer, BUFF_SIZE, partMess);

		//Left bytes to receive
		buffLen -= ret;
		byteRecv += ret;
	}
	return byteRecv;
}