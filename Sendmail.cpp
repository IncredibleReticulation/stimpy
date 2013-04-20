#include "ServerSocket.h"
#include "Status.h"

//for writing a new message
void ServerSocket::sendMail(helostring)
{
	//variables
    ofstream fout;
    vector<string> recipients;

    //checking out the string to see if it's helo
    if (helostring.substr(0,4) == "HELO")//if the first word is helo
    {
        sendData(Staus::SMTP_ACTION_COMPLETE);//send back 250 that it's good

        //if it's not HELO, return error code
        if (helostring.substr(0,4) != "HELO")
        {
            sendData(Status::SMTP_CMD_SNTX_ERR);//sending the error code
        }

    }

    recvData(verify);//recieving the verify and a username

    //checking to see if it's a verify
    if (verify.substr(0,4) == "VRFY")
    {
        //if it is, validate the username and continue
        if (this->validateUser(verify.substr(5)))
        {
            sendData(Staus::SMTP_ACTION_COMPLETE);//if the username was valid, send back 250
        }

        //sending back a bad error code
        if (!this->validateUser(verify.substr(5)))
        {
            sendData(Status::SMTP_CMD_SNTX_ERR);
        }

    }

    //at this point, we are going to check for multiple recipt to
    //Looping over the next function
    //It keeps looping until it is not a recipt to, then breaks out

    //getting the rcptto from the client
    recvData(toaddress);

    do //going to loop to add people to the vector
    {
        //checking to see if it's RCPT TO
        if (toaddress.substr(0,6) == "RCPT TO")
        {
            //checking to see if the user is valid
            if (this->validateUser(verify.substr(9)))
            {
                sendData(Staus::SMTP_ACTION_COMPLETE);//if the username was valid, send back 250
            }

            //sending back a bad error code
            else if (!this->validateUser(verify.substr(9)))
            {
                sendData(Status::SMTP_CMD_SNTX_ERR);
            }

            ServerSock.recipients.push_back(verify.substr(9));//putting the usernames into the vector

            recvData(toaddress);//getting the rcptto from the client
        } 

    } while (toaddress.substr(0,6) == "RCPT TO"); //it's going to keep getting users and break when it's not RCPT TO

    //doing this for readablility -- we can change this later
    string data;
    data = toaddress;

    //checking to see if the string is DATA 
    if (data.substr(0,6) == "DATA")
    {
        //if not, return an error code
        if (data.substr(0,6) != "DATA")
        {
            sendData(Status::SMTP_CMD_SNTX_ERR);//sending and error code back
        }

    }

    fout.open ("fout.txt", ios::app);//opening the file
    
    //while line !=. we want to keep getting input from the user
    while (true)
    {
        recvData(line);//getting a line from the user

        //checking to see if the line should be added
        if (line != ".")
        {
            fout << line;
        }
        
        //if they send a period, then we want to send back status number and quit
        else
        {
            sendData(SMTP_ACTION_COMPLETE);//sending the status code back
            break;
        }

    }

    fout.close();//closing the file

}


