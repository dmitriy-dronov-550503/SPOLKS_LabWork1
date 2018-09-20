#include "stdafx.h"
#include "FileTransport.h"
#include <fstream>

void FileTransport::Send(socket_ptr sock, string filenameFrom, string filenameTo)
{
	const uint32_t bufferSize = 16 * 1024 * 1024;
	char* data;
	data = new char[bufferSize + 1];

	sock->read_some(buffer(data, bufferSize));

	if (string(data) == "I'AM READY")
	{
		cout << "GOT READY" << endl;
	}

	// Write file here
	//--------------------------------------------------------

	// Open the file
	ifstream file;
	file.open(filenameFrom, ios::in | ios::binary);

	if (file.is_open())
	{
		// Record start time
		auto start = std::chrono::high_resolution_clock::now();
		uint32_t chunkCount = 0;
		const uint32_t maxChunkSize = bufferSize;

		ifstream fileEnd(filenameFrom, std::ifstream::ate | std::ifstream::binary);
		string fileSize = std::to_string(fileEnd.tellg());
		cout << "String filesize = " << fileSize << endl;
		int zeroPos = fileSize.size();
		strcpy_s(data, bufferSize, fileSize.c_str());
		data[zeroPos] = '\0';
		sock->write_some(buffer(data, zeroPos+1));
		cout << "Filesize = " << data << endl;

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

	delete data;
}

void FileTransport::Receive(socket_ptr sock, string filenameFrom, string filenameTo)
{
	const uint32_t bufferSize = 64 * 1024 * 1024;
	char* data;
	data = new char[bufferSize + 1];

	sock->write_some(buffer("I'AM READY"));

	// Get file here
	//--------------------------------------------------------

	// Create the file
	ofstream myfile;
	myfile.open(filenameTo, ios::out | ios::binary);

	sock->read_some(buffer(data, bufferSize));
	cout << "Receive filisize = " << data << endl;
	int64_t fileSize = std::stoi(data);
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

	cout << "File has been successfully received" << endl;
	
	sock->write_some(buffer("File received"));

	//--------------------------------------------------------
	// Get file ended

	delete data;
}