//ServerSocket class file
//Course: 4050-212-02
//Authors: Luke Matarazzo and Jackson Sadowski
//Filename: ServerSocket.cpp
//Purpose: 

#include <fstream>
#include "ServerSocket.h"
#include <dirent.h>

//Name: listFiles()
//Parameters: NOT KNOWN YET
//Purpose: This function lists all of the files that are avalible for tansfer
//Returns: NOT KNOWN YET
void ServerSocket::StartHosting(int port)
{
     Bind(port); //bind the port so we can start hosting
     Listen();  //listen
}

//Name: listFiles()
//Parameters: NOT KNOWN YET
//Purpose: This function lists all of the files that are avalible for tansfer
//Returns: NOT KNOWN YET
void ServerSocket::Listen()
{
    if(listen(mySocket, 1) == SOCKET_ERROR) //if there is an error listening on the socket print an error and end program
    {
        cerr<<"ServerSocket: Error listening on socket\n"; //error message
        WSACleanup(); //windows cleanup function
        exit(15); //ends program
    }

    acceptSocket = accept(myBackup, NULL, NULL);

    while(acceptSocket == SOCKET_ERROR)
    {
        acceptSocket = accept(myBackup, NULL, NULL);
    }
    mySocket = acceptSocket;
}

//Name: listFiles()
//Parameters: NOT KNOWN YET
//Purpose: This function lists all of the files that are avalible for transfer
//Returns: NOT KNOWN YET
void ServerSocket::Bind(int port)
{
    myAddress.sin_family = AF_INET;
    myAddress.sin_addr.s_addr = inet_addr("0.0.0.0");
    myAddress.sin_port = htons(port);

    if(bind(mySocket, (SOCKADDR*)&myAddress, sizeof(myAddress)) == SOCKET_ERROR) //if the connection fails print error and end program
    {
        cerr<<"ServerSocket: Failed to connect\n"; //error message
        WSACleanup(); //windows clean up
        exit(14); //ends program
    }
}


//Name: listFiles()
//Parameters: None
//Purpose: This function lists all of the files that are avalible for tansfer
//Returns: NOT KNOWN YET
void ServerSocket::listFiles()
{
    sendData("Listing the files...");//sending this over to the client
    
    //variables
    DIR* dir;
    dirent* pdir;
    string response = "";

    dir = opendir("files"); //open current directory

    while (pdir = readdir(dir))//continue to send the directory name while dir
    {   
        recvData(response); //waiting for the response before we send more
        
        if (response == "OK") //if the response is OK, clear to send a line of the directory
        {
            sendData(pdir->d_name);//sends another line of the directory
        } else //if the client doesn't respond print error and get out of while loop
        {
            cerr << "No response...\n";
            sendData("Ok yet?"); //send back
            //break;
        }
    }

    recvData(response); //waiting for the response before we send more
    sendData("*"); //letting the client know that there's no other directories.
    
    closedir(dir); //closing the directory
}

//Name: sendFile()
//Parameters: Going to take in the name of the file (string) for transfer
//Purpose: This function sends a file from the server to the client
//Returns: the file
void ServerSocket::sendFile(string s)
{
    //cout << "In the send file function.\n fileName: " << s << endl;
    //sendData("sending files...\n");

    s.insert(0, "files\\");
    ifstream fin(s.c_str()); //create a file input object with the filename the user gave

    if(!fin.is_open()) //if it isn't open print an error and get out of the function
    {
        sendData("Error opening file...\n"); //send that it couldn't open
        return; //end function
    } else
        sendData(s); //sends the name of the file to the server

    //if the file opened successfully, start sending stuff
    string response = ""; //will the client response
    string nextLine = ""; //will hold each line of text from the text file to send over
    getline(fin, nextLine); //get line of text

    while(!fin.eof()) //while not at the end of file, keep reading lines and sending them
    {
        recvData(response); //get the data

        if(response == "OK") //if the client sends the OK
        {
            sendData(nextLine); //send line to client

            getline(fin, nextLine); //get the next line
        } else //if the client doesn't respond print error and get out of while loop
        {
            cerr << "\n\tno response while sending file...\n"; //print error on server
            sendData("Ok yet?"); //send back
            //break; //break from while
        }
    }

    recvData(response);
    sendData("EOFEOFEOFEOFEOF"); //send EOF indicator to client
}

//Name: reply()
//Parameters: string
//Purpose: Reply takes a command from the client. Based on what the command does, it will perform different tasks.
//Returns: Nothing
void ServerSocket::reply(string s)
{
    vector<string> args; //create a vector to hold arguments the client sent

    split(&args, s, " "); //split the line they sent by a space

    if(args[0] == "LIST") //if they said list
    {
        listFiles(); //list the files
    } else if(args[0] == "SEND") //if they said send
    {
        if(args.size() < 2) //if the size isn't greater than 2, send error
            sendData("Error - you must enter a file name to transfer...\n"); //send error message to client
        else //if it was >= 2 then send the file
            sendFile(args[1]); //send the file they requested
    } else //anything else is an error so send an error to client
    {
        sendData("\n\tError - unrecognized command...\n\tAcceptable commands are 'LIST', 'SEND', and 'QUIT'\n"); //error message
    }
}