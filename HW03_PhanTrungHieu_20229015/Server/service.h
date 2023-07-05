#include <string>
#include <map>
#include <fstream>
#include "session.h"
#include "prefix.h"

//List of account 
std::map<const std::string, const int> accountList;

/*
Function handleRequests: Handle requests from client
Param[in] session: The session is being process
Param[in] buff: A pointer to the buffer of request
Return: response prefix
*/
int handleRequests(Session &session, const char *buff);

/*
Function handleUSER: Handle login request
Param[in] session
Param[in] user
Return: Login's response prefix
*/
int handleUSER(Session &session, std::string user);

/*
Function handlePOST: Handle post messages
Param[in] session
Param[in] message
Return: Post's response prefix
*/
int handlePOST(Session &session, std::string message);

/*
Function handleBYE: Handle logout request
Param[in] session
Return: Logout's response prefix
*/
int handleBYE(Session &session);

/*
Function splitString: Split string str into 2 string s1, s2 by delimiter " "
Param[in] str
Param[out] s1
Param[out] s2
*/
void splitString(std::string str, std::string &s1, std::string &s2) {
	size_t pos = str.find(" ");
	if (pos != std::string::npos) {
		s1 = str.substr(0, pos);
		s2 = str.substr(pos + 1);
	}
	else s1 = str;
}

/*
Function getAccount: Get account list from file
Param[in] namefile: Name of file account
*/
void getAccount(std::string namefile) {
	//Open file
	std::ifstream inf(namefile);
	if (!inf) {
		std::cout << "Can not open " << namefile << '\n';
		exit(1);
	}
	std::string input, username, status;
	while (inf) {
		getline(inf, input);
		splitString(input, username, status);
		accountList.insert(make_pair(username, stoi(status)));
	}
}

int handleRequests(Session &session, const char *buff) {
	std::string header;
	std::string data;
	std::string request = buff;

	splitString(request, header, data);

	if (header == "USER")
		return handleUSER(session, data);
	else if (header == "POST")
		return handlePOST(session, data);
	else if (header == "BYE")
		return handleBYE(session);
	else
		return prefix::RequestUndefined;
}



int handleUSER(Session &session, std::string username) {
	//Client already login
	if (session.statusLogin)
		return prefix::AlreadyLogin;

	auto search = accountList.find(username);

	//Can't find account
	if (search == accountList.end())
		return prefix::AccountNotFound;

	//Account is locked
	else if (search->second == 1)
		return prefix::LockedAccount;

    //Login successful
	else {
		session.login(username);
		return prefix::Login;
	}
}

int handlePOST(Session &session, std::string message) {
	//Post message successful
	if (session.statusLogin) {
		return prefix::MessageSuccessful;
	}

	//Not login
	else return prefix::NotLogin;
}

int handleBYE(Session &session) {
	//Logout successful
	if (session.statusLogin) {
		session.logout();
		return prefix::Logout;
	}

	//Not login
	else return prefix::NotLogin;
}