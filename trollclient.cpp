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

//prototypes
string trim(string);

int main(int argc, char * argv[])
{
    if(argc != 3)
    {
        cout << "USAGE: " << argv[0] << " servername(ip) portnum (usually 31000)" << endl;
        return 1;
    }

    int port = atoi(argv[2]); //port number to connect to
    string ipAddress = string(argv[1]); //ip address to connect to
    string recMessage; //message it receives
    string sendMessage; //message it sends
    string username = ""; //will hold username
    char trollChoice = ' '; //will hold if they want to troll the HELO process or not
    int serverFlop = 0; //will hold value given by recv function and will be -1 if the server flops and shuts down

    //print that we're attempting to connect
    cout << "Connecting to: " << ipAddress << ":" << port << endl;

    ClientSocket sockClient; //clientsocket object instance
    sockClient.connectToServer(ipAddress.c_str(), port); //connect to the server using the ip and port given

    cout << "Do you want to connect normal and troll (Y) or troll the HELO process (N): ";
    cin >> trollChoice;
    cin.ignore(10000, '\n');

    //receive the first 220 message
    serverFlop = sockClient.recvData(recMessage);

    if(toupper(trollChoice) == 'N') //if they want to mess with the HELO process
    {
        string badCommand = "";
        cout << "What would you like to send them (type quit to exit): ";
        getline(cin, badCommand);

        while(badCommand != "quit" && badCommand != "QUIT" && badCommand != "Quit")
        {
            sockClient.sendData(badCommand);
            serverFlop = sockClient.recvData(recMessage);

            cout << "\nResponse: " << recMessage << endl;

            if(serverFlop == -1)
            {
                cout << "they flopped\n\n";
                break;
            }

            cout << "\nWhat would you like to send them (type quit to exit): ";
            getline(cin, badCommand);
        }

        if(serverFlop != -1)
        {
            cout << "Send QUIT (Y) or hard disconnect (N): ";
            cin >> trollChoice;

            if(toupper(trollChoice) == 'Y')
                sockClient.sendData("QUIT");

            cout << "You chose to quit, goodbye.\n\n";
        }
        
        sockClient.closeConnection(); //closes connection
        return 0; //ends program
    }

    if(recMessage.substr(0,3) == "220")
    {
        sockClient.sendData("HELO THISISTROLL69.69.69.69");
    }
    else
    {
        cout << "Server did not indicate that they were ready to initiate a connection :(\n";
        return 69; //end program
    }

    serverFlop = sockClient.recvData(recMessage); //receive next status message from server
    if(recMessage.substr(0,3) == "250") //prompt for login info and send it
    {
        cout << "Username: ";
        getline(cin, username);
        username = trim(username);

        sendMessage = "VRFY " + username;
        sockClient.sendData(sendMessage);
    }
    else
    {
        cout << "Server may not have gotten our 'HELO' :(\n";
        return 69; //ends program
    }

    //receive server response after login
    serverFlop = sockClient.recvData(recMessage);

    //check if we logged in successfully
    if(recMessage.substr(0,3) == "550" || recMessage.substr(0,3) == "500") //if login fails, print error and end program
    {
        cout << "Invalid user...\n";
        sockClient.closeConnection(); //close connection
        return 1;
    }
    else if(recMessage.substr(0,3) == "250")
    {
        cout << "Logon successful.\n\n";
    }
    else
    {
        cout << "Unknown error when attempting to login to server...\n";
        return -69; //ends program
    }

    //menu options
    string menu = "1. Send mad data\n2. Send read inbox a lot\n3. Send bad command a lot\n4. Send VRFY a lot\n";
    menu += "5. Send the same email a lot (takes a little bit of time)\n6. Quit\n";
    int option = 1;

    while(option != 6) //while they don't enter 3 for the quit option, keep prompting for selection
    {
        if(serverFlop == -1)
        {
            cout << "There's been an unknown error on the server. Try reconnecting momentarily...\n\n";
            break;
        }
        if(option > 0 && option < 7) //only print menu if they entered a valid option last time
        {
            cout << menu; //print menu
        }

        //prompt for an option and get the option
        cout << endl << "Enter option: ";
        cin >> option;

        string recipient; //will hold the recipient
        int messageCount = 0; //will count how many messages we have received. for formatting purposes
        int lineCount = 0; //will count how many lines per message were sent. needed to know when to data is being sent
        int numLines = 0; //will hold number of lines of data or commands
        int numMessages = 0; //will hold number of emails to send
        char choice = ' '; //will hold the choice of whether they want to send real or garbage data for option 5
        vector<string> messageHolder; //will hold the message when we send the same message multiple times
        vector<string> headerHolder; //will hold the header of the message when we send the same message a lot
        string badCommand = ""; //will hold the bad command we want to send
        string messBuf = ""; //will hold the entire message and act as a buffer before printing it

        switch(option)
        {
            case 1: //option 1, to send an email
                //send who the mail is from and receive response
                sendMessage = "MAIL FROM:<guest@" + ipAddress + ">"; //set what we're sending
                sockClient.sendData(sendMessage); //notify server that we're sending mail
                serverFlop = sockClient.recvData(recMessage); //get the response from the server

                //check for an error
                if(!sockClient.checkError(recMessage, Status::SMTP_ACTION_COMPLETE))
                {
                    //cout << recMessage << endl;
                    break; //break if we found one
                }
                    
                //get recipient of the email
                cout << "Enter the recipient's email address (user@1.2.3.4): "; //prompts for recipient
                cin >> recipient; //get the recipient

                //send recipient of the email
                sendMessage = "RCPT TO:<" + recipient + ">"; //set what we're sending
                sockClient.sendData(sendMessage); //send data
                serverFlop = sockClient.recvData(recMessage); //get response

                //check for an error
                if(!sockClient.checkError(recMessage, Status::SMTP_ACTION_COMPLETE))
                {
                    //cout << recMessage << endl;
                    break; //break if we found one
                }

                //send data to the server and get a response
                sockClient.sendData("DATA"); //send that we're ready to send data
                serverFlop = sockClient.recvData(recMessage); //get the response

                //check for an error
                if(!sockClient.checkError(recMessage, Status::SMTP_BEGIN_MSG))
                    break; //break if we found one

                cout << "how many lines of garbage would you like to send? (0 means infinite): ";
                cin >> numLines;

                //set sendmessage
                sendMessage = "YOU ARE BEING TROLLED. TROLOLOL. YOU ARE BEING TROLLED. TROLOLOL. YOU ARE BEING TROLLED. TROLOLOL.";

                if(numLines == 0)
                {
                    while(true) //send forever
                    {
                        sockClient.sendData(sendMessage); //send the data
                        sockClient.recvData(recMessage); //receive message from server
                    }
                }
                else
                {
                    for(int i = 0; i < numLines; i++) //send for user specified amount
                    {
                        sockClient.sendData(sendMessage); //send the data
                        sockClient.recvData(recMessage); //receive message from server
                    }
                }
                
                Sleep(250);
                sockClient.sendData("."); //send final period
                cout << "Payload complete.\n\n";
                break; //break from case
            case 2: //option 2, to read messages in the user's mailbox
                cout << "how many times would you like to send INBOX? (0 means infinite): ";
                cin >> numLines;

                if(numLines == 0)
                {
                    while(true) //send forever
                    {
                        sockClient.sendData("INBOX"); //send the data
                        sockClient.recvData(recMessage); //receive message from server
                    }
                }
                else
                {
                    for(int i = 0; i < numLines; i++) //send for user specified amount
                    {
                        sockClient.sendData("INBOX"); //send the data
                        sockClient.recvData(recMessage); //receive message from server
                    }
                }

                cout << "Payload complete.\n\n";
                break;
            case 3:
                cout << "How many bad commands would you like to send (0 means infinite): ";
                cin >> numLines;

                cout << "What bad command would you like to send: ";
                cin.ignore(1000, '\n');
                getline(cin, badCommand);

                if(numLines == 0)
                {
                    while(true)
                    {
                        sockClient.sendData(badCommand);
                        sockClient.recvData(recMessage); //receive message from server
                    }
                }
                else
                {
                    for(int i = 0; i < numLines; i++)
                    {
                        sockClient.sendData(badCommand);
                        sockClient.recvData(recMessage); //receive message from server
                    }
                }

                cout << "Payload complete.\n\n";
                break;
            case 4:
                cout << "How many times would you like to send VRFY (0 means infinite): ";
                cin >> numLines;

                if(numLines == 0)
                {
                    while(true) //send forever
                    {
                        sockClient.sendData((numLines++ % 2 == 0 ? "VRFY TROLL" : "VRFY ")); //send the data
                        sockClient.recvData(recMessage); //receive message from server
                    }
                }
                else
                {
                    for(int i = 0; i < numLines; i++) //send for user specified amount
                    {
                        sockClient.sendData(((numLines % 2 == 0 ? "VRFY TROLL" : "VRFY "))); //send the data
                        sockClient.recvData(recMessage); //receive message from server
                    }
                }

                cout << "Payload complete.\n\n";
                break;
            case 5:
                cout << "How many times would you like to send this email: ";
                cin >> numMessages;

                cout << "Would you like to send your own data (Y) or garbage data (N): ";
                cin >> choice;

                cout << "Who would you like this email to be from (IP optional): ";
                cin >> username;

                //send who the mail is from and receive response
                sendMessage = "MAIL FROM:<";
                sendMessage += (username.find("@") == -1 ? username + "@" + ipAddress + ">" : username + ">"); //set what we're sending;

                //check to make sure it has an ip after the @ symbol
                if(username.find("@")+1 == username.length())
                    username.insert(username.length()-2, ipAddress);

                headerHolder.push_back(sendMessage); //get the MAIL FROM command in this vector

                //get recipient of the email
                cout << "Enter the recipient's email address (user@1.2.3.4): "; //prompts for recipient
                cin >> recipient; //get the recipient

                //send recipient of the email
                sendMessage = "RCPT TO:<" + recipient + ">"; //set what we're sending
                headerHolder.push_back(sendMessage); //put the RCPT command in this vector
                
                //send message loop
                for(int i = 0; i < numMessages; i++)
                {
                    if(toupper(choice) == 'N') //if they want garbage data
                    {
                        if(i == 0)
                        {
                            cout << "How many lines of garbage data would you like to send: ";
                            cin >> numLines;
                            if(numLines == 0)
                                numLines = 1;
                        }

                        sockClient.sendData(headerHolder[0]); //send mail from command
                        serverFlop = sockClient.recvData(recMessage); //get the response from the server

                        //check for an error
                        if(!sockClient.checkError(recMessage, Status::SMTP_ACTION_COMPLETE))
                        {
                            cout << recMessage << endl << "Trying again...\n";
                            continue; //break if we found one
                        }

                        sockClient.sendData(headerHolder[1]); //send data
                        serverFlop = sockClient.recvData(recMessage); //get response

                        //check for an error
                        if(!sockClient.checkError(recMessage, Status::SMTP_ACTION_COMPLETE))
                        {
                            cout << recMessage << endl << "Trying again...\n";
                            continue; //break if we found one
                        }

                        //send data to the server and get a response
                        sockClient.sendData("DATA"); //send that we're ready to send data
                        serverFlop = sockClient.recvData(recMessage); //get the response

                        //check for an error
                        if(!sockClient.checkError(recMessage, Status::SMTP_BEGIN_MSG))
                            continue; //break if we found one

                        for(int j = 0; j < numLines; j++)
                        {
                            Sleep(500);
                            sockClient.sendData("YOU ARE BEING TROLLED. TROLOLOL. YOU ARE BEING TROLLED. TROLOLOL."); //send data
                        }

                        sockClient.sendData("."); //send the period
                        serverFlop = sockClient.recvData(recMessage); //get response from the server

                        //check for an error
                        if(sockClient.checkError(recMessage, Status::SMTP_ACTION_COMPLETE))
                            cout << "Message sent successfully! :)\n";
                        else
                            cerr << "Error sending message. :(\n";
                    }
                    else //if they want to put in their own data
                    {
                        if(i == 0)
                        {
                            //get data from client and put it in the messageHolder vector
                            cout << "Enter data. Press '.' when the message is over.\n"; //prompt for data
                            cin.ignore(10000, '\n'); //ignore any newlines
                            //getline(cin, sendMessage); //get the message to send
                            
                            while(sendMessage != ".") //while user doesn't enter a period, keep sending data for message
                            {
                                cout << "Data > "; //prompt for data
                                getline(cin, sendMessage); //get the message to send

                                if(sendMessage == "") //check if it's an empty string, if so add a newline because a getline drops that
                                    sendMessage = "\n";

                                messageHolder.push_back(sendMessage); //put the message in our message holder vector
                            }

                            cout << "Data entered successfully. Now attempting to send this message " << numMessages << " times\n";
                        }

                        sockClient.sendData(headerHolder[0]); //send mail from command
                        serverFlop = sockClient.recvData(recMessage); //get the response from the server

                        //check for an error
                        if(!sockClient.checkError(recMessage, Status::SMTP_ACTION_COMPLETE))
                        {
                            cout << recMessage << endl << "Trying again...\n";
                            continue; //break if we found one
                        }

                        sockClient.sendData(headerHolder[1]); //send data
                        serverFlop = sockClient.recvData(recMessage); //get response

                        //check for an error
                        if(!sockClient.checkError(recMessage, Status::SMTP_ACTION_COMPLETE))
                        {
                            cout << recMessage << endl << "Trying again...\n";
                            continue; //break if we found one
                        }

                        //send data to the server and get a response
                        sockClient.sendData("DATA"); //send that we're ready to send data
                        serverFlop = sockClient.recvData(recMessage); //get the response

                        //check for an error
                        if(!sockClient.checkError(recMessage, Status::SMTP_BEGIN_MSG))
                            continue; //break if we found one

                        for(int j = 0; j < messageHolder.size(); j++)
                        {
                            Sleep(500);
                            sockClient.sendData(messageHolder[j]); //send
                        }

                        //get response after sending data and print status message for user
                        cout << "Sending data. Waiting for server...\n";
                        serverFlop = sockClient.recvData(recMessage); //get data from server

                        //check for an error
                        if(sockClient.checkError(recMessage, Status::SMTP_ACTION_COMPLETE))
                            cout << "Message " << i+1 << " sent successfully! :)\n";
                        else
                            cerr << "Error sending message " << i+1 << ". :(\n";
                    }
                }
                
                cout << "Payload complete.\n\n";
                break;
            case 6: //option 6, to quit
                //code
                cout << "Send QUIT (Y) or hard disconnect (N): ";
                cin >> choice;

                if(toupper(choice) == 'Y')
                    sockClient.sendData("QUIT");

                cout << "You chose to quit, goodbye.\n\n";
                break;
            default:
                cerr << "You entered an invalid command...\n";
                break;
        }
    }

    //close the connection
    sockClient.closeConnection();

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
