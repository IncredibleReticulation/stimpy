//Course: 4050-212-02
//Authors: Alex Buie, Luke Matarazzo, Jackson Sadowski, Steven Tucker
//IDE/editor: Sublime Text 2
//Compilers: mingw32-g++
//Final SMTP Project
//Filename: Client.cpp   client main
//Purpose: 

#include <iostream>
#include <cstdlib>
#include "Status.h"
#include "ClientSocket.h"

int main(int argc, char * argv[])
{
    if(argc != 3)
    {
        cout << "USAGE: " << argv[0] << " servername(ip) portnum (usually 25)" << endl;
        return 1;
    }

    int port = atoi(argv[2]); //port number to connect to
    string ipAddress = string(argv[1]); //ip address to connect to
    string recMessage; //message it receives
    string sendMessage; //message it sends
    string username = ""; //will hold username
    int serverFlop = 0; //will hold value given by recv function and will be -1 if the server flops and shuts down

    //print that we're attempting to connect
    cout << "Connecting to: " << ipAddress << ":" << port << endl;

    ClientSocket sockClient; //clientsocket object instance
    sockClient.connectToServer(ipAddress.c_str(), port); //connect to the server using the ip and port given

    //receive the first 220 message
    serverFlop = sockClient.recvData(recMessage);
    if(recMessage.substr(0,3) == "220")
    {
        sockClient.sendData("HELO 127.0.0.1");
    }

    serverFlop = sockClient.recvData(recMessage); //receive next status message from server
    if(recMessage.substr(0,3) == "250") //prompt for login info and send it
    {
        cout << "Username: ";
        getline(cin, username);
        sendMessage = "VRFY " + username;
        sockClient.sendData(sendMessage);
    }

    //receive server response after login
    serverFlop = sockClient.recvData(recMessage);

    //check if we logged in successfully
    if(recMessage == "550" || recMessage == "500") //if login fails, print error and end program
    {
        cout << "Invalid user...\n";
        sockClient.closeConnection(); //close connection
        return 1;
    }
    else if(recMessage == "250")
    {
        cout << "Logon successful.\n\n";
    }

    string menu = "1. Send Email\n2. Read Inbox\n3. Quit\n"; //our menu of options
    int option = 1;

    while(option != 3) //while they don't enter 3 for the quit option, keep prompting for selection
    {
        if(serverFlop == -1)
        {
            cout << "There's been an unknown error on the server. Try reconnecting momentarily...\n\n";
            break;
        }
        if(option > 0 && option < 4) //only print menu if they entered a valid option last time
        {
            cout << menu; //print menu
        }

        //prompt for an option and get the option
        cout << endl << "Enter option: ";
        cin >> option;

        string recipient; //will hold the recipient

        switch(option)
        {
            case 1: //option 1, to send an email
                //send who the mail is from and receive response
                sendMessage = "MAIL FROM:<" + username + "@" + ipAddress + ">"; //set what we're sending
                sockClient.sendData(sendMessage); //notify server that we're sending mail
                serverFlop = sockClient.recvData(recMessage); //get the response from the server

                //check for an error
                if(!sockClient.checkError(recMessage, Status::SMTP_ACTION_COMPLETE))
                {
                    cout << recMessage << endl;
                    break; //break if we found one
                }
                    

                //get recipient of the email
                cout << "Enter the recipient's email address: "; //prompts for recipient
                cin >> recipient; //get the recipient

                //send recipient of the email
                sendMessage = "RCPT TO:<" + recipient + ">"; //set what we're sending
                sockClient.sendData(sendMessage); //send data
                serverFlop = sockClient.recvData(recMessage); //get response

                //check for an error
                if(!sockClient.checkError(recMessage, Status::SMTP_ACTION_COMPLETE))
                {
                    cout << recMessage << endl;
                    break; //break if we found one
                }
                    

                //send data to the server and get a response
                sockClient.sendData("DATA"); //send that we're ready to send data
                serverFlop = sockClient.recvData(recMessage); //get the response

                //check for an error
                if(!sockClient.checkError(recMessage, Status::SMTP_BEGIN_MSG))
                    break; //break if we found one

                //get data from client and send it over
                cout << "Enter data. Press '.' when the message is over.\n"; //prompt for data
                cin.ignore(10000, '\n'); //ignore any newlines
                //getline(cin, sendMessage); //get the message to send
                
                while(sendMessage != ".") //while user doesn't enter a period, keep sending data for message
                {
                    cout << "Data > "; //prompt for data
                    getline(cin, sendMessage); //get the message to send

                    if(sendMessage == "") //check if it's an empty string, if so add a newline because a getline drops that
                        sendMessage = "\n";

                    sockClient.sendData(sendMessage); //send the data
                }

                //get response after sending data and print status message for user
                cout << "Sending data. Waiting for server...\n";
                // sockClient.sendData(sendMessage); //send the period over
                serverFlop = sockClient.recvData(recMessage); //get data from server

                //check for an error
                if(sockClient.checkError(recMessage, Status::SMTP_ACTION_COMPLETE))
                    cout << "Message sent successfully! :)\n\n";
                else
                    cerr << "Error sending message. Please retry in a few minutes. :(\n\n";

                break; //break from case
            case 2: //option 2, to read messages in the user's mailbox
                //code
                cout << "Read messages option net implemented yet...\n";
                break;
            case 3: //option 5, to quit
                //code
                cout << "You chose to quit, goodbye.\n\n";
                sockClient.sendData("QUIT"); //send quit to the server so it knows we're disconnecting
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
