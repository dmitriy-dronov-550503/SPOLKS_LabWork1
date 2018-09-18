#pragma once
#include <iostream>
#include "stdafx.h"

using namespace std;

class LogSystem {
public:
	static const string CurrentDateTime();
	static void Log(string message);
	ostream& operator<<(ostream& stream);
};
