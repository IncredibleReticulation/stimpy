#include "Status.h"

//for writing a new message
void ServerSocket::sendMail(helostring)
{

	ofstream fout;

	if (helostring.substr(0,4) == "HELO")//if the first word is helo
	{
		sendData("250 Hello emailaddress, I am glad to meet you.\n");

		if (helostring.substr(0,4) != "HELO")
		{
			return Status::SMTP_CMD_SNTX_ERR;
		}

	}

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


