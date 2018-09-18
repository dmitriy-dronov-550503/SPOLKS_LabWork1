#pragma once
#include <iostream>
#include <vector>
using namespace std;

typedef string(*CommandHandler)(vector<string> args);

class CommandCenter
{
/*private:
	constexpr static uint32_t commandsCount = 2;

	class CommandItem
	{
	public:
		string cmdName;
		CommandHandler cmdHandler;

		CommandItem(const char* cmdNm, CommandHandler cmdHdlr)
		{
			cmdName = string(cmdNm);
			cmdHandler = cmdHdlr;
		}
	};

	static const CommandItem commands[commandsCount] =
	{
		//CommandItem("ECHO", CmdEcho),
		//CommandItem("TIME", CommandCenter::CmdTime)
	};


	static string ToLowerCase(string str);

public:

	static string Request(string command);

	static string CmdEcho(vector<string> args);
	static string CmdTime(vector<string> args);*/

private:
	static string ToLowerCase(string str);

public:
	static vector<string> Parse(string command);
};








