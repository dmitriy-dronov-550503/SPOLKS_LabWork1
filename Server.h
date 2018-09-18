#pragma once
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "CommandCenter.h"

using namespace std;
using namespace boost::asio;

typedef boost::shared_ptr<ip::tcp::socket> socket_ptr;

class Server
{
private:
	thread* serverThread;
	static void ServerThread();

public:
	Server();
	~Server();

	void Join();

	static void ClientSession(socket_ptr sock, bool& isServerActive);

	static void ParseCommand(socket_ptr sock, string command);

	static void CmdEcho(socket_ptr sock, vector<string> cmds);

	static void CmdTime(socket_ptr sock, vector<string> cmds);

	static void CmdReceiveFile(socket_ptr sock, vector<string> cmds);

};

