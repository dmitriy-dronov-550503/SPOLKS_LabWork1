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
	void ServerThread();
	bool isServerActive;

public:
	Server();
	~Server();

	void Join();

	void ClientSession(socket_ptr sock, bool& isServerActive);

	void ParseCommand(socket_ptr sock, string command);

	void CmdEcho(socket_ptr sock, vector<string> cmds);

	void CmdTime(socket_ptr sock, vector<string> cmds);

	void CmdReceiveFile(socket_ptr sock, vector<string> cmds);

	void CmdSendFile(socket_ptr sock, vector<string> cmds);
};

