#include "stdafx.h"
#include <fstream>

#include "Server.h"
#include "LogSystem.h"

Server::Server()
{
	serverThread = new thread(&Server::ServerThread);
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
	ip::tcp::endpoint ep(ip::tcp::v4(), 2001); // listen on 2001
	ip::tcp::acceptor acc(service, ep);

	bool isServerActive = true;

	try
	{

		while (isServerActive)
		{
			socket_ptr sock(new ip::tcp::socket(service));

			acc.accept(*sock);

			// Setup KEEP_ALIVE option
			boost::asio::socket_base::keep_alive keepAlive(true);
			sock->set_option(keepAlive);

			LogSystem::Log("Accept client");

			ClientSession(sock, isServerActive);
		}

	}
	catch (boost::system::system_error &e)
	{
		error_code ec = e.code();
		cout << LogSystem::CurrentDateTime() << "Exception caught in initialization " << ec.value() << " " << ec.category().name() << " " << ec.message();
	}
}



void Server::ClientSession(socket_ptr sock, bool& isServerActive)
{
	try
	{
		while (true)
		{
			char data[512];

			size_t len = sock->read_some(buffer(data));

			if (len > 0)
			{
				LogSystem::Log("Got request > " + string(data));

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
			LogSystem::Log("Client closed connection");
			isServerActive = false;
			break;
		default:
			cout << LogSystem::CurrentDateTime() << "Exception caught in client session: " << ec.value() << " " << ec.category().name() << " " << ec.message();
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
	else
	{
		write(*sock, buffer("Unrecognized command"));
	}
}

void Server::CmdEcho(socket_ptr sock, vector<string> argv)
{
	string result = "";
	char outputBuffer[512];

	for (uint32_t i = 1; i < argv.size(); i++)
	{
		result += argv[i] + " ";
	}

	strcpy_s(outputBuffer, result.c_str());
	write(*sock, buffer(outputBuffer));
}

void Server::CmdTime(socket_ptr sock, vector<string> cmds)
{
	char outputBuffer[512];

	strcpy_s(outputBuffer, LogSystem::CurrentDateTime().c_str());

	write(*sock, buffer(outputBuffer));
}

void Server::CmdReceiveFile(socket_ptr sock, vector<string> cmds)
{
	const uint32_t bufferSize = 20 * 1024;
	char* data;
	data = new char[bufferSize];

	sock->write_some(buffer("I'AM READY"));

	// Get file here
	//--------------------------------------------------------

	// Get filename
	sock->read_some(buffer(data, bufferSize));
	cout << "Upload to the file " << data << endl;

	// Create the file
	ofstream myfile;
	myfile.open(data, ios::out | ios::binary);
	
	if (myfile.is_open())
	{
		// Get file content
		while (true)
		{
			// Get packet
			sock->read_some(buffer(data, bufferSize));

			uint32_t packetSize = *((uint32_t*)data);
			char* payload = data + 4;

			if (packetSize == 0)
			{
				break;
			}

			//cout << "Get packet with size " << packetSize << endl;

			myfile.write(payload, packetSize);

			sock->write_some(buffer("GOT"));
		}

		myfile.close();
	}
	else
	{
		cout << "Can't open file" << endl;
	}
	

	//--------------------------------------------------------
	// Get file ended
	write(*sock, buffer("\r\n"));

	delete data;
}
