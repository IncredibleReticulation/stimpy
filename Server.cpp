//Course: 4050-212-02
//Authors: Luke Matarazzo and Jackson Sadowski
//IDE/editor: Sublime Text 2
//Compilers: mingw32-g++
//Project 2
//File name: main.cpp   server main
//Purpose: 

#include <iostream>
#include <cstdlib>
#include "ServerSocket.h"

int main()
{
    //constant
    const char LID[] = "user"; //user identifier

    //variables
    int port = 8000; //port number to connect to//atoi(argv[1]); //port number to connect to
    string recMessage; //message we receive from client

    ServerSocket sockServer; //serversocket instance
    cout << "HOSTING...\n"; //print status message
    sockServer.StartHosting(port); //start connection using port number given

    //send request for login identifier
    sockServer.sendData("LOGIN");

    //get the return data and validate the login
    sockServer.recvData(recMessage);
    
    //if they entered the wrong login ID, send back unwelcome and close connection
    if(recMessage != LID)
    {
        sockServer.sendData("UNWELCOME\n"); //unwelcome message

        //close the connection
        sockServer.closeConnection();
    } else //if valid LID, commence operation file sharing
    {
        sockServer.sendData("WELCOME"); //send welcome message

        sockServer.recvData(recMessage); //receive the command

        while(recMessage != "QUIT")
        {
            //reply to command
            sockServer.reply(recMessage);

            sockServer.recvData(recMessage); //receive the command
        }
    }

    //close the connection
    sockServer.closeConnection();

    return 0; //ends program
}
