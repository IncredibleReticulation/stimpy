//Course: 4050-212-02
//Authors: Alex Buie, Luke Matarazzo, Jackson Sadowski, Steven Tucker
//IDE/editor: Sublime Text 2
//Compilers: mingw32-g++
//Final SMTP Project
//Filename: TSMain.cpp   server main
//Purpose:

#define _WIN32_WINNT 0x501
#include <cstdlib>
#include <cctype>
#include "ThreadSock.h"
#include "Status.h"

using namespace std;

string sSrvrIP; //global variable which holds the IP address of the server

//our thread for FIFO reading and message relaying
DWORD WINAPI relayMail(LPVOID lpParam)
{
    cout << "FIFO Thread Created\n";

    /*******************************************************************************************************************************
    *                                                                                                                                   *
    *   I'm pretty sure we'll need to have a sockclient instance here which means we'll need to include clientsocket.h...kinda nasty    *
    *                                                                                                                                   *
    ********************************************************************************************************************************/

    //create a threadsock object and set our socket to the socket passed in as a parameter
    ThreadSock fifoClient;
    fifoClient.setSock((SOCKET)lpParam);

    string recMessage = ""; //will hold the command the client sent
    string sendMessage = ""; //will hold the reply we send

    //file input object to read stuff in from the message fifo queue
    ifstream fin("email.fifo");
}

