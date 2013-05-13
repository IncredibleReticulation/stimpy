//Socket class header file
//Course: 4050-212-02
//Authors: Alex Buie, Luke Matarazzo, Jackson Sadowski, Steven Tucker
//Filename: Socket.h

#ifndef SOCKET_H
#define SOCKET_H
#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "WinSock2.h"

using namespace std;

class Socket
{
    protected:
        WSADATA wsaData;
        SOCKET mySocket;
        SOCKET myBackup;
        SOCKET acceptSocket;
        sockaddr_in myAddress;
    
    public:

        Socket();   //default constructor
        ~Socket();  //default destructor

        //communication methods
        bool sendData(string);
		bool sendData(int);
        int recvData(string&);
        void closeConnection();
        int split(vector<string>*, string, string);
};

#endif
