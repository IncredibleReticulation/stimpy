#define _WIN32_WINNT 0x501
#include <cstdlib>
#include <cctype>
#include <stdlib.h>
#include "ThreadSock.h"
#include "Status.h"

using namespace std;

//prototype for the trim function
string trim(string);

// our thread for recving commands
DWORD WINAPI handleMail(LPVOID lpParam)
{
    cout << "Thread Created\n";

    //set our socket to the socket passed in as a parameter
    ThreadSock current_client;

    current_client.setSock((SOCKET)lpParam);

    string recMessage = ""; //will hold the command the client sent

    //Set and send the welcome message
    current_client.sendResponse(Status::SMTP_SRV_RDY, "stimpy 0.0.1/flopcity - Welcome"); //send initiation hello

    current_client.recvData(recMessage); //get data from the client

    //checking out the string to see if it's helo
	bool bHeloSent = FALSE;
	do{
		if (recMessage.substr(0,4) == "HELO") //if the first word is helo
		{
			current_client.sendData(Status::SMTP_ACTION_COMPLETE); //send back 250 that it's good
			cout << "Connection Successful. We received a HELO from the client.\n";
			bHeloSent = TRUE;

		}    
		else //if it's not HELO, return error code
		{
			current_client.sendData(Status::SMTP_CMD_SNTX_ERR); //sending the error code
			cout << "Connection Failed. We did not recieive a HELO from client...\n";
			current_client.recvData(recMessage); //get data from the client
		}
	} while (!bHeloSent);

    current_client.recvData(recMessage); //recieving the verify and a username

    //checking to see if it's a verify
    if (recMessage.substr(0,4) == "VRFY")
    {
        string username = trim(recMessage.substr(5)); //trim the username

        //if it is, validate the username and continue
        if (current_client.validateUser(username))
        {
            current_client.sendData(Status::SMTP_ACTION_COMPLETE); //if the username was valid, send back 250
        }

        //sending back a bad error code
        if (!current_client.validateUser(username))
        {
            current_client.sendData(Status::SMTP_MBOX_UNAV);
        }
    }

    //getting data from the client
    current_client.recvData(recMessage);

    //our recv loop
    while(recMessage != "QUIT" || recMessage != "Quit" || recMessage != "quit")
    {
		bool bRecipientSent = FALSE;
		string sRecipient = "";

        if(recMessage.substr(0,9) == "MAIL FROM")
        {
            cout << "client sent: " << recMessage << endl; //for debugging
            current_client.sendData(Status::SMTP_ACTION_COMPLETE);
        }
        else
        {
            current_client.sendData(Status::SMTP_CMD_SNTX_ERR);
        }

        //get more data from client
        current_client.recvData(recMessage);

		if (recMessage.substr(0,7) == "RCPT TO") //checking to see if it's RCPT TO
		{
            cout << "client sent: " << recMessage << endl; //for debugging
			//checking to see if the user is valid
			if (current_client.validateUser(recMessage.substr(9, recMessage.find("@")-9)))
			{
				current_client.sendResponse(Status::SMTP_ACTION_COMPLETE, "ok");//if the username was valid, send back 250
				bRecipientSent = TRUE;
				sRecipient = recMessage.substr(9);
			}

			//sending back a bad error code
			else
			{
				current_client.sendResponse(Status::SMTP_CMD_SNTX_ERR, "malformed recipient");
			}

			current_client.recvData(recMessage);//getting the rcptto from the client
		}


        //checking to see if the string is DATA
        if (recMessage.substr(0,6) == "DATA")
        {
			stringstream message;
            //if not, return an error code
            if (!bRecipientSent)
            {
                current_client.sendResponse(Status::SMTP_CMD_SNTX_ERR,"you must specify a recipient first");//sending and error code back
            }
			else
			{
				current_client.sendResponse(Status::SMTP_ACTION_COMPLETE,"ok -- send data");
				do
				{
					current_client.recvData(recMessage);
					if(recMessage != ".")
						message << recMessage << endl;
				} while (recMessage != ".");
			}
			

        }

        //create file output object and open it in append mode
        ofstream fout;
        fout.open ("fout.txt", ios::app);
        
        current_client.recvData(recMessage); //getting a line from the user

        while (recMessage != ".") //while line !=. we want to keep getting input from the user
        {
            fout << recMessage; //write line to file

            current_client.recvData(recMessage); //getting next line from the user
        }

        //send status code that action is complete and close the file
        current_client.sendData(Status::SMTP_ACTION_COMPLETE);
        fout.close();

        //get data from the client before starting loop again
        current_client.recvData(recMessage);

        if(recMessage == "QUIT") //if they sent quit, break from while loop and the thread will end after exiting this
            break;
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
        cout << "Client connected.\n";
        //create our recv_cmds thread and pass client socket as a parameter
        CreateThread(NULL, 0,handleMail,(LPVOID)client, 0, &thread);
    }

    WSACleanup(); //windows cleanup

    return 0; //ends program
}

string trim(string s)
{   
    while(isspace(s[0])) //if the first thing is a space, erase it until it is longer a space.
    {
        s.erase(0, 1); //remove the first index because it is a space
    }

    while(isspace(s[s.length()-1])) //if the last char of the string is a space, remove it until it is no longer a space
    {
        s.erase(s.length()-1, 1); //remove that char because it is a space
    }

    return s; //return the final string
}
