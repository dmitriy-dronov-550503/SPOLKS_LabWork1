#include "stdafx.h"
#include <fstream>

#include "Server.h"
#include "LogSystem.h"
#include "FileTransport.h"
#include "SocketLow.h"

Server::Server()
{
	serverThread = new thread(&Server::ServerThread, this);
}

Server::~Server()
{
}

void Server::Join()
{
	serverThread->join();
}

void Server::ServerThread()
{
	io_service service;
	ip::tcp::endpoint ep(ip::tcp::v4(), 7850); // listen on 7850
	ip::tcp::acceptor acc(service, ep);

	isServerActive = true;

	try
	{

		while (isServerActive)
		{
			socket_ptr sock(new ip::tcp::socket(service));

			acc.accept(*sock);

			// Set KEEP_ALIVE
			boost::asio::socket_base::keep_alive keepAlive(true);
			sock->set_option(keepAlive);
			sock->set_option(ip::tcp::no_delay(true));
			//SocketLow::SetKeepAlive(sock);

			cout << log_time << "Accept client" << endl;

			ClientSession(sock, isServerActive);
		}

	}
	catch (boost::system::system_error &e)
	{
		error_code ec = e.code();
		cout << log_time << " : Exception caught in initialization " << ec.value() << " " << ec.category().name() << " " << ec.message() << endl;
	}
}

void Server::ClientSession(socket_ptr sock, bool& isServerActive)
{
	try
	{
		while (isServerActive)
		{
			const uint32_t bufferSize = 512;
			char data[bufferSize];

			size_t len = sock->read_some(buffer(data));

			if (len > 0)
			{
				cout << log_time << "Got request > " << string(data) << endl;

				for (int i = 0; i < bufferSize-1; i++)
				{
					if (data[i] == '\r' || data[i] == '\n' || data[i] == 204)
					{
						data[i+1] = '\0';
						break;
					}
				}

				ParseCommand(sock, string(data));
			}
			else
			{
				write(*sock, buffer("\r\n"));
			}
		}
	}
	catch (boost::system::system_error &e)
	{
	    error_code ec = e.code();

		switch (ec.value())
		{
		case 2:
			cout << log_time << "Client closed connection" << endl;
			break;
		default:
			cout << log_time << " : Exception caught in client session: " << ec.value() << " " << ec.category().name() << " " << ec.message() << endl;
			break;
		}

	}
}

void Server::ParseCommand(socket_ptr sock, string command)
{
	vector<string> cmds = CommandCenter::Parse(command);

	if (cmds[0] == "echo")
	{
		CmdEcho(sock, cmds);
	}
	else if (cmds[0] == "time")
	{
		CmdTime(sock, cmds);
	}
	else if (cmds[0] == "upload")
	{
		CmdReceiveFile(sock, cmds);
	}
	else if (cmds[0] == "download")
	{
		CmdSendFile(sock, cmds);
	}
	else if (cmds[0] == "halt")
	{
		isServerActive = false;
		sock->write_some(buffer("Server closed connection"));
	}
	else
	{
		write(*sock, buffer("Unrecognized command"));
	}
}

void Server::CmdEcho(socket_ptr sock, vector<string> argv)
{
	string result = "";
	const uint32_t bufferSize = 512;
	char outputBuffer[bufferSize];

	for (uint32_t i = 1; i < argv.size(); i++)
	{
		result += argv[i] + " ";
	}

	strcpy_s(outputBuffer, bufferSize, result.c_str());
	write(*sock, buffer(outputBuffer));
}

void Server::CmdTime(socket_ptr sock, vector<string> cmds)
{
	const uint32_t bufferSize = 512;
	char outputBuffer[bufferSize];

	strcpy_s(outputBuffer, bufferSize, LogSystem::CurrentDateTime().c_str());

	write(*sock, buffer(outputBuffer));
}

void Server::CmdSendFile(socket_ptr sock, vector<string> argv)
{
	FileTransport ft(sock);
	ft.Send(argv[1], argv[2]);

	// Get file ended
	sock->write_some(buffer("\r\n"));
}


void Server::CmdReceiveFile(socket_ptr sock, vector<string> argv)
{
	FileTransport ft(sock);
	ft.Receive(argv[1], argv[2]);

	// Get file ended
	sock->write_some(buffer("\r\n"));
}
