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
        username = trim(username);

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
        int messageCount = 0; //will count how many messages we have received. for formatting purposes
        int lineCount = 0; //will count how many lines per message were sent. needed to know when to data is being sent
        string messBuf = ""; //will hold the entire message and act as a buffer before printing it

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
                cout << "Enter the recipient's email address (username@123.123.123.123): "; //prompts for recipient
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

                    if(sendMessage != ".")
                        sendMessage = sockClient.encrypt(sendMessage); //encrypt the message before we send it to the server

                    sockClient.sendData(sendMessage); //send the data
                }

                //get response after sending data and print status message for user
                cout << "Sending data. Waiting for server...\n";
                serverFlop = sockClient.recvData(recMessage); //get data from server

                //check for an error
                if(sockClient.checkError(recMessage, Status::SMTP_ACTION_COMPLETE))
                    cout << "Message sent successfully! :)\n\n";
                else
                    cerr << "Error sending message. Please retry in a few minutes. :(\n\n";

                break; //break from case
            case 2: //option 2, to read messages in the user's mailbox
                sockClient.sendData("INBOX"); //send the inbox command
                sockClient.recvData(recMessage); //await a reply with a status code from the server


                //check for an error; should be a 250
                if(!sockClient.checkError(recMessage, Status::SMTP_ACTION_COMPLETE))
                    break; //break if we found an error

                //print a newline for formatting
                cout << endl << "Your mailbox\n\n";
                sockClient.sendData("OK");
                sockClient.recvData(recMessage); //get the first part of the email message
                
                while(recMessage != "EOF")
                {
                    if(recMessage != "EOF")
                        sockClient.sendData("OK");
                    
                    lineCount++;
                    if(recMessage == ".") //if the last message sent was a period, that's the entire email and we can print it out
                    {
                        if(messageCount > 0) //if this isn't the first message, print a message separator
                            cout << "\n**********************************************\n\n";

                        messageCount++; //increment message count
                        lineCount = 0; //reset line count

                        //print out the message and reset the message buffer string
                        cout << messBuf << endl;
                        messBuf = "";
                    } else
                    {
                        switch(lineCount)
                        {
                            case 1: //timestamp
                                messBuf += "Time: " + recMessage + "\n"; //add what the server sent to the message buffer string
                                break;
                            case 2: //to
                                messBuf += "To: " + recMessage + "\n"; //add what the server sent to the message buffer string
                                break;
                            case 3: //from
                                messBuf += "From: " + recMessage + "\n\n"; //add what the server sent to the message buffer string
                                break;
                            default: //data/message body
                                messBuf += sockClient.decrypt(recMessage); //decrypt cuz we're at the data part

                                if(recMessage != "\n") //if it's not a newline character, add one
                                    messBuf += "\n";
                                break;
                        }
                        // if(lineCount < 4) //if we're not at the data part yet, don't decrypt
                        //     messBuf += recMessage; //add what the server sent to the message buffer string
                        // else
                        //     messBuf += sockClient.decrypt(recMessage); //decrypt cuz we're at the data part
                    }

                    sockClient.recvData(recMessage); //get the next part of the email message

                }



        //         while(recMessage != ".") //while the server doesn't send a single period, keep getting and outputting email messages
        //         {
                   
		    		// //split the message into the parts we need
		    		// vector<string> message;
		    		// sockClient.split(&message, recMessage, ",\"");
	
        //             //print information
        //             cout << "Time: " << message[0].substr(1, message[0].length()-2) << endl; //print timestamp with date
        //             cout << "To: " << message[1].substr(1, message[1].length()-2) << endl; //print who the message was to
        //             cout << "From: " << message[2].substr(1, message[2].length()-2) << endl; //print who the message was from
        //             cout << "Message Body: \n" << sockClient.decrypt(message[3].substr(1, message[3].length()-2)); //decrypt & print the message

        //             sockClient.recvData(recMessage); //get the next email message
        //         }
				
                cout << "End of the inbox!\n\n"; //letting the client know it's the end of their inbox
                break;
            case 3: //option 3, to quit
                //code
                cout << "You chose to quit, goodbye.\n\n";
                sockClient.sendData("QUIT"); //send quit to the server so it knows we're disconnecting
                //sockClient.recvData(recMessage); //get the final message
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
