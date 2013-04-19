#include "ServerSocket.h"
#include "Status.h"

//for writing a new message
void ServerSocket::sendMail(helostring)
{

	ofstream fout;
	vector<string> recipients;

	//checking out the string to see if it's helo
	if (helostring.substr(0,4) == "HELO")//if the first word is helo
	{
		//send back 250 that it's good
		sendData(Staus::SMTP_ACTION_COMPLETE);

		//if it's not HELO, return error code
		if (helostring.substr(0,4) != "HELO")
		{
			sendData(Status::SMTP_CMD_SNTX_ERR);//sending the error code
		}

	}

	
	//recieving the verify and a username
	recvData(verify);

	//checking to see if it's a verify
	if (verify.substr(0,4) == "VRFY")
	{
		//if it is, validate the username and continue
		if (this->validateUser(verify.substr(5)))
		{
			//if the username was valid, send back 250
			sendData(Staus::SMTP_ACTION_COMPLETE);
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

	//checking to see if it's RCPT TO
	if (toaddress.substr(0,7) == "RCPT TO")
	{
		//checking to see if the user is valid
		if (this->validateUser(verify.substr(9)))
		{
			//if the username was valid, send back 250
			sendData(Staus::SMTP_ACTION_COMPLETE);
		}

		//sending back a bad error code
		if (!this->validateUser(verify.substr(9)))
		{
			sendData(Status::SMTP_CMD_SNTX_ERR);
		}

		//putting the usernames into the vector
		ServerSock.recipients.push_back(verify.substr(9));

	} while (toaddress.substr(0,7) == "RCPT TO");







	//getting a string from the client
	recvData(fromaddress);

	if (fromaddress.substr(0,9) == "MAIL FROM")//if the first word is helo
	{
		sendData("250 OK\n");

		if (fromaddress.substr(0,9) != "MAIL FROM")
		{
			return Status::SMTP_CMD_SNTX_ERR;
		}

	}


	//getting a string from the client
	recvData(data);

	if (data.substr(0,6) == "DATA")//if the first word is helo
	{
		sendData("354 End data with <CR><LF>.<CR><LF>");

		if (data.substr(0,6) != "DATA")
		{
			return Status::SMTP_CMD_SNTX_ERR;
		}

	}

	//while line !=. we want to keep getting input from the user
	while (true)
	{
		
		//getting a line from the user
		recvData(line);

		//checking to see if it should break
		if (line == ".")
		{
			sendData("250 Ok: queued as PUT THREAD HERE!!!!!!");
			//close the connection and return the stmp code and break
		}


		fout.open ("emailfile.txt", ios::app);
		fout << line;
		
	}

	//closing the file
	fout.close();

}


