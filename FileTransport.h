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
public:
	static void Send(socket_ptr sock, string filenameFrom, string filenameTo);
	static void Receive(socket_ptr sock, string filenameFrom, string filenameTo);
};

