#define _WIN32_WINNT 0x501
#include <windows.h>
#include <cstdlib>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
// #include "serversocket.h"
// #include "clientsocket.h"
#include "ThreadSock.h"

using namespace std;

// our thread for recving commands
DWORD WINAPI receive_cmds(LPVOID lpParam)
{
    //printf("thread created\r\n");
    cout << "thread created\r\n";

    //set our socket to the socket passed in as a parameter
    SOCKET current_client = (SOCKET)lpParam;

    //cout << "get here!\n";

    //buffer to hold our recived data
    //char rcvbuf[100];
    string command = ""; //will hold the command the client sent
    //string data = ""; //will hold our reply
    //buffer to hold our sent data
    //char sendData[100];
    //for error checking
    //int res;

    // cout << "Client socket:"<< (int)current_client << endl;

    //Set and send the welcome message
    // strcpy(sendData, "Welcome to IP");
    // send(current_client, sendData, strlen(sendData), 0);
    //current_client->sendData("Welcome to das multi-threaded server :)"); //send initiation hello
    sendData(current_client, "Welcome to das multi-threaded server :)"); //send initiation hello


    //our recv loop
    while(true)
    {
        //clear the recieve buffer prior to receiving
        //  the message; put the code here
        //memset(rcvbuf, 0, sizeof(rcvbuf));

        //receive the message
        //res = recv(current_client,rcvbuf,sizeof(rcvbuf),0); //recv cmds
        //what did I receive
        // cout << "Received:" << rcvbuf << ":message" << "\n";
        // if(res == -1)
        // {
        //     break;
        // }

        //put the client command in a c++ string
        //command = rcvbuf;
        //cout << command << endl;

        //get command from client
        //current_client->recvData(command);
        recvData(current_client, command);

        cout << "Received:" << command << ":message" << "\n";

        /* Put your stuff here */
        if(command == "hello" || command == "Hello" || command == "HELLO")
        {
            // strcpy(sendData, "Hello little friend");
            // send(current_client, sendData, strlen(sendData), 0);
            cout << "inside if1\n";
            //current_client->sendData("Hello little friend");
            sendData(current_client, "Hello little friend");
        } else if(command == "message" || command == "Message" || command == "MESSAGE")
        {
            cout << "inside if2\n";
            //strcpy(sendData, "Send the message...");
            //send(current_client, sendData, strlen(sendData), 0);
            //current_client->sendData("Send the message..."); //tell the client it's okay to start sending the message
            sendData(current_client, "Send the message...");

            //reset the receivebuffer
            //memset(rcvbuf, 0, sizeof(rcvbuf));
            
            //recv(current_client,rcvbuf,sizeof(rcvbuf),0); //recv cmds
            //current_client->recvData(command); //get the first part of the message
            recvData(current_client, command);
            //command = rcvbuf; //get into a c++ string

            while(command != ".") //while the client is still sending the message, send it back to the client and get more
            {
                //strcpy(sendData, "Hello little friend");
                //send(current_client, rcvbuf, strlen(rcvbuf), 0);
                //current_client->sendData(command); //send the client what they sent us
                sendData(current_client, command);

                //reset the receivebuffer
                //memset(rcvbuf, 0, sizeof(rcvbuf));

                //recv(current_client,rcvbuf,sizeof(rcvbuf),0); //recv cmds
                //current_client->recvData(command); //get the command which is actually the message from the client
                recvData(current_client, command);
            }

            // strcpy(sendData, "Okay I got the message");
            // send(current_client, sendData, strlen(sendData), 0);
            //current_client->sendData("Okay I got the message"); //send to the client that we got the message okay
            sendData(current_client, "Okay I got the message"); //send to the client that we got the message okay
        } else if(command == "quit" || command == "Quit" || command == "QUIT")
        {
            cout << "inside if3\n";
            // strcpy(sendData, "quit");
            // send(current_client, sendData, strlen(sendData), 0);
            //current_client->sendData("quit"); //send quit because that's what they sent us
            sendData(current_client, "quit");
            break; //break from while loop because they entered quit
        } else
        {
            cout << "inside else\n";
            // strcpy(sendData, "\tError - unknown command...");
            // send(current_client, sendData, strlen(sendData), 0);
            //current_client->sendData("\tError - unknown command..."); //send the client an error message bc we could not recognize command
            sendData(current_client, "\tError - unknown command..."); //send the client an error message bc we could not recognize command
        }

        //cout << "after the if statements\n";
        
        
        /* Stop putting your stuff here */  


        //clear the send buffer; put the code here
        //memset(rcvbuf, 0, sizeof(rcvbuf));

    }
}  
 
int main()
{
    cout << "Starting up multi-threaded SMTP server\n";

    //our masterSocket(socket that listens for connections)
    SOCKET sock;
    // //ServerSocket sockServer; //socket on the server to listen for connections

    

    WSADATA wsaData;
    sockaddr_in server;

    //start winsock
    int ret = WSAStartup(0x101,&wsaData); //use highest version of winsock avalible

    if(ret != 0)
    {
        return 0;
    }

    //fill in winsock struct ...
    server.sin_family=AF_INET;
    server.sin_addr.s_addr=INADDR_ANY;
    server.sin_port=htons(31000); //listen on telnet port 31000

    //create our socket
    sock=socket(AF_INET,SOCK_STREAM,0);

    if(sock == INVALID_SOCKET)
    {
        return 0;
    }

    //bind our socket to a port(port 123)
    if(bind(sock,(sockaddr*)&server,sizeof(server)) !=0)
    {
        return 0;
    }

    //listen for a connection
    if(listen(sock,5) != 0)
    {
        return 0;
    }

    //for our thread
    DWORD thread;

    //socket that we sendrecv data on
    SOCKET client;

    sockaddr_in from;
    int fromlen = sizeof(from);

    //loop forever
    while(true)
    {
        //accept connections
        client = accept(sock,(struct sockaddr*)&from,&fromlen);
        //ServerSocket sockServer; //client socket to send and receive data on
        //sockServer.StartHosting(31000);
        //printf("Client connected\r\n");
        cout << "Client connected\r\n";
        //cout << "client before thread:" << (int) client << endl;
        //create our recv_cmds thread and pass client socket as a parameter
        CreateThread(NULL, 0,receive_cmds,(LPVOID)client, 0, &thread);
    }

    WSACleanup();

    //exit
    return 0;
}
