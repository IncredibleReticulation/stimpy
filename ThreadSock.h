//ThreadSock header file
//Course: 4050-212-02
//Authors: Alex Buie, Luke Matarazzo, Jackson Sadowski, Steven Tucker
//Filename: ThreadSock.h

#include <vector>
#include <string>
#include <winsock2.h>
#include <sstream>
#include <iostream>
#include <fstream>
using namespace std;

class ThreadSock
{
	private:
		SOCKET socket;
		vector<string> users; //holds all of the local usernames
		
	public:
		ThreadSock();
		~ThreadSock();
		bool sendData(string);
		bool sendData(int);
		bool sendResponse(int,string);
		int recvData(string&);
		bool validateUser(string);
		void setSock(SOCKET);
		string getDateTime();
		void closeConnection();
};
