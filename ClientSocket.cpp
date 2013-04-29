//ClientSocket class file
//Course: 4050-212-02
//Authors: Alex Buie, Luke Matarazzo, Jackson Sadowski, Steven Tucker
//Filename: ClientSocket.cpp
//Purpose: 

#include <fstream>
#include "ClientSocket.h"

//Name: listFiles()
//Parameters: NOT KNOWN YET
//Purpose: This function lists all of the files that are avalible for tansfer
//Returns: NOT KNOWN YET
void ClientSocket::connectToServer(const char *ipAddress, int port)
{
    //populate values in the struct
    myAddress.sin_family = AF_INET;
    myAddress.sin_addr.s_addr = inet_addr(ipAddress);
    myAddress.sin_port = htons(port);

    if (connect(mySocket,(SOCKADDR*)&myAddress, sizeof(myAddress)) == SOCKET_ERROR) //if there is an error connecting, print error and end program
    {
        cerr<<"ClientSocket: Failed to connect\n"; //error message
        WSACleanup(); //windows cleanup function
        exit(13); //ends program
    }
}

bool ClientSocket::checkError(string message, int error)
{
    if(atoi(message.substr(0,3).c_str()) != error)
    {
        cerr << "\nSomething went wrong. Please try again.\n\n"; //error message
        return false;
    }

    return true;
}