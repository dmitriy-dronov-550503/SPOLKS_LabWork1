#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

using namespace std;
using namespace boost::asio;

typedef boost::shared_ptr<ip::tcp::socket> socket_ptr;

class FileTransport
{
private:
	constexpr static uint32_t sendBufferSize = 16 * 1024 * 1024;
	constexpr static uint32_t receiveBufferSize = 64 * 1024 * 1024;
public:
	static void Send(socket_ptr sock, string filenameFrom, string filenameTo);
	static void Receive(socket_ptr sock, string filenameFrom, string filenameTo);
	static int64_t WriteFileSize(char* data, string filename);
	static void SendChunk();
};

