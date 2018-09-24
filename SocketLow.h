#pragma once
#include <iostream>
#include <boost/asio.hpp>

#ifdef _WIN32
#include <Mstcpip.h>
#endif

using namespace std;
using namespace boost::asio;

typedef boost::shared_ptr<ip::tcp::socket> socket_ptr;

class SocketLow
{
public:
	SocketLow();
	~SocketLow();
	static void SetKeepAlive(socket_ptr sock);
};

