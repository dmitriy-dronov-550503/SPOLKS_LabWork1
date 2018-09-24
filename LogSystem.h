#pragma once
#include <iostream>

using namespace std;

ostream& log_time(ostream& os);

class LogSystem {
public:
	static const string CurrentDateTime();
	static void Log(string message);
	ostream& operator<<(ostream& stream);
};
