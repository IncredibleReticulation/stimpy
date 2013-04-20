//sockets header file which will hold functions that need to be used for communication between sockets
#include <vector>
#include <string>
#include <winsock2.h>
//#include "WinSock2.h"
#include <ws2tcpip.h>
#include <windows.h>

using namespace std;

class ThreadSock : public SOCKET {
	public:
		bool sendData(SOCKET, string);
		bool recvData(SOCKET, string&);
		int split(vector<string>*, string, string);
		bool validateUser(string);

};
