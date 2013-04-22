//ClientSocket class file
//Course: 4050-212-02
//Authors: Luke Matarazzo and Jackson Sadowski
//Filename: ClientSocket.cpp
//Purpose: 

#include <fstream>
#include "ClientSocket.h"

//Name: listFiles()
//Parameters: NOT KNOWN YET
//Purpose: This function lists all of the files that are avalible for tansfer
//Returns: NOT KNOWN YET
void ClientSocket::connectToServer(const char *ipAddress, int port)
{
    //populate values in the struct
    myAddress.sin_family = AF_INET;
    myAddress.sin_addr.s_addr = inet_addr(ipAddress);
    myAddress.sin_port = htons(port);

    if (connect(mySocket,(SOCKADDR*)&myAddress, sizeof(myAddress)) == SOCKET_ERROR) //if there is an error connecting, print error and end program
    {
        cerr<<"ClientSocket: Failed to connect\n"; //error message
        WSACleanup(); //windows cleanup function
        exit(13); //ends program
    }
}

bool ClientSocket::checkError(string message, int error)
{
    if(atoi(message.substr(0,3).c_str()) != error)
    {
        cerr << "Something went wrong. Please try again.\n"; //error message
        return false;
    }

    return true;
}



// void ClientSocket::getFile()
// {
//     string line = ""; //will hold name of file being received
//     recvData(line); //get the file name
//     //cout << "\t\t" << line;
//     string temp = line.substr(0, line.find(' ')); //get the first part of the string up to a space

//     if(temp == "Error") //if they server sends us an error, print that and end the function
//     {
//         cerr << "\n\t" << line << endl; //error message
//         return; //return to end function
//     }

//     sendData("OK"); //send OK that we got the first line

//     ofstream fout; //open a file output object with the file name the server sent
//     fout.open(line.c_str()); //open the file

//     recvData(line); //get the data
//     //cout << "\t\t" << line;

//     //while not at the end of the file, keep getting data from the server and writing it to the file
//     while(line != "EOFEOFEOFEOFEOF")
//     {
//         //write to file
//         fout << line << endl;

//         sendData("OK"); //send OK that we got the line of text

//         //get the next line of data
//         recvData(line);
//         //cout << "\t\t" << line;
//     }

//     fout.close(); //close the file after we're done writing to it
//     //sendData("OK"); //send that wee got it all okay
//     cout << "File written successfully\n\n";
// }

// void ClientSocket::listFiles()
// {
//     string line = ""; //will hold the each line the server sends
//     recvData(line); //get the first line of text

//     sendData("OK"); //say okay that we got the message that the server is listing files now
//     recvData(line); //get the next thing the server sends

//     while(line != "*") //while it isn't *, keep getting data and printing it out
//     {
//         //print the line sent from server
//         cout << "\t" << line << endl;

//         sendData("OK"); //send the OK that we got the line

//         recvData(line); //get the next line of text
//     }
// }