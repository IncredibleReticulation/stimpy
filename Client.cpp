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
    if(recMessage.substr(0,3) == "220")
    {
        sockClient.sendData("HELO 127.0.0.1");
    }

    sockClient.recvData(recMessage);
    if(recMessage.substr(0,3) == "250")
    {
        string username;
        cout << "Username: ";
        getline(cin, username);
        sendMessage = "VRFY " + username;
        sockClient.sendData(sendMessage);
    }
    sockClient.recvData(recMessage);

    if(recMessage == "550" || recMessage == "500")
    {
        cout << "Invalid user...\n";
        return 1;
    }
    else if(recMessage == "250")
    {
        cout << "Logon successful.\n";
    }

    //bool done = false; //boolean for if the client is done communicating with the server or not
    string menu = "1. Send Email\n2. Read Inbox\n3. Quit\n"; //our menu of options
    int option = 1;

    while(option != 3) //while they don't enter 3 for the quit option, keep prompting for selection
    {
        if(option > 0 && option < 4) //only print menu if they entered a valid option last time
        {
            cout << menu; //print menu
        }

        //prompt for an option and get the option
        cout << endl << "Enter option: ";
        cin >> option;

        switch(option)
        {
            case 1: //option 1, to send an email
                //code
                cout << "send email option not yet implemented...\n";
                break;
            case 2: //option 2, to read messages in the user's mailbox
                //code
                cout << "read messages option net yet implemented...\n";
                break;
            case 3: //option 5, to quit
                //code
                cout << "quit option. goodbye...\n\n";
                break;
            default:
                cerr << "You entered an invalid command...\n";
                break;
        }
    }


    //bool done = false; //boolean for if the client is done communicating with the server or not

    //if the server replies with welcome, we can start sharin filez
    // if(recMessage == "WELCOME")
    // {
    //     sendMessage = sockClient.getAndSendMessage("Server>"); //prompt user for input and send to server

    //     while(sendMessage != "QUIT") //while not done
    //     {
    //         vector<string> v; //vector to hold the split string
    //         sockClient.split(&v, sendMessage, " "); //split the sent message so we can compare its elements

    //         if(v[0] == "LIST")
    //         {
    //             sockClient.listFiles(); //list the files from the server
    //         } else if(v[0] == "SEND") //if the client used the send command, get the file
    //         {
    //             sockClient.getFile(); //gets the file from the server and writes it locally
    //         } else //if they didn't choose to send
    //         {
    //             sockClient.recvData(recMessage); //receive the command
    //             cout << recMessage << endl; //print reply from server
    //         }

    //         sendMessage = sockClient.getAndSendMessage("Server>"); //prompt user for input and send to server
    //     }
    // }

    //close the connection
    sockClient.closeConnection();

    return 0; //ends program
}
