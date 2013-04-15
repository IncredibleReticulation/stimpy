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
        WSADATA wsaData;        //for windows cleanup stuff
        SOCKET mySocket;        //socket to connect to
        SOCKET myBackup;        //backup of the socket
        SOCKET acceptSocket;    //socket that we connect to
        sockaddr_in myAddress;  //client socket
    public:
        //constructor/destructor
        Socket();   //default
        ~Socket();  //default

        //communication methods
        bool sendData(string);
        bool recvData(string&);
        void closeConnection();
        int split(vector<string>*, string, string);
};

#endif
