#include <winsock2.h>

//Implementation of a session

struct Session {
	SOCKET connSock;
	const char *clientIP;
	int clientPort;
	const char *userName;
	int statusLogin;

	Session(SOCKET connsock, const char *clientip, const int clientport)
		:connSock{ connsock }, clientIP{ _strdup(clientip) },
		clientPort{ clientport }, userName{ _strdup("") }, statusLogin{ 0 }
	{}

	/*
	Function login: When a client login, change session's username and status login
	Param[in] username: Account's username
	Return: None
	*/
	void login(const std::string username) {
		userName = _strdup(username.c_str());
		statusLogin = 1;
	}

	/*
	Function logout: When a client logout, change session's username and status login to default
	Return: None
	*/
	void logout() {
		userName = _strdup("");
		statusLogin = 0;
	}
};
