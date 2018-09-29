#pragma once
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

using namespace std;
using namespace std::chrono;
using namespace boost::asio;

typedef boost::shared_ptr<ip::tcp::socket> socket_ptr;

void ShowSpeed(bool& isActive, time_point<steady_clock, nanoseconds> start, int64_t& fileSizeLeft, const int64_t fileSize);

class FileTransport
{
private:
	constexpr static uint32_t sendBufferSize = 16 * 1024;
	constexpr static uint32_t sendChunkSize = 8 * 1024;
	constexpr static uint32_t receiveBufferSize = 16 * 1024;
	
	socket_ptr sock;
	char* data;
	time_point<steady_clock, nanoseconds> start;
	thread* speedThread;
	bool isShowSpeed;
	fstream file;

public:
	FileTransport(socket_ptr sock_in);
	void Send(string filenameFrom, string filenameTo);
	void Receive(string filenameFrom, string filenameTo);
	int64_t SendFileSize(string fileName);
};

