//server side inbox
string line;
string fileName = username + ".txt";//concatinating the filename to look for
ifstream myfile(fileName);
  
if (myfile.is_open())
{
	cout << "Found a file to send... sending now.";

	getline (myfile,line);

	while (!myfile.eof())
	{
	  current_client.sendData(line);
	  getline (myfile,line);
	}

	myfile.close();
}

else
{
	cerr << "There are no messages to send to the client.";
}


//client side inbox
sockClient.recvData(recMessage);//looking for the 250

if (recMessage != 250)
{
	cerr << "ERRRRRRROR";
}

else
{
	while (recMessage != ERROR STATUS CODE HERE)
	{
		sockClient.decrypt(recMessage);
		cout << recMessage << endl;
	}
}


