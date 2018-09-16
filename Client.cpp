#include "stdafx.h"
#include "Client.h"


Client::Client(string ipAddress)
{
	clientThread = new thread(&Client::ClientThread, ipAddress);
}

Client::~Client()
{
}

void Client::Join()
{
	clientThread->join();
}

void Client::ClientThread(string ipAddress)
{
	cout << "GOT IP: " << ipAddress << endl;

	io_service service;
	ip::tcp::endpoint ep(ip::address::from_string(ipAddress.c_str()), 2001);
	ip::tcp::socket sock(service);
	sock.async_connect(ep, &Client::ConnectionHandler);
	service.run();

	char data[512];
	while (true)
	{
		string str;
		cout << endl << "> ";
		getline(cin, str);
		
		if (!str.empty())
		{
			strcpy_s(data, str.c_str());

			sock.write_some(buffer(data));
			sock.read_some(buffer(data));
			cout << data << endl;
		}

		if (str == "exit") break;
	}
}

void Client::ConnectionHandler(const boost::system::error_code & ec)
{
	cout << "error_code: " << ec << endl;
}
