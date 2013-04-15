//ServerSocket class header file
//Course: 4050-212-02
//Authors: Luke Matarazzo and Jackson Sadowski
//Filename: ServerSocket.h

#include "Socket.h"

class ServerSocket : public Socket
{
    public:
        void Listen();
        void Bind(int port);
        void StartHosting(int port);
        void listFiles();
        void sendFile(string);
        void reply(string);
};
