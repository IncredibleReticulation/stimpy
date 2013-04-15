//Socket class file
//Course: 4050-212-02
//Authors: Luke Matarazzo and Jackson Sadowski
//Filename: Socket.cpp
//Purpose: 

#include "Socket.h"

//Name: listFiles()
//Parameters: NOT KNOWN YET
//Purpose: This function lists all of the files that are avalible for tansfer
Socket::Socket()
{
    if(WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) //if there is an error, print an error and exit the program
    {
        cerr<<"Socket Initialization: Error with WSAStartup\n"; //error message
        WSACleanup(); //windows clean up function
        exit(10); //ends program
    }

    //Create a socket
    mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (mySocket == INVALID_SOCKET) //if there is an error creating or connecting with the socket print an error and end program
    {
        cerr << "Socket Initialization: Error creating socket\n"; //error message
        WSACleanup(); //windows clean up function
        exit(11); //ends program
    }

    myBackup = mySocket; //backup our current socket for future use in case
}

//Name: listFiles()
//Parameters: NOT KNOWN YET
//Purpose: This function lists all of the files that are avalible for tansfer
Socket::~Socket()
{
    WSACleanup(); //windows clean up function
}

//Name: listFiles()
//Parameters: NOT KNOWN YET
//Purpose: This function lists all of the files that are avalible for tansfer
//Returns: NOT KNOWN YET
bool Socket::sendData(string s)
{
    const char *buffer = s.c_str();
    send(mySocket, buffer, strlen(buffer), 0);
    return true; //return true for success
}

//Name: listFiles()
//Parameters: NOT KNOWN YET
//Purpose: This function lists all of the files that are avalible for tansfer
//Returns: NOT KNOWN YET
bool Socket::recvData(string &s)
{
    char buffer[256];
    int i = recv(mySocket, buffer, 256, 0);
    buffer[i] = '\0';
    s = buffer;
    return true; //return true for success
}

//Name: listFiles()
//Parameters: NOT KNOWN YET
//Purpose: This function lists all of the files that are avalible for tansfer
//Returns: NOT KNOWN YET
void Socket::closeConnection()
{
    closesocket(mySocket); //close the connection
    mySocket = myBackup; //set the socket back to the backup we set earlier
}

//Name: split()
//Parameters: Pointer to a vector of strings, a string that needs to be split, a delimeter to split the string with.
//Purpose: Split string according to given delimeter and add those elements to a vector.
//Returns: Number of elements added to the vector as an int.
int Socket::split(vector<string>* v, string s, string del)
{
    //if the delimeter does not appear once, return 0 because string will not be split
    if(s.find(del) == -1)
    {
        v->push_back(s); //put the string in the vector so the user can still use the value they passed in
        return 0;
    }
        
    int delims = 0; //will hold number of delimeters found in the string that needs to be split

    while(s.find(del) != -1) //while there is still a delimeter present, continue to break the string apart
    {
        v->push_back(s.substr(0, s.find(del)));     //add part of string up to the delimeter to the vector
        s.erase(0, s.find(del) + 1);                //delete added part
        delims++; //increment
    }

    v->push_back(s.substr(0));  //add the rest of the string after the final delimeter

    return delims + 1;          //return number of elements added to the vector
}