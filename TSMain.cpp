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
#include "ClientSocket.h"
#include "Status.h"
#include <algorithm>

using namespace std;

string sSrvrIP; //global variable which holds the IP address of the server
bool isWritten = false; //global boolean which will indicate whether or not we have written to the fifo file

string upCase(string str)
{
    std::transform(str.begin(), str.end(),str.begin(), ::toupper);
    return str;
}

//our thread for FIFO reading and message relaying - doesn't need a parameter passed in at this point
DWORD WINAPI relayMail(LPVOID lpParam)
{
    cout << "FIFO Thread Created\n";

    while(true) //endless loop to open the fifo file and send the email in it
    {
        string recMessage = ""; //will hold the command the client sent
        string sendMessage = ""; //will hold the reply we send
        vector<string> message;
        string line = ""; //will hold each line we read in from the file
        int serverFlop = 0; //will hold value given by recv function and will be -1 if the server flops and shuts down
        ifstream fin; //file input object to read stuff in from the message fifo queue
        bool isSent = false; //bool that will hold if the message sent to the server successfully
        bool isFTS = false;
        int timeoutCount = 0; //timeout counter
        ClientSocket fifoClient; //create an instance of clientsocket called fifoClient
        timeoutCount = 0; //reset the timeout counter

        if(isWritten) //if we have ownership of the mutex
        {
            fin.open("email.fifo"); //open the email.fifo file
            if(!fin.is_open())
            {
                Sleep(100); //wait a little while before trying to open the file again
            }
            else
            {
                getline(fin, line); //get the first line from the file

                while(line != ".") //while we don't read in a period, keep going. period denotes the end of a message
                {
                    message.push_back(line); //add the lines to the vector that will hold the message
                    getline(fin, line); //get the next line
                }

                fin.close(); //close file; done reading stuff in

                remove("email.fifo"); //remove the file after we're done with it
                isWritten = false; //set isWritten equal to false because now we read in and deleted the file

                while(!isSent)
                {
                    timeoutCount++; //adding one to the timeout count.

                    //connect to the server where the message should be going
                    if(!fifoClient.connectToServer(message[1].substr(message[1].find("@")+1).c_str(), 31000))
                    {
                        ofstream fout;
                        fout.open("fts.flop", ios::app); //opening the new flop

                        for (int i =0; i < message.size(); i++) //writing the contents of the vector to the error log
                            fout << message[i] << endl; //writing the message
                        
                        fout.close();
                        isSent = true;
                        isFTS = true;
                    }
                    else
                    {
                        //receive the first 220 message
                        serverFlop = fifoClient.recvData(recMessage);
                        if(serverFlop == -1) //check if the server flopped
                        {
                            fifoClient.closeConnection();
                            continue;
                        }

                        if(recMessage.substr(0,3) == "220")
                        {
                            fifoClient.sendData("HELO 127.0.0.1");
                        }
                        else
                        {
                            fifoClient.closeConnection();
                            continue;
                        }

                        serverFlop = fifoClient.recvData(recMessage); //receive next status message from server

                        if(serverFlop == -1) //check if the server flopped
                        {
                            fifoClient.closeConnection();
                            continue;
                        }

                        if(recMessage.substr(0,3) == "250") //send the login information as long as we got a 250 from the server first
                        {
                            //sendMessage = "VRFY " + message[2].substr(0, message[2].find("@"));
                            sendMessage = "VRFY guest"; //login using guest account
                            fifoClient.sendData(sendMessage);
                        }
                        else //if we can't login, close connection
                        {
                            fifoClient.closeConnection();
                            continue;
                        }

                        serverFlop = fifoClient.recvData(recMessage); //receive next status message from server

                        if(serverFlop == -1) //check if the server flopped
                        {
                            fifoClient.closeConnection();
                            continue;
                        }

                        if(recMessage == "550" || recMessage == "500") //if login fails, print error and close connection
                        {
                            cout << "Invalid user when trying to login as guest...\n";
                            fifoClient.closeConnection(); //close connection
                            continue;
                        }

                        //send the message in the fifo queue
                        sendMessage = "MAIL FROM:<" + message[2] + ">";
                        fifoClient.sendData(sendMessage); //send the mail from command to the server
                        serverFlop = fifoClient.recvData(recMessage); //receive next status message from server
                        if(serverFlop == -1) //check if the server flopped
                        {
                            fifoClient.closeConnection();
                            continue;
                        }

                        //check for an error
                        if(!fifoClient.checkError(recMessage, Status::SMTP_ACTION_COMPLETE))
                        {
                            cout << recMessage << endl;
                            fifoClient.closeConnection();
                            continue;
                        }

                        sendMessage = "RCPT TO:<" + message[1] + ">"; //set what we're sending
                        fifoClient.sendData(sendMessage); //send data
                        serverFlop = fifoClient.recvData(recMessage); //get response

                        if(serverFlop == -1) //check if the server flopped
                        {
                            fifoClient.closeConnection();
                            continue;
                        }

                        //check for an error
                        if(!fifoClient.checkError(recMessage, Status::SMTP_ACTION_COMPLETE))
                        {
                            cout << recMessage << endl;
                            fifoClient.closeConnection();
                            continue;
                        }

                        //send data command to the server and get a response
                        fifoClient.sendData("DATA"); //send that we're ready to send data
                        serverFlop = fifoClient.recvData(recMessage); //get the response

                        if(serverFlop == -1) //check if the server flopped
                        {
                            fifoClient.closeConnection();
                            continue;
                        }

                        //check for an error
                        if(!fifoClient.checkError(recMessage, Status::SMTP_BEGIN_MSG))
                        {
                            cout << recMessage << endl;
                            fifoClient.closeConnection();
                            continue;
                        }

                        for(int i = 3; i < message.size(); i++)
                        {
                            sendMessage = message[i];

                            if(sendMessage == "") //check if it's an empty string, if so add a newline because a getline drops that
                                sendMessage = "\n";

                            Sleep(250);
                            fifoClient.sendData(sendMessage); //send the data, it's already encrypted
                        }
                        fifoClient.sendData(".");

                        serverFlop = fifoClient.recvData(recMessage); //get data from server

                        if(serverFlop == -1) //check if the server flopped
                        {
                            fifoClient.closeConnection();
                            continue;
                        }
                        
                        //check for an error, if there was an error we need to try this entire loop again
                        if(fifoClient.checkError(recMessage, Status::SMTP_ACTION_COMPLETE))
                        {
                            cout << "Message sent successfully! :)\n\n";
                            isSent = true;
                        }
                        else
                        {
                            cerr << "Error sending message. Please retry in a few minutes. :(\n\n";
                            isSent = false;
                        }
                    }


                } //end of the sending while

                //close connection to the server because we're done
                if(!isFTS)
                {
                    fifoClient.closeConnection();
                }

                isWritten = false; ///adding this so it doesn't shiot the bed
            } //end of the else
        } //end of the if checking the mutex result
        Sleep(5000); //wait a little while before trying to open the file again
    } //end of the entire while loop

}

