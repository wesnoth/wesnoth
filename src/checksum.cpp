/* $Id$ */
/*
   Copyright (C) 2005 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "checksum.hpp"

checksumstreambuf::checksumstreambuf() : csuma(0), csumb(0)
{
	setp((char*)buffer, (char*)buffer+BUFFERSIZE);
}

unsigned long checksumstreambuf::checksum()
{
	sum((unsigned char*)pbase(), (unsigned char*)pptr());

	return (sa << 16) + sb;
}

int checksumstreambuf::overflow(int c)
{
	sum(buffer, buffer+BUFFERSIZE);
	csuma = sa;
	csumb = sb;

	setp((char*)buffer, (char*)buffer+BUFFERSIZE);
	buffer[0] = (unsigned char)(c);
	pbump(1);
	return c;
}

void checksumstreambuf::sum(unsigned char* begin, unsigned char* end)
{
	sa = csuma;
	sb = csumb;

	for(unsigned char* p = begin; p < end; p += 2) {
		unsigned short x;

		if(p+1 == end) {
			x = *p;
		} else {
			x = (unsigned short)(*p) + ((unsigned short)(*(p+1)) << 8);
		}

		sa += sb;
		sb += x;
	}
}


checksumstream::checksumstream() : std::ostream(&sbuf)
{
}

unsigned long checksumstream::checksum()
{
	return sbuf.checksum();
}

