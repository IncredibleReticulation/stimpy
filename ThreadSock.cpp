//ThreadSock class file
//Course: 4050-212-02
//Authors: Alex Buie, Luke Matarazzo, Jackson Sadowski, Steven Tucker
//Filename: ThreadSock.cpp

#include "ThreadSock.h"

//default constructor which will read in users from the text file and put it in the vector
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
    } else //if the file cannot open, don't access file and output an error
    {
        cerr << "users.txt cannot be opened...\n";
    }
    
}

//blank default constructor
ThreadSock::~ThreadSock()
{

}

void ThreadSock::setSock(SOCKET socket)
{
    this -> socket = socket;
}

//Name: listFiles()
//Parameters: NOT KNOWN YET
//Purpose: This function lists all of the files that are avalible for tansfer
//Returns: NOT KNOWN YET
bool ThreadSock::sendData(string s)
{
    const char *buffer = s.c_str();
    send(this -> socket, buffer, strlen(buffer), 0);
    return true; //return true for success
}

bool ThreadSock::sendData(int i)
{
    stringstream s;
    s << i;
    return this -> sendData(s.str());
}

bool ThreadSock::sendResponse(int responseCode, string message)
{
	stringstream s;
	s << responseCode << " " << message;
	this -> sendData(s.str());
}

//Name: listFiles()
//Parameters: NOT KNOWN YET
//Purpose: This function lists all of the files that are avalible for tansfer
//Returns: NOT KNOWN YET
int ThreadSock::recvData(string &s)
{
    char buffer[1000];
    int i = recv(this -> socket, buffer, 1000, 0);
    buffer[i] = '\0';
    s = buffer;
    return i; //return true for success
}

//Name: split()
//Parameters: Pointer to a vector of strings, a string that needs to be split, a delimeter to split the string with.
//Purpose: Split string according to given delimeter and add those elements to a vector.
//Returns: Number of elements added to the vector as an int.
int ThreadSock::split(vector<string>* v, string s, string del)
{
    //if the delimeter does not appear once, return 0 because string will not be split
    if(s.find(del) == -1)
    {
        v->push_back(s); //put the string in the vector so the user can still use the value they passed in
        return 0;
    }
        
    int delims = 0; //will hold number of delimeters found in the string that needs to be split

    while(s.find(del) != -1) //while there is still a delimeter present, continue to break the string apart
    {
        v->push_back(s.substr(0, s.find(del)));     //add part of string up to the delimeter to the vector
        s.erase(0, s.find(del) + 1);                //delete added part
        delims++; //increment
    }

    v->push_back(s.substr(0));  //add the rest of the string after the final delimeter

    return delims + 1;          //return number of elements added to the vector
}

//This fuction will validate life.
bool ThreadSock::validateUser(string user)
{
    for(int i = 0; i < this -> users.size(); i++)
        if(user == this -> users[i])
            return true;

    return false;
}
