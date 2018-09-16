#include "stdafx.h"
#include "CommandCenter.h"
#include <algorithm>
#include <string>

string CommandCenter::ToLowerCase(string str)
{
	for (int i = 0; i < str.length(); i++)
	{
		if (str[i] > 'a')
		{
			str[i] = str[i] - ('a' - 'A');
		}
	}

	return str;
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

	std::transform(cmds[0].begin(), cmds[0].end(), cmds[0].begin(), ::tolower);

	return cmds;
}