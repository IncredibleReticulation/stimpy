//Socket class file
//Course: 4050-212-02
//Authors: Alex Buie, Luke Matarazzo, Jackson Sadowski, Steven Tucker
//Filename: Socket.cpp
//Purpose: 

#include "Socket.h"

//Name: Socket()
//Parameters: NONE
//Purpose: default constructor for Socket which creates our socket, a backup of it and tests to make sure there was no error
Socket::Socket()
{
    if(WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) //if there is an error, print an error and exit the program
        cerr<<"Socket Initialization: Error with WSAStartup\n"; //error message

    //Create a socket
    mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (mySocket == INVALID_SOCKET) //if there is an error creating or connecting with the socket print an error and end program
        cerr << "Socket Initialization: Error creating socket\n"; //error message

    myBackup = mySocket; //backup our current socket for future use in case
}

//Name: ~Socket()
//Parameters: NONE
//Purpose: destructor for Socket which just performs windows socket cleanup stuff
Socket::~Socket()
{
    WSACleanup();
}

//Name: sendData()
//Parameters: string which is the message to be sent
//Purpose: takes a string from the user and sends it
//Returns: bool, true for success (which it will always be technically)
bool Socket::sendData(string s)
{
    const char *buffer = s.c_str();
    send(mySocket, buffer, strlen(buffer), 0);
    return true; //return true for success
}

//Name: sendData()
//Parameters: int to be sent
//Purpose: takes a int from the user, then calls send data to send as char array
//Returns: bool, true for success (which it will always be technically)
bool Socket::sendData(int i)
{
    stringstream s;
    s << i;
    return this -> sendData(s.str());
}

//Name: recvData()
//Parameters: string by reference so we can set it equal to the message we received
//Purpose: recieves data and sets our string equal to it
//Returns: int - if data == -1, then they're disconnected
int Socket::recvData(string &s)
{
    char buffer[1000];
    int i = recv(mySocket, buffer, 1000, 0);
    buffer[i] = '\0';
    s = buffer;
    return i; //return number of bytes returned by recv function, -1 if they disconnected
}

//Name: closeConnection()
//Parameters: NONE
//Purpose: closes the connection
//Returns: NONE
void Socket::closeConnection()
{
    closesocket(mySocket);
    mySocket = myBackup;
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