//our thread for client connections - takes in a socket as a parameter
DWORD WINAPI handleMail(LPVOID lpParam)
{
    cout << "Email Thread Created\n";

    //set our socket to the socket passed in as a parameter
    ThreadSock current_client;
    current_client.setSock((SOCKET)lpParam);

    string recMessage = ""; //will hold the command the client sent
    string sendMessage = ""; //will hold the reply we send
    bool isGuest = false;
    int clientFlop = 0; //will hold the value that the recv function returns

    //send the welcome message and receive input back from the client
    current_client.sendResponse(Status::SMTP_SRV_RDY, "Welcome to the SMTP Server!"); //send initiation hello
    clientFlop = current_client.recvData(recMessage); //get data from the client

    //checking out the string to see if it's helo
	bool bHeloSent = FALSE;
	do{
        if(clientFlop == -1) //check if the client flops and disconnects and break if they do
            break;

		if (upCase(recMessage.substr(0,4)) == "HELO") //if the first word is helo
		{
			current_client.sendResponse(Status::SMTP_ACTION_COMPLETE, "Welcome to MAST-Stimpy@" + sSrvrIP); //send 250 and welcome message
			cout << "Connection Successful. We received a HELO from the client.\n";
			bHeloSent = TRUE;
		}
		else //if it's not HELO, return error code
		{
			current_client.sendData(Status::SMTP_CMD_SNTX_ERR); //sending the error code
			cout << "Connection Failed. We did not recieive a HELO from client...\n";
			clientFlop = current_client.recvData(recMessage); //get data from the client
		}
	} while (!bHeloSent);

    current_client.recvData(recMessage); //receiving the verify and a username
    string username;

    //checking to see if it's a verify
    if (upCase(recMessage.substr(0,4)) == "VRFY")
    {
        username = recMessage.substr(5); //get the username from the command the user sent

        //if it is, validate the username and continue
        if (current_client.validateUser(username))
        {
            current_client.sendData(Status::SMTP_ACTION_COMPLETE); //if the username was valid, send back 250
        }
        else //make any other username that tries to login, a guest account
        {
            isGuest = true; //changing the value of the bool to true
            current_client.sendData(Status::SMTP_ACTION_COMPLETE);
        }
    }

    //getting data from the client
    clientFlop = current_client.recvData(recMessage);

    //our send/recv loop
    while(upCase(recMessage) != "QUIT")
    {
		bool bRecipientSent = FALSE;
        bool canWrite = false; //will be true if this thread can write to the file
		string sRecipient = "";

        if(upCase(recMessage.substr(0,9)) == "MAIL FROM")
        {
            string sender = recMessage.substr(11, recMessage.length()-12);
            //cout << "Sender  = " << sender << endl;
            
            cout << "Client Sent: " << recMessage << endl; //for debugging
            current_client.sendData(Status::SMTP_ACTION_COMPLETE);

            //get more data from client
            clientFlop = current_client.recvData(recMessage);

            //rcpt to section
            if (upCase(recMessage.substr(0,7)) != "RCPT TO") //checking to see if it's RCPT TO
            {
                cout << "RCPT TO wasn't sent...\n"; //should probably sent syntax error back
                current_client.sendData(Status::SMTP_CMD_SNTX_ERR); //send error
            }
            else
            {
                cout << "Client Sent: " << recMessage << endl; //for debugging
                //sRecipient = recMessage.substr(9, recMessage.find("@")-9);
                sRecipient = recMessage.substr(9, recMessage.length()-10); //the entire string including IP Address

                //checking to see if the user is valid
                bool bLocalDelivery = FALSE;

                if (sRecipient.find("@") == -1)
                {
                    cout << "Assuming this is a local address\n";
                    sRecipient += "@" + sSrvrIP;
                    bLocalDelivery = TRUE;
                }

                string sSrvrT = recMessage.substr(recMessage.find("@")+1); //@ to end, including the >
                string sSrvr = sSrvrT.substr(0, sSrvrT.length()-1); //@ to end, minus the bracet

                if ((sSrvr == "127.0.0.1" || sSrvr == sSrvrIP || sSrvr == ""))
                {
                    bLocalDelivery = TRUE;
                }

                if(isGuest && !bLocalDelivery) //if the guest account tries to send an email to a user not on our server, send bad error code
                    current_client.sendResponse(Status::SMTP_CMD_SNTX_ERR, "Guest doesn't have permission to send emails to outside servers.");
                else if(bLocalDelivery && !current_client.validateUser(sRecipient.substr(0,sRecipient.find("@"))))
                    current_client.sendResponse(Status::SMTP_CMD_SNTX_ERR, "Malformed Recipient"); //sending back a bad error code
                else
                {
                    current_client.sendResponse(Status::SMTP_ACTION_COMPLETE, "OK"); //if the username was valid, send back 250
                    bRecipientSent = TRUE;

                    //getting data and writing to file part
                    //cout << "before recvdata: " << recMessage << endl;
                    clientFlop = current_client.recvData(recMessage); //getting more data from client
                    //cout << "after recvdata " << recMessage << endl;

                    //checking to see if the string is DATA
                    if (upCase(recMessage.substr(0,6)) != "DATA")
                    {
                        cout << "DATA wasn't received\n";
                        current_client.sendData(Status::SMTP_CMD_SNTX_ERR); //send error
                    }
                    else
                    {
                        //get the data of the message part
                        //create file output object and open it in append mode
                        ofstream fout;

                        if(bLocalDelivery) //if local
                            fout.open ((string(sRecipient.substr(0,sRecipient.find("@")) + ".txt")).c_str(), ios::app);
                        else //if on a different server and needs to be relayed
                        {
                            while(!canWrite)
                            {
                                if(!isWritten)
                                {
                                    fout.open("email.fifo", ios::app);
                                    canWrite = true;
                                }
                            }
                        }

                        //write the initial part of the email
                        fout << current_client.getDateTime() << endl << sRecipient << endl << sender << endl;

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

                        //write a . to denote the end of the message
                        fout << "." << endl;

                        //send status code that action is complete and close the file
                        current_client.sendData(Status::SMTP_ACTION_COMPLETE);
                        fout.close();
                        isWritten = true; //set this to true because the file is now written
                    }
                }
            }
        }

        else if(upCase(recMessage.substr(0,5)) == "INBOX") //if they send an inbox and want to check their inbox
        {
            cout << "The Client Sent: " << recMessage << endl;

            if (isGuest == true) //if isGuest is true, send an error and send error
            {
                current_client.sendResponse(Status::SMTP_MBOX_UNAV, "No mailbox on a guest account.");
            }

            else
            {
                //open file for the user's mailbox that is logged in
                ifstream fin(string(username + ".txt").c_str()); //file input object

                if(!fin.is_open()) //check if the file opens or not
                {
                    current_client.sendResponse(Status::SMTP_MBOX_UNAV, "No messages in your inbox."); //send back that there aren't any messages
                } else
                {
                    current_client.sendResponse(Status::SMTP_ACTION_COMPLETE, "OK"); //send 250 OK so they know we got the command okay then send mail

                    getline(fin, sendMessage); //get first line from file
                    
                    while(!fin.eof()) //until we get to the end of the file
                    {
                        if(sendMessage == "") //if it doesn't have anything, then make it a newline because getline skips over it
                            sendMessage = "\n";

                        //send the line from the file and get an ok from the client
                        current_client.sendData(sendMessage);
                        clientFlop = current_client.recvData(recMessage);

                        if(clientFlop == -1) //check to see if they actually sent a message and break if they didn't
                            break;

                        //get another line from the file
                        getline(fin, sendMessage);
                    }

                    current_client.sendData("EOF"); //send to the client that we're at the eof
                }
            }
        }

        else
        {
            cout << "Client didn't send 'MAIL FROM' or 'INBOX' in the beginning\n";
            current_client.sendData(Status::SMTP_CMD_SNTX_ERR);
        }

        //get data from the client before starting loop again
        clientFlop = current_client.recvData(recMessage);

        if (clientFlop == -1) //if they sent quit, break from while loop and the thread will end after exiting this
        {
            break;
        }

        if(upCase(recMessage) == "QUIT")
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
    DWORD thread2;

    //socket that we sendrecv data on
    SOCKET client;

    sockaddr_in from;
    int fromlen = sizeof(from);

    //create our fifo thread only once because it will loop continuously
    CreateThread(NULL, 0,relayMail,(LPVOID)NULL, 0, &thread);

    //loop forever
    while(true)
    {
        //accept connections
        client = accept(sock,(struct sockaddr*)&from,&fromlen);
        cout << "Client connected.\n";
        //create our handleMail thread and pass client socket as a parameter
        CreateThread(NULL, 0,handleMail,(LPVOID)client, 0, &thread2);
    }

    WSACleanup(); //windows cleanup

    return 0; //ends program
}



