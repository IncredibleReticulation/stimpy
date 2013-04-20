//sockets header file which will hold functions that need to be used for communication between sockets
#include <vector>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

using namespace std;

class ThreadSock{
	private:
		SOCKET socket;
		vector<string> users; //holds all of the local usernames
		
	public:
		bool sendData(string);
		bool sendData(int);
		bool recvData(string&);
		int split(vector<string>*, string, string);
		bool validateUser(string);
		void setSock(SOCKET);
};
