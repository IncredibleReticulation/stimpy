//Socket class header file
//Course: 4050-212-02
//Authors: Luke Matarazzo and Jackson Sadowski
//Filename: Socket.h

#ifndef SOCKET_H
#define SOCKET_H
#pragma once
#include <iostream>
#include <string>
#include <vector>
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
        //constructor/destructor
        Socket();   //default
        ~Socket();  //default

        //communication methods
        bool sendData(string);
		bool sendData(int);
        bool recvData(string&);
        void closeConnection();
        string getAndSendMessage(const string&);
        int split(vector<string>*, string, string);
};

#endif
