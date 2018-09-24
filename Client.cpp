#include "stdafx.h"
#include <fstream>
#include <chrono>

#include "Client.h"
#include "CommandCenter.h"
#include "LogSystem.h"
#include "FileTransport.h"
#include "SocketLow.h"

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
	socket_ptr sock(new ip::tcp::socket(service));
	//sock->async_connect(ep, &Client::ConnectionHandler);
	boost::system::error_code ec;
	sock->connect(ep, ec);

	if (!ec)
	{
		service.run();

		// Set KEEP_ALIVE
		boost::asio::socket_base::keep_alive keepAlive(true);
		sock->set_option(keepAlive);
		SocketLow::SetKeepAlive(sock);

		char data[512];


		try
		{
			while (true)
			{
				string str;
				cout << endl << "> ";
				getline(cin, str);

				if (!str.empty())
				{
					vector<string> cmds = CommandCenter::Parse(str);

					strcpy_s(data, str.c_str());

					sock->write_some(buffer(data));

					if (cmds[0] == "upload") {
						SendFile(sock, cmds);
					}
					else if (cmds[0] == "download") {
						ReceiveFile(sock, cmds);
					}

					sock->read_some(buffer(data));
					cout << data << endl;
				}

				if (str == "exit" || str == "halt") break;
			}
		}
		catch (boost::system::system_error &e)
		{
			error_code ec = e.code();
			cout << LogSystem::CurrentDateTime() << "Exception caught: " << ec.value() << " " << ec.category().name() << " " << ec.message().c_str() << endl;
		}
	}
	else
	{
		cout << "Can't connect. Error code " << ec << endl;
	}
}

void Client::SendFile(socket_ptr sock, vector<string> argv)
{
	FileTransport ft(sock);
	ft.Send(argv[1], argv[2]);
}

void Client::ReceiveFile(socket_ptr sock, vector<string> argv)
{
	FileTransport ft(sock);
	ft.Receive(argv[1], argv[2]);
}
