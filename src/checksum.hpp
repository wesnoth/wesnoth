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

#ifndef CHECKSUM_HPP_INCLUDED
#define CHECKSUM_HPP_INCLUDED

#include <streambuf>
#include <iostream>

class checksumstreambuf : public std::basic_streambuf<char>
{
public:
	checksumstreambuf();
	unsigned long checksum();

protected:
	virtual int overflow(int c);

private:
	void sum(unsigned char* begin, unsigned char* end);

	enum { BUFFERSIZE = 20 };
	unsigned char buffer[BUFFERSIZE];

	unsigned short sa;
	unsigned short sb;

	unsigned short csuma;
	unsigned short csumb;
};

class checksumstream : private checksumstreambuf, public std::basic_ostream<char>
{
public:
	checksumstream();
	unsigned long checksum();
private:
	checksumstreambuf& sbuf;
};

#endif // CHECKSUM_HPP_INCLUDED
