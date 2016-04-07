/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "input_stream.hpp"

#ifndef _WIN32

#include <algorithm>
#include <iostream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>

#endif

input_stream::input_stream(const std::string& path) :
	fd_(-1),
	path_(path),
	data_()
{
#ifndef _WIN32
	if(path == "") {
		return;
	}

	const int res = mkfifo(path.c_str(),0660);
	if(res != 0) {
		std::cerr << "could not make fifo at '" << path << "' (" << errno << ")\n";
	}

	fd_ = open(path.c_str(),O_RDONLY|O_NONBLOCK);

	if(fd_ == -1) {
		std::cerr << "failed to open fifo at '" << path << "' (" << errno << ")\n";
	} else {
		std::cerr << "opened fifo at '" << path << "'. Server commands may be written to this file.\n";
	}
#endif
}

input_stream::~input_stream()
{
#ifndef _WIN32
	stop();
#endif
}

void input_stream::stop()
{
#ifndef _WIN32
	if(fd_ != -1) {
		close(fd_);
		unlink(path_.c_str());
		fd_ = -1;
	}
#endif
}

#ifndef _WIN32
bool input_stream::read_line(std::string& str)
{
	if(fd_ == -1) {
		return false;
	}

	const size_t block_size = 4096;
	char block[block_size];

	const size_t nbytes = read(fd_,block,block_size);
	if (nbytes == static_cast<size_t>(-1)) {
		return false;
	}
	std::copy(block,block+nbytes,std::back_inserter(data_));

	const std::deque<char>::iterator itor = std::find(data_.begin(),data_.end(),'\n');
	if(itor != data_.end()) {
		str.resize(itor - data_.begin());
		std::copy(data_.begin(),itor,str.begin());
		data_.erase(data_.begin(),itor+1);
		return true;
	} else {
		return false;
	}
}
#else
bool input_stream::read_line(std::string&)
{
	return false;
}
#endif
