#include <string>

//Prefix and message response with prefix

//Prefix code
enum prefix :int {
	Login = 10,						//Login Successful
	LockedAccount = 11,				//Account is locked
	AccountNotFound = 12,			//Can't find account
	AlreadyLogin = 13,				//Already login

	MessageSuccessful = 20,			//Post message successful
	NotLogin = 21,					//Not login

	Logout = 30,					//Logout successful

	RequestUndefined = 99			//Request undefined
};

/*
Function messageResponse: Message responese with prefix to client
Param[in] buff: A pointer to the buffer of prefix receive
Return : A string message
*/
std::string messageResponse(char *buff) {
	std::string s = buff;
	int p = stoi(s);
	switch (p) {
	case 10: return "Login successful";
	case 11: return "Account is locked";
	case 12: return "Can't find account";
	case 13: return "Already login";

	case 20: return "Post message successful";
	case 21: return "Must login";

	case 30: return "Logout successful";

	case 99: return "Request undefined";

	default: return std::string();
	}
}

