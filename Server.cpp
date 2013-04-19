//Course: 4050-212-02
//Authors: Luke Matarazzo and Jackson Sadowski
//IDE/editor: Sublime Text 2
//Compilers: mingw32-g++
//Project 2
//File name: main.cpp   server main
//Purpose: 

#include <iostream>
#include <cstdlib>
#include <fstream>
#include "ServerSocket.h"

int main()
{
    ServerSocket sockServer; //serversocket instance
    ifstream fin;
    
    string temp;
    fin.open("users.txt");
    

    while(!fin.eof())
    {
        getline(fin, temp);
        sockServer.user.pushback(temp);//pushing the usernames into the vector
    }

    //variables
    int port = 8000; //port number to connect to//atoi(argv[1]); //port number to connect to
    string recMessage; //message we receive from client

    
    cout << "HOSTING...\n"; //print status message
    sockServer.StartHosting(port); //start connection using port number given

    //send request for login identifier
    sockServer.sendData("LOGIN");

    //get the return data and validate the login
    sockServer.recvData(recMessage);
    
    //if they entered the wrong login ID, send back unwelcome and close connection
    if(!isAUser(recMessage, user))
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

//Function that will decide whether or not the user is allowed to log in.
bool isAUser(string s, vector v)
{
    for (int i = 0; i < v.size(); i++)
    {
        if (s == v[i])
            return true;
    }

    return false;

}
