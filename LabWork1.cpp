// LabWork1.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "Server.h"
#include "Client.h"
#include "CommandCenter.h"
#include <iostream>
using namespace std;
	
int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		cout << "Start server and client" << endl;
		Server server;
		Client client;

		client.Join();
		server.Join();
	}
	else
	{
		string argument(argv[1]);

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
	}

	system("pause");

    return 0;
}

