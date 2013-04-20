//Course: 4050-212-02
//Authors: Luke Matarazzo and Jackson Sadowski
//IDE/editor: Sublime Text 2
//Compilers: mingw32-g++
//Project 2
//File name: main.cpp   client main
//Purpose: 

#include <iostream>
#include <cstdlib>
#include "ClientSocket.h"

int main(int argc, char * argv[])
{
    if(argc != 3)
    {
        cout << "USAGE: " << argv[0] << " servername portnum (usually 25)" << endl;
        return 1;
    }

    int port = atoi(argv[2]); //port number to connect to
    string ipAddress = string(argv[1]); //ip address to connect to
    string recMessage; //message it receives
    string sendMessage; //message it sends

    //ask for an ip address for the client to connect to
    cout << "Connecting to: " << ipAddress << ":" << port << endl;

    ClientSocket sockClient; //clientsocket object instance
    sockClient.connectToServer(ipAddress.c_str(), port); //connect to the server using the ip and port given

    //receive the login prompt and enter stuff
    //sockClient.recvData(recMessage);

    //get user login info and send to server
    //cin.ignore(1000, '\n'); //ignore needed bc of previous cin
    //sockClient.getAndSendMessage(recMessage + ": "); //get login from user and send to server

    //receive message
    sockClient.recvData(recMessage);
    cout << recMessage << endl;
    bool done = false; //boolean for if the client is done communicating with the server or not

    //if the server replies with welcome, we can start sharin filez
    if(recMessage == "WELCOME")
    {
        sendMessage = sockClient.getAndSendMessage("Server>"); //prompt user for input and send to server

        while(sendMessage != "QUIT") //while not done
        {
            vector<string> v; //vector to hold the split string
            sockClient.split(&v, sendMessage, " "); //split the sent message so we can compare its elements

            if(v[0] == "LIST")
            {
                sockClient.listFiles(); //list the files from the server
            } else if(v[0] == "SEND") //if the client used the send command, get the file
            {
                sockClient.getFile(); //gets the file from the server and writes it locally
            } else //if they didn't choose to send
            {
                sockClient.recvData(recMessage); //receive the command
                cout << recMessage << endl; //print reply from server
            }

            sendMessage = sockClient.getAndSendMessage("Server>"); //prompt user for input and send to server
        }
    }

    //close the connection
    sockClient.closeConnection();

    return 0; //ends program
}
