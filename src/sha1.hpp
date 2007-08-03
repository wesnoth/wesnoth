/* $Id$ */
/*
   Copyright (C) 2007 by Benoit Timbert <benoit.timbert@free.fr>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SHA1_H_INCLUDED
#define SHA1_H_INCLUDED

#include <string>
#include <stdint.h>

class sha1_hash
{
public:
	void hash(const std::string& str);
	std::string display();
private:
	void next(char block[64]);
	uint32_t H0, H1, H2, H3, H4;
	
};

#endif
