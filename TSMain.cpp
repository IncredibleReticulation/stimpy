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
HANDLE mailMutex; //Golbal Mutex Variable

string upCase(string str)
{
    std::transform(str.begin(), str.end(),str.begin(), ::toupper);
    return str;
}

//our thread for FIFO reading and message relaying - doesn't need a parameter passed in at this point
DWORD WINAPI relayMail(LPVOID lpParam)
{
    cout << "FIFO Thread Created\n";

    //create a threadsock object and set our socket to the socket passed in as a parameter
    ClientSocket fifoClient; //create an instance of clientsocket called fifoClient
    DWORD dwWaitResult;     //wait for instance

    string recMessage = ""; //will hold the command the client sent
    string sendMessage = ""; //will hold the reply we send
    vector<string> message;
    string line = ""; //will hold each line we read in from the file
    int serverFlop = 0; //will hold value given by recv function and will be -1 if the server flops and shuts down
    ifstream fin; //file input object to read stuff in from the message fifo queue
    bool isSent = false;
    int timeoutCount = 0;

    while(true) //endless loop to open the fifo file and send the email in it
    {
        //see: http://msdn.microsoft.com/en-us/library/ms687032%28v=vs.85%29.aspx
        //cout << "before wait in fifo thread\n";
        // dwWaitResult = WaitForSingleObject( 
        //     mailMutex,    // handle to mutex
        //     INFINITE);  // no time-out interval
        //cout << "after wait in fifo thread\n";
        dwWaitResult = WAIT_OBJECT_0;

        if(dwWaitResult == WAIT_OBJECT_0) //if we have ownership of the mutex
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
                //ReleaseMutex(mailMutex); //release the mutex from the thread

                while(!isSent)
                {
                    timeoutCount++; //adding one to the timeout count.

                    //connect to the server where the message should be going
                    fifoClient.connectToServer(message[1].substr(message[1].find("@")+1).c_str(), 31000);

                    //receive the first 220 message
                    serverFlop = fifoClient.recvData(recMessage);
                    if(recMessage.substr(0,3) == "220")
                    {
                        fifoClient.sendData("HELO 127.0.0.1");
                    }

                    serverFlop = fifoClient.recvData(recMessage); //receive next status message from server
                    if(recMessage.substr(0,3) == "250") //send the login information as long as we got a 250 from the server first
                    {
                        //sendMessage = "VRFY " + message[2].substr(0, message[2].find("@"));
                        sendMessage = "VRFY guest"; //login using guest account
                        fifoClient.sendData(sendMessage);
                    }

                    serverFlop = fifoClient.recvData(recMessage); //receive next status message from server

                    if(recMessage == "550" || recMessage == "500") //if login fails, print error and end program
                    {
                        cout << "Invalid user...\n";
                        fifoClient.closeConnection(); //close connection
                        //break;
                    }

                    //send the message in the fifo queue
                    sendMessage = "MAIL FROM:<" + message[2] + ">";
                    fifoClient.sendData(sendMessage); //send the mail from command to the server
                    serverFlop = fifoClient.recvData(recMessage); //receive next status message from server

                    //check for an error
                    if(!fifoClient.checkError(recMessage, Status::SMTP_ACTION_COMPLETE))
                    {
                        cout << recMessage << endl;
                        //break; //break if we found one
                    }

                    sendMessage = "RCPT TO:<" + message[1] + ">"; //set what we're sending
                    fifoClient.sendData(sendMessage); //send data
                    serverFlop = fifoClient.recvData(recMessage); //get response

                    //check for an error
                    if(!fifoClient.checkError(recMessage, Status::SMTP_ACTION_COMPLETE))
                    {
                        cout << recMessage << endl;
                        //break; //break if we found one
                    }

                    //send data command to the server and get a response
                    fifoClient.sendData("DATA"); //send that we're ready to send data
                    serverFlop = fifoClient.recvData(recMessage); //get the response

                    //check for an error
                    if(!fifoClient.checkError(recMessage, Status::SMTP_BEGIN_MSG))
                    {
                        cout << recMessage << endl;
                        //break; //break if we found one
                    }

                    int index = 3; //the position of message we should get data from
                    while(sendMessage != ".") //while user doesn't enter a period, keep sending data for message
                    {
                        sendMessage = message[index];

                        if(sendMessage == "") //check if it's an empty string, if so add a newline because a getline drops that
                            sendMessage = "\n";

                        fifoClient.sendData(sendMessage); //send the data, it's already encrypted
                        index++; //increment the index so we can get the next part of the message
                    }

                    serverFlop = fifoClient.recvData(recMessage); //get data from server

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

                    if(serverFlop == -1) //if the server disconnects unexpectedly break from the while loop
                    {
                        isSent = false;
                        //break; //not sure if we want to break from the while loop or not, because then the message will not be sent and the file
                                //will be deleted without being sent anywhere. maybe we should write it to another text file if doesn't send? idk
                    }

                    if (timeoutCount == 3) //if the count gets to three, write to the log file
                    {
                        ofstream fout;
                        fout.open("fts.flop", ios::app); //opening the new flop

                        for (int i =0; i < message.size(); i++) //writing the contents of the vector to the error log
                            fout << message[i] << endl; //writing the message
                        
                        fout.close();
                        break;
                    }
                } //end of the sending while
            } //end of the else
        } //end of the if checking the mutex result

        Sleep(1000); //wait a little while before trying to open the file again
    } //end of the entire while loop

}

