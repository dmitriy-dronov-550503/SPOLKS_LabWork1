#include "stdafx.h"
#include "CommandCenter.h"
#include <algorithm>
#include <string>

string CommandCenter::ToLowerCase(string str)
{
	string lowerStr = str;
	std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);

	return lowerStr;
}

vector<string> CommandCenter::Parse(string command)
{
	vector<string> cmds;	
	string parsed;
	char divider = ' ';

	if (command.empty()) return cmds;

	for (int i = 0; i < command.length(); i++)
	{
		if (command[i] == '\"')
		{
			if (divider == ' ')
			{
				divider = '\"';
			}
			else
			{
				divider = ' ';
				cmds.push_back(parsed);
				parsed = "";
				i++;
			}

			continue;
		}

		if (command[i] != divider)
		{
			parsed.push_back(command[i]);

			if (command[i + 1] == '\0')
			{
				cmds.push_back(parsed);
			}
		}
		else
		{
			cmds.push_back(parsed);
			parsed = "";
		}
	}

	cmds[0] = ToLowerCase(cmds[0]);

	return cmds;
}