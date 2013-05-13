//ThreadSock class file
//Course: 4050-212-02
//Authors: Alex Buie, Luke Matarazzo, Jackson Sadowski, Steven Tucker
//Filename: ThreadSock.cpp

#include <ctime>
#include "ThreadSock.h"


//Name: ThreadSock()
//Parameters: NONE
//Purpose: default constructor which will read in users from the text file and put it in the vector
//Returns: NONE
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
//Purpose: basic constructor
//Returns: NONE
ThreadSock::~ThreadSock()
{
    //constructor    
}

void ThreadSock::setSock(SOCKET socket)
{
    this -> socket = socket;
}

//Name: sendData()
//Parameters: string
//Purpose: takes a string from the user, turns it into a char array and sends it
//Returns: bool - returns true if 
bool ThreadSock::sendData(string s)
{
    const char *buffer = s.c_str();
    send(this -> socket, buffer, strlen(buffer), 0);
    return true; //return true for success
}

//Name: sendData()
//Parameters: string
//Purpose: takes a int from the user, then calls the senData function to send it as a char array
//Returns: bool
bool ThreadSock::sendData(int i)
{
    stringstream s;
    s << i;
    return this -> sendData(s.str());
}

//Name: sendResponse()
//Parameters: int, string
//Purpose: sends a response code and message to a client
//Returns: bool - returns true if sent
bool ThreadSock::sendResponse(int responseCode, string message)
{
	stringstream s;
	s << responseCode << " " << message;
	this -> sendData(s.str());
}

//Name: recvData()
//Parameters: string
//Purpose: recieves data and changes it to a char array
//Returns: int - if data == -1, then they're disconnected
int ThreadSock::recvData(string &s)
{
    char buffer[1000];
    int i = recv(this -> socket, buffer, 1000, 0);
    buffer[i] = '\0';
    s = buffer;
    return i; //return true for success
}

//Name: validateUser()
//Parameters: string
//Purpose: validates the user on the server
//Returns: bool - true if the user exists
bool ThreadSock::validateUser(string user)
{
    for(int i = 0; i < this -> users.size(); i++)
        if(user == this -> users[i])
            return true;

    return false;
}

//Name: getDateTime()
//Parameters: string
//Purpose: gets the date and time and presents it in a readbale fashion
//Returns: bool - true if the user exists
string ThreadSock::getDateTime()
{
    time_t rawtime; //to get the right time
    string date, time; //strings for date and time
    string temp; //will hold the date for easier manipulation

    //get time
    std::time(&rawtime);
    char *myTime = ctime(&rawtime); //put the time

    temp = myTime;
    date = temp.substr(4, 7);
    date += temp.substr(20, 4);
    time = temp.substr(11, 8);

    return (date + ", " + time);
}

//Name: closeConnection()
//Parameters: NONE
//Purpose: closes the connection
//Returns: NONE
void ThreadSock::closeConnection()
{
    closesocket(socket); //close our socket
}
