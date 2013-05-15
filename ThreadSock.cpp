//ThreadSock class file
//Course: 4050-212-02
//Authors: Alex Buie, Luke Matarazzo, Jackson Sadowski, Steven Tucker
//Filename: ThreadSock.cpp

#include <ctime>
#include "ThreadSock.h"

//Name: ThreadSock()
//Parameters: NONE
//Purpose: default constructor which will read in users from the text file and put it in the vector
ThreadSock::ThreadSock()
{
    ifstream fin("users.txt"); //create file input object to read users in

    if(fin.is_open()) //if the file is open, read in users
    {
        string temp = ""; //temp string which will hold the user before putting it into the users vector
        getline(fin, temp); //get the first user

        while(!fin.eof()) //while not at the end of file, keep reading in users
        {
            users.push_back(temp); //add the user to the next spot in the users vector
            getline(fin, temp); //get the next user in the file
        }
    } 
    else //if the file cannot open, don't access file and output an error
        cerr << "users.txt cannot be opened...\n";
}

//Name: ThreadSock()
//Parameters: NONE
//Purpose: blank default destructor
ThreadSock::~ThreadSock()
{

}

//Name: setSock()
//Parameters: a socket which we will set equal to the socket we have as a private attribute
//Purpose: sets our private attribute socket equal to the socket given as a parameter
//Returns: NONE
void ThreadSock::setSock(SOCKET socket)
{
    this -> socket = socket;
}

//Name: sendData()
//Parameters: string
//Purpose: takes a string from the user, turns it into a char array and sends it
//Returns: bool - returns true if sent successfully (which is technically always)
bool ThreadSock::sendData(string s)
{
    const char *buffer = s.c_str();
    send(this -> socket, buffer, strlen(buffer), 0);
    return true; //return true for success
}

//Name: sendData()
//Parameters: int
//Purpose: takes an int from the user, then calls the sendData function to send it as a char array
//Returns: bool
bool ThreadSock::sendData(int i)
{
    stringstream s;
    s << i;
    return this -> sendData(s.str());
}

//Name: sendResponse()
//Parameters: int response code that we're sending, string the human readable string to follow the code
//Purpose: sends a response code and message to a client
//Returns: bool - returns true if sent
bool ThreadSock::sendResponse(int responseCode, string message)
{
	stringstream s;
	s << responseCode << " " << message;
	this -> sendData(s.str());
}

//Name: recvData()
//Parameters: string passed by reference so we can set it equal to what we received
//Purpose: receives data and sets our string equal to it
//Returns: int - if data == -1, then they're disconnected
int ThreadSock::recvData(string &s)
{
    char buffer[1000];
    int i = recv(this -> socket, buffer, 1000, 0);
    buffer[i] = '\0';
    s = buffer;
    return i; //the value recv returns, which is the number of bytes we received, and -1 if nothing received
}

//Name: validateUser()
//Parameters: the username to check as a string
//Purpose: validates the user on the server
//Returns: bool - true if the user exists on this server and false if else
bool ThreadSock::validateUser(string user)
{
    for(int i = 0; i < this -> users.size(); i++)
        if(user == this -> users[i])
            return true;

    return false;
}

//Name: getDateTime()
//Parameters: NONE
//Purpose: gets the date and time and presents it in a readbale fashion
//Returns: the date and time separated by a comma
string ThreadSock::getDateTime()
{
    time_t rawtime; //to get the right time
    string dateTime; //string to hold date and time

    //get time
    time(&rawtime);
    char *myTime = ctime(&rawtime); //put the time
    dateTime = myTime;
    return (dateTime.substr(4, 7) + dateTime.substr(20, 4) + ", " + dateTime.substr(11, 8)); //return date and time
}

//Name: closeConnection()
//Parameters: NONE
//Purpose: closes the socket connection
//Returns: NONE
void ThreadSock::closeConnection()
{
    closesocket(socket); //close our socket
}
