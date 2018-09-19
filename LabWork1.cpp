// LabWork1.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "Server.h"
#include "Client.h"
#include "CommandCenter.h"
#include <iostream>
#include <string>
using namespace std;
	
int main(int argc, char* argv[])
{
	string argument = "";

	if (argc != 1)
	{
		argument = argv[1];
	}
	else
	{
		cout << "-s server -c client: ";
		getline(cin, argument);
	}

	if (argument == "-s")
	{
		cout << "Start server" << endl;
		Server server;
		server.Join();
	}

	if (argument == "-c")
	{
		cout << "Start client" << endl;

		string ipAddress = "127.0.0.1";

		if (argc == 3)
		{
			ipAddress = argv[2];
		}

		Client client(ipAddress);
		client.Join();
			
	}

    return 0;
}

