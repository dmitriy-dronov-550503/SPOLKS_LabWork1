#include "stdafx.h"
#include "FileTransport.h"
#include "LogSystem.h"

FileTransport::FileTransport(socket_ptr sock_in)
{
	sock = sock_in;
}

void ShowSpeed(bool& isActive, time_point<steady_clock, nanoseconds> start, int64_t& fileSizeLeft, const int64_t fileSize)
{
	while (isActive)
	{
		std::cout << '\r';
		time_point<steady_clock, nanoseconds> finish = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed = finish - start;
		//std::cout << "Average speed: " << ((chunkCount * chunkSize) / elapsed.count()) / (1024 * 1024) << " MB/s";
		
		double percentReady = (1.0 - ((double)fileSizeLeft / fileSize));

		cout << '[';
		for (int i = 0; i < 60; i++)
		{
			if (i == 30)
			{
				cout << (uint32_t)(percentReady * 100) << " %";
			}
			if (i <= (percentReady * 60))
			{
				cout << '#';
			}
			else
			{
				cout << ' ';
			}
		}
		cout << ']';

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
					isShowSpeed = true;
					start = std::chrono::high_resolution_clock::now();
					int64_t fileSizeLeft = fileSize - downloadedSize;
					speedThread = new thread(ShowSpeed, std::ref(isShowSpeed), start, std::ref(fileSizeLeft), fileSizeLeft);

					try
					{
						// Send file content
						while (true)
						{
							file.read(data, sendChunkSize);

							uint32_t packetSize = file.gcount();

							//cout << "Readed from file " << packetSize << " bytes" << endl;

							size_t sendedSize = sock->write_some(buffer(data, packetSize));

							fileSizeLeft -= sendedSize;

							//cout << "Sended size = " << sendedSize << endl;

							if (packetSize < sendChunkSize)
							{
								break;
							}
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
						cout << endl << "File has been successfully sent" << endl;
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

			int64_t fileSizeLeft = fileSize - downloadedSize;

			if (fileSizeLeft == 0)
			{
				cout << "Error cause on sent size" << endl;
			}
			else
			{
				isShowSpeed = true;
				start = std::chrono::high_resolution_clock::now();
				speedThread = new thread(ShowSpeed, std::ref(isShowSpeed), start, std::ref(fileSizeLeft), fileSizeLeft);

				try
				{
					// Get file content
					while (fileSizeLeft != 0)
					{
						// Get packet
						int64_t readedSize = sock->read_some(buffer(data, receiveBufferSize));

						fileSizeLeft -= readedSize;

						//cout << "Readed packet size = " << readedSize << endl;

						//cout << "Readed " << readedSize << " left " << fileSize << '\r';

						/*if (readedSize > fileSizeConst)
						{
							readedSize = fileSizeConst;
							file.write(data, readedSize);
							break;
						}*/

						file.write(data, readedSize);
					}
				}
				catch (...)
				{
					cout << endl << "Transfer was interrupted" << endl;
					isShowSpeed = false;
					speedThread->join();
					delete speedThread;
					file.close();
					delete data;
				}
				
				isShowSpeed = false;

				file.close();

				sock->write_some(buffer("File received"));

				int err = rename(fileDownload.c_str(), filenameTo.c_str());

				if (err)
				{
					cout << endl << "Can't rename file" << endl;
				}
				else
				{
					cout << endl << "File renamed from " << fileDownload.c_str() << " to " << filenameTo.c_str() << endl;
				}
				
				speedThread->join();
				delete speedThread;

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
