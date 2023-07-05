#include <string>
#include <algorithm>
#include <list>
#include <winsock2.h>


//Implementation of a session
struct Session {
	const SOCKET connSock;
	const char *clientIP;
	const int clientPort;
	const char *userName;
	int statusLogin;

	Session(const SOCKET connsock, const char *clientip, const int clientport)
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

//Critical section
struct CS_Acquire {
	CRITICAL_SECTION &cs;
	CS_Acquire(CRITICAL_SECTION &_cs) : cs( _cs) { EnterCriticalSection(&cs); }
	~CS_Acquire() { LeaveCriticalSection(&cs); }
};

//List of session for multithread
//Critical resource
struct SessionList {
	std::list<Session *> sessionList;
	CRITICAL_SECTION cs;

	SessionList() {InitializeCriticalSection(&cs);}

	/*
	Add a session into the session list
	Param[in] session: Pointer to a session
	*/
	void add(Session *session) {
		CS_Acquire acquire(cs);
		sessionList.push_back(session);
	}

	/*
	Remove a session from the session list
	Param[in] session: Pointer to a session
	 */
	void remove(Session *session) {
		CS_Acquire acquire(cs);
		sessionList.remove(session);
	}

	/*
	Function findSessionByUserName: Find a session in the session list by username
	Param[in] username: Account's username
	Return: 1 if session is found
	        0 if session isn't found
	 */
	int findSessionByUserName(const std::string username) {
		CS_Acquire acquire(cs);
		auto search = std::find_if(sessionList.begin(), sessionList.end(), [username](const Session *s)->bool{
			return username == s->userName;
		});
		if (search != sessionList.end()) return 1;
		else return 0;
	}
};