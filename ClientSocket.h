//ClientSocket class header file
//Course: 4050-212-02
//Authors: Alex Buie, Luke Matarazzo, Jackson Sadowski, Steven Tucker
//Filename: ClientSocket.h

#include "Socket.h"

class ClientSocket : public Socket
{
    public:
        void connectToServer(const char*, int); //connects to server
        bool checkError(string, int); //check for errors
        // void getFile();
        // void listFiles();
};
