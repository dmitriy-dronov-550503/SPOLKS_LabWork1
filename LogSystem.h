#pragma once
#include <iostream>

using namespace std;

class LogSystem {
public:
	static const string CurrentDateTime();
	static void Log(string message);
	ostream& operator<<(ostream& stream);
};
