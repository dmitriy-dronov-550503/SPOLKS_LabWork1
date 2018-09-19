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
	else if (cmds[0] == "download")
	{
		CmdSendFile(sock, cmds);
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

void Server::CmdReceiveFile(socket_ptr sock, vector<string> cmds)
{
	const uint32_t bufferSize = 64 * 1024 * 1024;
	char* data;
	data = new char[bufferSize + 1];

	sock->write_some(buffer("I'AM READY"));

	// Get file here
	//--------------------------------------------------------

	// Get filename
	sock->read_some(buffer(data, bufferSize));
	cout << "Upload to the file " << data << endl;

	// Create the file
	ofstream myfile;
	myfile.open(data, ios::out | ios::binary);
	
	int64_t fileSize;
	sock->read_some(buffer(&fileSize, sizeof(int64_t)));
	cout << "Filesize = " << fileSize << endl;

	if (myfile.is_open())
	{
		// Get file content
		while (true)
		{
			// Get packet
			int64_t readedSize = sock->read_some(buffer(data, bufferSize));

			fileSize -= readedSize;

			//cout << "Readed " << readedSize << " left " << fileSize << '\r';

			myfile.write(data, readedSize);

			if (fileSize == (int64_t)0)
			{
				break;
			}
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

void Server::CmdSendFile(socket_ptr sock, vector<string> argv)
{
	const uint32_t bufferSize = 16 * 1024 * 1024;
	char* data;
	data = new char[bufferSize + 1];

	sock->read_some(buffer(data, bufferSize));

	if (string(data) == "I'AM READY")
	{
		cout << "Client is ready to get file" << endl;
	}

	// Write file here
	//--------------------------------------------------------

	// Open the file
	ifstream file;
	file.open(argv[1], ios::in | ios::binary);

	if (file.is_open())
	{
		// Record start time
		auto start = std::chrono::high_resolution_clock::now();
		uint32_t chunkCount = 0;
		const uint32_t maxChunkSize = bufferSize;

		ifstream fileEnd(argv[1], std::ifstream::ate | std::ifstream::binary);
		int64_t fileSize = fileEnd.tellg();
		sock->write_some(buffer(&fileSize, sizeof(int64_t)));
		cout << "Filesize = " << fileSize << endl;

		// Send file content
		while (true)
		{
			file.read(data, maxChunkSize);

			uint32_t packetSize = file.gcount();

			// Send packet
			size_t sendedSize = sock->write_some(buffer(data, packetSize));

			//cout << "Sended " << sendedSize << endl;

			if (packetSize < maxChunkSize)
			{
				break;
			}

			chunkCount++;
		}

		// Record end time
		auto finish = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed = finish - start;
		std::cout << "Average speed was: " << ((chunkCount * maxChunkSize) / elapsed.count()) / (1024 * 1024) << " MB/s\n";

		file.close();
	}
	else cout << "Unable to open file" << endl;
	//--------------------------------------------------------

	sock->read_some(buffer(data, bufferSize));

	if (string(data) == "File received")
	{
		cout << "File has been successfully sent" << endl;
	}

	// Get file ended
	sock->write_some(buffer("\r\n"));

	delete data;
}
