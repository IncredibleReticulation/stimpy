//ThreadSock header file
//Course: 4050-212-02
//Authors: Alex Buie, Luke Matarazzo, Jackson Sadowski, Steven Tucker
//Filename:ThreadSock.cpp

#include <vector>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <sstream>
#include <iostream>
#include <fstream>

using namespace std;

class ThreadSock{
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
		int split(vector<string>*, string, string);
		bool validateUser(string);
		void setSock(SOCKET);
};
