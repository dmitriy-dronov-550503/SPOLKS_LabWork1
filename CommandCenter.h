#pragma once
#include <iostream>
#include <vector>
using namespace std;

typedef string(*CommandHandler)(vector<string> args);

class CommandCenter
{
private:
	static string ToLowerCase(string str);

public:
	static vector<string> Parse(string command);
};








