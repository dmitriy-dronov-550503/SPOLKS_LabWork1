#include "stdafx.h"
#include "FileTransport.h"
#include <fstream>

void FileTransport::Send(socket_ptr sock, string filenameFrom, string filenameTo)
{
	const uint32_t bufferSize = 16 * 1024 * 1024;
	char* data;
	data = new char[bufferSize + 1];

	if (data != nullptr)
	{
		// Open the file
		ifstream file;
		file.open(filenameFrom, ios::in | ios::binary);

		// Wait receiver ready
		sock->read_some(buffer(data, bufferSize));

		if (string(data) == "I'AM READY")
		{
			cout << "GOT READY" << endl;

			if (file.is_open())
			{
				uint32_t chunkCount = 0;
				const uint32_t maxChunkSize = bufferSize;

				// Send filesize
				ifstream fileEnd(filenameFrom, std::ifstream::ate | std::ifstream::binary);
				int64_t fileSize = fileEnd.tellg();
				string fileSizeStr = std::to_string(fileSize);
				int zeroPos = fileSizeStr.size();
				strcpy_s(data, bufferSize, fileSizeStr.c_str());
				data[zeroPos] = '\0';
				sock->write_some(buffer(data, zeroPos + 1));
				cout << "Filesize = " << data << endl;

				if (fileSize != 0)
				{
					// Record start time
					auto start = std::chrono::high_resolution_clock::now();

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

					sock->read_some(buffer(data, bufferSize));

					if (string(data) == "File received")
					{
						cout << "File has been successfully sent" << endl;
					}
				}
				else
				{
					cout << "Empty file, nothing to send" << endl;
				}
			}
			else
			{
				data[0] = '0';
				data[1] = '\0';
				sock->write_some(buffer(data, 2));
				cout << "Filesize = " << data << endl;
				cout << "Can't open file" << endl;
			}
		}
		else
		{
			cout << "Reciever isn't ready" << endl;
		}

		delete data;
	}
	else
	{
		cout << "Can't allocate send buffer" << endl;
	}
}

void FileTransport::Receive(socket_ptr sock, string filenameFrom, string filenameTo)
{
	const uint32_t bufferSize = 64 * 1024 * 1024;
	char* data;
	data = new char[bufferSize + 1];

	if (data != nullptr)
	{
		// Create the file
		ofstream myfile;
		myfile.open(filenameTo, ios::out | ios::binary);

		if (myfile.is_open())
		{
			sock->write_some(buffer("I'AM READY"));

			// Get filesize
			//--------------------------------------------------------
			sock->read_some(buffer(data, bufferSize));
			int64_t fileSize = std::stoi(data);
			cout << "Filesize = " << fileSize << endl;

			if (fileSize == 0)
			{
				cout << "Error cause on sent size" << endl;
			}
			else
			{
				// Get file content
				while (fileSize != 0)
				{
					// Get packet
					int64_t readedSize = sock->read_some(buffer(data, bufferSize));

					fileSize -= readedSize;

					//cout << "Readed " << readedSize << " left " << fileSize << '\r';

					myfile.write(data, readedSize);
				}

				myfile.close();

				sock->write_some(buffer("File received"));

				cout << "File has been successfully received" << endl;
			}
		}
		else
		{
			sock->write_some(buffer("I'AM NOT READY"));
			cout << "Can't open file" << endl;
		}

		delete data;
	}
	else
	{
		cout << "Can't allocate receive buffer" << endl;
	}
}