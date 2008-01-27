/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include <iostream>
#include <string>

const int BUFFER_SIZE = 64;
const std::string replace = "SVN_REVISION";

void output(char buf[], int bufi, int matchi)
{
	for (int i = 0;i <= matchi; --matchi)
	{
		std::cout << buf[bufi - matchi + i];
	}
}

int main(int argc, char ** argv)
{
	if (argc != 2)
	{
		std::cout << argc;
		return -1;
	}
	std::string replace_with(argv[1]);
	char * buffer = new char[BUFFER_SIZE];
	int bufi;
	int matchi = 0;
	int read = 0;

	std::cin.read(buffer, BUFFER_SIZE);
	while( read = std::cin.gcount() )
	{
		for(bufi = 0; bufi < read; ++bufi)
		{
			if (buffer[bufi] == replace[matchi] )
			{
				++matchi;
				if (matchi == replace.size())
				{
					matchi = 0;
					std::cout << replace_with;
				}
			}
			else
			{
				output(buffer,bufi,matchi);
				matchi = 0;
			}
		}
		std::cin.read(buffer, BUFFER_SIZE);
	}
	return 0;
}
