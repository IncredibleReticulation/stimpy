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

string ClientSocket::encrypt(string message)
{
    int aVal; //will hold ascii value of each character
    int key = 5; //key value, in sloppy way

    for(int i = 0; i < message.length(); i++)
    {
        if(message[i] != '\n') //don't encrypt a newline
        {
            aVal = message[i] - 32 + key; //get the ascii value of character, subtract 32 and add the key

            if(aVal > 95) //check if adding the key makes it greater than 95
                aVal -= 95; //if so, subtract 95

            aVal += 32; //add 32 back to it
            message[i] = aVal; //set the char equal to the new, encrypted one
        }   
    }

    return message; //return the new message which is now encrypted
}

string ClientSocket::decrypt(string message)
{
    int value = 0; //holds the int value of the char
    int key = 5; //key value, in sloppy way

    for (int i=0; i<message.length(); i++)
    {
        if(message[i] != '\n') //don't decrypt a newline
        {
            value = (message[i] - 32) - key; //get ascii value, then subtract 32 and the key

            if (value < 0) // if the value is negative
                value += 95; //add 95 to the int

            value += 32; //add 32 to the int regardless of the value
            message[i] = value; //assigning c the character associated with the int of value    
        }
    }

    return message; //returning the string
}