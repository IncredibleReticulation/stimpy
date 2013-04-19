//ServerSocket class header file
//Course: 4050-212-02
//Authors: Luke Matarazzo and Jackson Sadowski
//Filename: ServerSocket.h

#include "Socket.h"

class ServerSocket : public Socket
{
   	private:
   		vector<string> users; //holds all of the local usernames

    public:
        void Listen();
        void Bind(int);
        void StartHosting(int);
        void listFiles();
        void sendFile(string);
        void reply(string);
        bool validateUser(string);
};
