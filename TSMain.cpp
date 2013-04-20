#define _WIN32_WINNT 0x501
#include <windows.h>
#include <cstdlib>
//#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <fstream>
#include "ThreadSock.h"
#include "Status.h"

using namespace std;
using namespace Status;

// our thread for recving commands
DWORD WINAPI handleMail(LPVOID lpParam)
{
    cout << "thread created\r\n";

    //set our socket to the socket passed in as a parameter
    ThreadSock current_client;

    current_client.setSock((SOCKET)lpParam);

    string command = ""; //will hold the command the client sent

    //Set and send the welcome message
    current_client.sendData("Welcome to das multi-threaded server :)"); //send initiation hello

    //our recv loop
    while(true)
    {
        //variables
        string helostring, verify, toaddress, line;
        ofstream fout;
        vector<string> recipients;

        //checking out the string to see if it's helo
        if (helostring.substr(0,4) == "HELO") //if the first word is helo
        {
            current_client.sendData(SMTP_ACTION_COMPLETE); //send back 250 that it's good

            //if it's not HELO, return error code
            if (helostring.substr(0,4) != "HELO")
            {
                current_client.sendData(Status::SMTP_CMD_SNTX_ERR); //sending the error code
            }

        }

        current_client.recvData(verify); //recieving the verify and a username

        //checking to see if it's a verify
        if (verify.substr(0,4) == "VRFY")
        {
            //if it is, validate the username and continue
            if (current_client.validateUser(verify.substr(5)))
            {
                current_client.sendData(Status::SMTP_ACTION_COMPLETE);//if the username was valid, send back 250
            }

            //sending back a bad error code
            if (!current_client.validateUser(verify.substr(5)))
            {
                current_client.sendData(Status::SMTP_CMD_SNTX_ERR);
            }

        }

        //at this point, we are going to check for multiple recipt to
        //Looping over the next function
        //It keeps looping until it is not a recipt to, then breaks out

        //getting the rcptto from the client
        current_client.recvData(toaddress);

        do //going to loop to add people to the vector
        {
            //checking to see if it's RCPT TO
            if (toaddress.substr(0,6) == "RCPT TO")
            {
                //checking to see if the user is valid
                if (current_client.validateUser(verify.substr(9)))
                {
                    current_client.sendData(Status::SMTP_ACTION_COMPLETE);//if the username was valid, send back 250
                }

                //sending back a bad error code
                else if (!current_client.validateUser(verify.substr(9)))
                {
                    current_client.sendData(Status::SMTP_CMD_SNTX_ERR);
                }

                recipients.push_back(verify.substr(9, verify.length()-10));//putting the usernames into the vector

                current_client.recvData(toaddress);//getting the rcptto from the client
            }

        } while (toaddress.substr(0,6) == "RCPT TO"); //it's going to keep getting users and break when it's not RCPT TO

        //doing this for readablility -- we can change this later
        string data;
        data = toaddress;

        //checking to see if the string is DATA 
        if (data.substr(0,6) == "DATA")
        {
            //if not, return an error code
            if (data.substr(0,6) != "DATA")
            {
                current_client.sendData(Status::SMTP_CMD_SNTX_ERR);//sending and error code back
            }

        }

        fout.open ("fout.txt", ios::app);//opening the file
        
        //while line !=. we want to keep getting input from the user
        while (true)
        {
            current_client.recvData(line);//getting a line from the user

            //checking to see if the line should be added
            if (line != ".")
            {
                fout << line << endl;
            }
            
            //if they send a period, then we want to send back status number and quit
            else
            {
                current_client.sendData(Status::SMTP_ACTION_COMPLETE);//sending the status code back
                break;
            }

        }

        fout.close();//closing the file

        // //get command from client
        // recvData(current_client, command);

        // cout << "Received:" << command << ":message" << "\n";

        // /* Put your stuff here */
        // if(command == "hello" || command == "Hello" || command == "HELLO")
        // {
        //     current_client.sendData("Hello little friend"); //send initial message
        // } else if(command == "message" || command == "Message" || command == "MESSAGE")
        // {
        //     current_client.sendData("Send the message..."); //tell the client it's okay to start sending the message

        //     recvData(current_client, command);

        //     while(command != ".") //while the client is still sending the message, send it back to the client and get more
        //     {
        //         current_client.sendData(command); //send the client what they sent us

        //         recvData(current_client, command); //get the command which is actually the message from the client
        //     }

        //     current_client.sendData("Okay I got the message"); //send to the client that we got the message okay
        // } else if(command == "quit" || command == "Quit" || command == "QUIT")
        // {
        //     current_client.sendData("quit"); //send quit because that's what they sent us
        //     break; //break from while loop because they entered quit
        // } else
        // {
        //     current_client.sendData("\tError - unknown command..."); //send the client an error message bc we could not recognize command
        // }

    }
}  
 
int main()
{
    cout << "Starting up multi-threaded SMTP server\n";

    //our masterSocket(socket that listens for connections)
    SOCKET sock;
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
        cout << "Client connected\r\n";
        //create our recv_cmds thread and pass client socket as a parameter
        CreateThread(NULL, 0,handleMail,(LPVOID)client, 0, &thread);
    }

    WSACleanup(); //windows cleanup

    return 0; //ends program
}
