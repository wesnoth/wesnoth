/*
   Copyright (C) 2007 - 2014 by Benoit Timbert <benoit.timbert@free.fr>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SHA1_H_INCLUDED
#define SHA1_H_INCLUDED

#include <string>
#include <SDL_types.h>

class sha1_hash
{
public:

	/** Make a hash from a string */
	sha1_hash(const std::string& str);

	/** Display the hash */
	std::string display();
private:

	/** Process the next 512 bits block */
	void next(Uint8 block[64]);
	Uint32 H0, H1, H2, H3, H4;

};

#endif
