#include "stdafx.h"
#include "FileTransport.h"
#include <fstream>

#include "LogSystem.h"

FileTransport::FileTransport(socket_ptr sock_in)
{
	sock = sock_in;
}

void ShowSpeed(bool& isActive, time_point<steady_clock> start, uint32_t& chunkCount, uint32_t sendChunkSize)
{
	while (isActive)
	{
		std::cout << '\r';
		auto finish = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed = finish - start;
		std::cout << "Average speed: " << ((chunkCount * sendChunkSize) / elapsed.count()) / (1024 * 1024) << " MB/s";
		std::this_thread::sleep_for(100ms);
	}
}

void FileTransport::Send(string filenameFrom, string filenameTo)
{
	data = new char[sendBufferSize + 1];

	if (data != nullptr)
	{
		sock->read_some(buffer(data, sendBufferSize));
		int64_t downloadedSize = std::stoi(data);
		cout << "Download stopped at = " << downloadedSize << endl;
		sock->write_some(buffer("GOT FILESIZE"));

		// Open the file
		file.open(filenameFrom, ios::in | ios::binary);

		// Wait receiver ready
		sock->read_some(buffer(data, sendBufferSize));

		if (string(data) == "I'AM READY")
		{
			cout << "GOT READY" << endl;

			if (file.is_open())
			{
				// Seek
				file.seekg(downloadedSize);
				cout << "Start upload from " << downloadedSize << endl;
				// Send filesize
				int64_t fileSize = SendFileSize(filenameFrom);
				cout << "Filesize = " << data << endl;

				if (fileSize != 0)
				{
					chunkCount = 0;
					isShowSpeed = true;
					start = std::chrono::high_resolution_clock::now();
					speedThread = new thread(ShowSpeed, std::ref(isShowSpeed), start, std::ref(chunkCount), sendChunkSize);

					try
					{
						// Send file content
						while (true)
						{
							file.read(data, sendChunkSize);

							uint32_t packetSize = file.gcount();

							size_t sendedSize = sock->write_some(buffer(data, packetSize));

							if (packetSize < sendChunkSize)
							{
								break;
							}

							chunkCount++;
						}
					}
					catch (...)
					{
						cout << endl  << "Transfer was interrupted" << endl;
						file.close();
						isShowSpeed = false;
						speedThread->join();
						delete speedThread;
						delete data;
					}

					isShowSpeed = false;
					cout << endl;

					file.close();

					sock->read_some(buffer(data, sendBufferSize));

					if (string(data) == "File received")
					{
						cout << "File has been successfully sent" << endl;
					}


					speedThread->join();
					delete speedThread;
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

int64_t FileTransport::SendFileSize(string fileName)
{
	ifstream file(fileName, std::ifstream::ate | std::ifstream::binary);
	int64_t fileSize = file.tellg();
	file.close();
	string fileSizeStr = std::to_string(fileSize);
	int zeroPos = fileSizeStr.size();
	strcpy_s(data, sendBufferSize, fileSizeStr.c_str());
	data[zeroPos] = '\0';

	sock->write_some(buffer(data, sendBufferSize));

	return fileSize;
}

void FileTransport::Receive(string filenameFrom, string filenameTo)
{
	data = new char[receiveBufferSize + 1];

	remove(filenameTo.c_str());

	string remoteAddress = sock->remote_endpoint().address().to_string();
	string fileDownload = filenameTo + remoteAddress + ".download";

	if (data != nullptr)
	{
		uint32_t downloadedSize = 0;
		ifstream fd(fileDownload);
		if (fd.good())
		{
			downloadedSize = SendFileSize(fileDownload);
			fd.close();
		}
		else
		{
			sock->write_some(buffer("0"));
		}
		sock->read_some(buffer(data, receiveBufferSize));
		cout << data << endl;

		// Create the file
		file.open(fileDownload, ios::out | ios::app | ios::binary);

		if (file.is_open())
		{
			file.seekp(downloadedSize);
			cout  << "Start download from " << downloadedSize << endl;
			sock->write_some(buffer("I'AM READY"));

			// Get filesize
			//--------------------------------------------------------
			sock->read_some(buffer(data, receiveBufferSize));
			int64_t fileSize = std::stoi(data);
			cout << "Filesize = " << fileSize << endl;

			fileSize -= downloadedSize;

			if (fileSize == 0)
			{
				cout << "Error cause on sent size" << endl;
			}
			else
			{
				try
				{
					// Get file content
					while (fileSize != 0)
					{
						// Get packet
						int64_t readedSize = sock->read_some(buffer(data, receiveBufferSize));

						fileSize -= readedSize;

						//cout << "Readed " << readedSize << " left " << fileSize << '\r';

						file.write(data, readedSize);
					}
				}
				catch (...)
				{
					cout << endl << "Transfer was interrupted" << endl;
					file.close();
					delete data;
				}
				
				file.close();

				sock->write_some(buffer("File received"));

				int result = rename(fileDownload.c_str(), filenameTo.c_str());
				if (!result)
				{
					cout << "Can't rename file" << endl;
				}
				else
				{
					cout << "File renamed from " << fileDownload.c_str() << " to " << filenameTo.c_str() << endl;
				}

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