#include "stdafx.h"
#include "FileTransport.h"
#include <fstream>

int64_t FileTransport::WriteFileSize(char* data, string fileName)
{
	ifstream file(fileName, std::ifstream::ate | std::ifstream::binary);
	int64_t fileSize = file.tellg();
	file.close();
	string fileSizeStr = std::to_string(fileSize);
	int zeroPos = fileSizeStr.size();
	strcpy_s(data, sendBufferSize, fileSizeStr.c_str());
	data[zeroPos] = '\0';
	return fileSize;
}

void FileTransport::Send(socket_ptr sock, string filenameFrom, string filenameTo)
{
	
	char* data;
	data = new char[sendBufferSize + 1];

	if (data != nullptr)
	{
		// Open the file
		ifstream file;
		file.open(filenameFrom, ios::in | ios::binary);

		// Wait receiver ready
		sock->read_some(buffer(data, sendBufferSize));

		if (string(data) == "I'AM READY")
		{
			cout << "GOT READY" << endl;

			if (file.is_open())
			{
				uint32_t chunkCount = 0;
				const uint32_t maxChunkSize = sendBufferSize;

				// Send filesize
				ifstream file2(filenameFrom, std::ifstream::ate | std::ifstream::binary);
				int64_t fileSize = file2.tellg();
				file2.close();
				string fileSizeStr = std::to_string(fileSize);
				int zeroPos = fileSizeStr.size();
				strcpy_s(data, sendBufferSize, fileSizeStr.c_str());
				data[zeroPos] = '\0';
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

					sock->read_some(buffer(data, sendBufferSize));

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
	char* data;
	data = new char[receiveBufferSize + 1];

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
			sock->read_some(buffer(data, receiveBufferSize));
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
					int64_t readedSize = sock->read_some(buffer(data, receiveBufferSize));

					fileSize -= readedSize;

					//cout << "Readed " << readedSize << " left " << fileSize << endl;

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