#pragma once
#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

using namespace std;
using namespace boost::asio;

typedef boost::shared_ptr<ip::tcp::socket> socket_ptr;

class Client
{
private:
	thread* clientThread;
	static void ClientThread(string ipAddress);
	static void SendFile(socket_ptr sock, vector<string> argv);
	static void ReceiveFile(socket_ptr sock, vector<string> argv);

public:
	Client(string ipAddress = "127.0.0.1");
	~Client();

	void Join();
};