//our thread for client connections
DWORD WINAPI handleMail(LPVOID lpParam)
{
    cout << "Email Thread Created\n";

    //set our socket to the socket passed in as a parameter
    ThreadSock current_client;
    current_client.setSock((SOCKET)lpParam);

    string recMessage = ""; //will hold the command the client sent
    string sendMessage = ""; //will hold the reply we send

    //Set and send the welcome message
    current_client.sendResponse(Status::SMTP_SRV_RDY, "Welcome to the SMTP Server!"); //send initiation hello

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
    string username;

    //checking to see if it's a verify
    if (recMessage.substr(0,4) == "VRFY")
    {
        username = recMessage.substr(5); //trim the username

        //if it is, validate the username and continue
        if (current_client.validateUser(username))
        {
            current_client.sendData(Status::SMTP_ACTION_COMPLETE); //if the username was valid, send back 250
        }
        else
        {
            current_client.sendData(Status::SMTP_MBOX_UNAV);
        }
    }

    //getting data from the client
    int clientFlop = 0; //will hold the value that the recv function returns
    clientFlop = current_client.recvData(recMessage);

    //our send/recv loop
    while(recMessage != "QUIT" || recMessage != "Quit" || recMessage != "quit")
    {
		bool bRecipientSent = FALSE;
		string sRecipient = "";

        if(recMessage.substr(0,9) == "MAIL FROM")
        {
            cout << "Client Sent: " << recMessage << endl; //for debugging
            current_client.sendData(Status::SMTP_ACTION_COMPLETE);

            //get more data from client
            clientFlop = current_client.recvData(recMessage);

            //rcpt to section
            if (recMessage.substr(0,7) != "RCPT TO") //checking to see if it's RCPT TO
            {
                cout << "RCPT TO wasn't sent...\n"; //should probably sent syntax error back
                current_client.sendData(Status::SMTP_CMD_SNTX_ERR); //send error
            }
            else
            {
                cout << "Client Send: " << recMessage << endl; //for debugging
                //sRecipient = recMessage.substr(9, recMessage.find("@")-9);
                sRecipient = recMessage.substr(9, recMessage.length()-10);
                string sSrvrT = recMessage.substr(recMessage.find("@")+1);
                string sSrvr = sSrvrT.substr(0, sSrvrT.length()-1);
                cout << "User server: " << sSrvr << endl;
                //cout << "recipient username: " << recMessage.substr(9, recMessage.find("@")-9) << endl; //for debugging
                //checking to see if the user is valid
                bool bLocalDelivery = FALSE;

                if ((sSrvr == "127.0.0.1" || sSrvr == sSrvrIP || sSrvr == ""))
                {
                    bLocalDelivery = TRUE;
                    
                }
                if(bLocalDelivery && !current_client.validateUser(sRecipient.substr(0,sRecipient.find("@"))))
                        current_client.sendResponse(Status::SMTP_CMD_SNTX_ERR, "Malformed Recipient"); //sending back a bad error code
                else
                {
                    current_client.sendResponse(Status::SMTP_ACTION_COMPLETE, "OK");//if the username was valid, send back 250
                    bRecipientSent = TRUE;

                    //getting data and writing to file part
                    //cout << "before recvdata: " << recMessage << endl;
                    clientFlop = current_client.recvData(recMessage); //getting more data from client
                    //cout << "after recvdata " << recMessage << endl;

                    //checking to see if the string is DATA
                    if (recMessage.substr(0,6) != "DATA")
                    {
                        cout << "DATA wasn't received\n";
                        current_client.sendData(Status::SMTP_CMD_SNTX_ERR); //send error
                    }
                    else
                    {
                        //get the data of the message part
                        //create file output object and open it in append mode
                        ofstream fout;
                        if(bLocalDelivery)
                            fout.open ((string(sRecipient.substr(0,sRecipient.find("@")) + ".txt")).c_str(), ios::app);
                        else
                            fout.open("email.fifo", ios::app);

                        //write the initial part of the email
                        //fout << "\"" << current_client.getDateTime() << "\",\"" << sRecipient << "\",\"" << username << "\",\"";
                        fout << current_client.getDateTime() << endl << sRecipient << endl << username << endl;

                        //tell client to send data, then get data and write to file
                        current_client.sendResponse(Status::SMTP_BEGIN_MSG,"OK -- Send Data");
                        clientFlop = current_client.recvData(recMessage); //getting a line from the client

                        while (recMessage != ".") //while line !=. we want to keep getting message data from the client
                        {
                            if(clientFlop == -1)
                                break;

                            fout << recMessage; //write line to file

                            if(recMessage != "\n")
                                fout << endl;

                            cout << "Message: " << recMessage << endl;

                            clientFlop = current_client.recvData(recMessage); //getting next line from the user
                        }

                        //write the final quotation and add a newline to the end of it
                        //fout << "\"" << endl;
                        //char del = 236;
                        fout << "." << endl;

                        //send status code that action is complete and close the file
                        current_client.sendData(Status::SMTP_ACTION_COMPLETE);
                        fout.close();
                    }
                }
            }
        }

        else if(recMessage.substr(0,5) == "INBOX") //if they send an inbox and want to check their inbox
        {
            cout << "The Client Sent: " << recMessage << endl;

            //open file for the user's mailbox that is logged in
            ifstream fin(string(username + ".txt").c_str()); //file input object

            if(!fin.is_open()) //check if the file opens or not
            {
                current_client.sendResponse(Status::SMTP_MBOX_UNAV, "No messages in your inbox."); //send back that there aren't any messages
            } else
            {
                current_client.sendResponse(Status::SMTP_ACTION_COMPLETE, "OK"); //send 250 OK so they know we got the command okay then send mail

                getline(fin, sendMessage); //get first line from file

                while(!fin.eof()) //until we read in a single period from the file
                {
                    clientFlop = current_client.recvData(recMessage); //get the OK from the client

                    if(clientFlop == -1) //check to see if they actually sent a message and break if they didn't
                        break;

                    if(sendMessage == "") //if it doesn't have anything, then make it a newline because getline skips over it
                        sendMessage = "\n";
                    //send the line we got from the file and get another line
                    current_client.sendData(sendMessage);
                    getline(fin, sendMessage);

                    //wait a little bit so the client can definitely get the message correctly
                    //Sleep(5); //sleep for 5 milliseconds
                }

                current_client.sendData("EOF"); //send to the client that we're at the eof
                current_client.recvData(recMessage); //get the final OK
            }

        }

        else
        {
            cout << "Client didn't send 'MAIL FROM' or 'INBOX' in the beginning\n";
            current_client.sendData(Status::SMTP_CMD_SNTX_ERR);
        }

        //get data from the client before starting loop again
        clientFlop = current_client.recvData(recMessage);

        if(recMessage == "QUIT" || clientFlop == -1) //if they sent quit, break from while loop and the thread will end after exiting this
        {
            current_client.sendResponse(Status::SMTP_SRV_CLOSE, "OK -- Goodbye...");
            break;
        }

    } //end of while
}

int main(int argc, char * argv[])
{
    if(argc != 2)
    {
        cout << "Usage: " << argv[0] << " <listening IP>" << endl;
        return -69;
    }
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

    sSrvrIP = string(argv[1]);
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
