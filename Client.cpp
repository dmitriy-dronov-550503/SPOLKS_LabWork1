#include "stdafx.h"
#include <fstream>
#include <chrono>

#include "Client.h"
#include "CommandCenter.h"
#include "LogSystem.h"

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
	if (ec)
	{
		cout << "error_code: " << ec << endl;
	}
	service.run();

	boost::asio::socket_base::keep_alive keepAlive(true);
	sock->set_option(keepAlive);

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
					UploadFile(sock, cmds);
				}

				sock->read_some(buffer(data));
				cout << data << endl;
			}

			if (str == "exit") break;
		}
	}
	catch (boost::system::system_error &e)
	{
		error_code ec = e.code();
		cout << LogSystem::CurrentDateTime() << "Exception caught: " << ec.value() << " " << ec.category().name() << " " << ec.message().c_str();
	}
}

void Client::UploadFile(socket_ptr sock, vector<string> argv)
{
	const uint32_t bufferSize = 16 * 1024 * 1024;
	char* data;
	data = new char[bufferSize + 1];

	sock->read_some(buffer(data, bufferSize));

	if (string(data) == "I'AM READY")
	{
		cout << "Server's ready to get file" << endl;
	}

	// Write file here
	//--------------------------------------------------------
	
	// Send filename
	strcpy_s(data, bufferSize, argv[2].c_str());
	write(*sock, buffer(data, bufferSize));

	// Open the file
	ifstream file;
	file.open(argv[1], ios::in | ios::binary);

	if (file.is_open())
	{
		// Record start time
		auto start = std::chrono::high_resolution_clock::now();
		uint32_t chunkCount = 0;
		const uint32_t maxChunkSize = bufferSize;

		// Send file content
		while (true)
		{
			file.read(data, maxChunkSize);
	
			uint32_t packetSize = file.gcount();

			// Send packet
			size_t sendedSize = sock->write_some(buffer(data, packetSize));

			if (packetSize < maxChunkSize)
			{
				sock->write_some(buffer("THIS IS THE END", 0));
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

	delete data;
}
