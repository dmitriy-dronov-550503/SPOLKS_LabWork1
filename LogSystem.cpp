#include "stdafx.h"
#include "LogSystem.h"

const string LogSystem::CurrentDateTime() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	localtime_s(&tstruct, &now);
	strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

	return buf;
}

void LogSystem::Log(string message)
{
	cout << "[" << CurrentDateTime().c_str() << "] : " << message.c_str() << endl;
}

ostream& LogSystem::operator<<(ostream& stream)
{
	stream << "[" << CurrentDateTime().c_str() << "] : ";
	return stream;
}