//our thread for client connections - takes in a socket as a parameter
DWORD WINAPI handleMail(LPVOID lpParam)
{
    cout << "Email Thread Created\n";

    //set our socket to the socket passed in as a parameter
    ThreadSock current_client;
    current_client.setSock((SOCKET)lpParam);
    DWORD dwWaitResult;     //wait for instance

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
    while(upCase(recMessage) != "QUIT" || recMessage != "Quit" || recMessage != "quit")
    {
		bool bRecipientSent = FALSE;
        bool isOwned = false; //will hold if the data has been sent and written to file
		string sRecipient = "";

        if(upCase(recMessage.substr(0,9)) == "MAIL FROM")
        {
            string sender = recMessage.substr(11, recMessage.length()-12);
            cout << "Sender  = " << sender << endl;

            if (sender.find("@") == -1)
            {
                cout << "Assuming this is a local address\n";
                sender += "@" + sSrvrIP;
            }
            
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
                cout << "Client Send: " << recMessage << endl; //for debugging
                //sRecipient = recMessage.substr(9, recMessage.find("@")-9);
                sRecipient = recMessage.substr(9, recMessage.length()-10); //the entire string including IP Address
                string sSrvrT = recMessage.substr(recMessage.find("@")+1); //@ to end, including the >
                string sSrvr = sSrvrT.substr(0, sSrvrT.length()-1); //@ to end, minus the bracet
                cout << "User server: " << sSrvr << ">" << endl;
                //cout << "recipient username: " << recMessage.substr(9, recMessage.find("@")-9) << endl; //for debugging
                //checking to see if the user is valid
                bool bLocalDelivery = FALSE;

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
                            //cout << "before while loop\n";
                            while(!isOwned)
                            {
                                //cout << "after while loop, before wait call\n";
                                //IT BREAKS WHEN RIGHT HERE. it never gets to the cout after calling this function to wait IDK why dad
                                // dwWaitResult = WaitForSingleObject(
                                //     mailMutex,    // handle to mutex
                                //     INFINITE);   // no time-out interval
                                dwWaitResult = WAIT_OBJECT_0;

                                //cout << "after waiting for single object\n";

                                if(dwWaitResult == WAIT_OBJECT_0)
                                {
                                    cout << "in if inside of while\n";
                                    fout.open("email.fifo", ios::app);
                                    isOwned = true;
                                }
                            }
                        }

                        //write the initial part of the email
                        //fout << "\"" << current_client.getDateTime() << "\",\"" << sRecipient << "\",\"" << username << "\",\"";
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

                        //write the final quotation and add a newline to the end of it
                        //fout << "\"" << endl;
                        //char del = 236;
                        fout << "." << endl;

                        //send status code that action is complete and close the file
                        current_client.sendData(Status::SMTP_ACTION_COMPLETE);
                        fout.close();

                        // if(!bLocalDelivery) //if it was not local, we used a mutex and need to release it
                        //     ReleaseMutex(mailMutex); //Release the mutex from the thread
                    }
                }
            }
        }

        else if(upCase(recMessage.substr(0,5)) == "INBOX") //if they send an inbox and want to check their inbox
        {
            cout << "The Client Sent: " << recMessage << endl;

            if (isGuest == true) //if isGuest is true, send an error and send error
            {
                current_client.sendResponse(Status::SMTP_CMD_SNTX_ERR, "No mailbox on a guest account.");
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

        if(upCase(recMessage) == "QUIT" || clientFlop == -1) //if they sent quit, break from while loop and the thread will end after exiting this
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

    // mailMutex = CreateMutex( 
    //     NULL,              // default security attributes
    //     FALSE,             // initially not owned
    //     NULL);             // unnamed mutex

    //create our fifo thread only once because it will loop continuously
    CreateThread(NULL, 0,relayMail,(LPVOID)NULL, 0, &thread);

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
    //CloseHandle(mailMutex); //Close the Handle for Mutex

    return 0; //ends program
}



