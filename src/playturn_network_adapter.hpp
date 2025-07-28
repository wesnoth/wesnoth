/*
	Copyright (C) 2017 - 2025
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "config.hpp"
#include <list>
#include <functional>
/*
	The purpose if this class is to preprocess incoming network data, and provide a steam that always returns just one command/action at a time.
	Especially we want each replay command in his own [turn].
*/
class playturn_network_adapter
{
public:
	typedef std::function<bool(config&)> source_type;

	playturn_network_adapter(source_type source);
	~playturn_network_adapter();

	//returns true on success.
	//dst has to be empty before the call.
	//after the call dst contains one child when returned true otherwise it's empty.
	bool read(config& dst);
	//returns false if there is still data in the internal buffer.
	bool is_at_end() const;
	void push_front(config&& cfg);
private:
	//reads data from the network stream.
	void read_from_network();
	//a function to receive data from the network.
	source_type network_reader_;

	// note: all of the following could be replaced by a simple std::list<config> if we would
	//       split incoming tags right after we rechived them from network_reader_ the reason
	//       why we currently don'T do that is for performance.

	//this always contains one empty config because we want a valid value for next_.
	std::list<config> data_;
	//packages that the client could not process at that point.
	std::list<config> data_front_;
	//the position of the next to be received element in data_->front().
	config::all_children_iterator next_;
	//if we are processing a [turn] with multiple [command] we want to split them.
	//In this case next_command_num_ is the next to be processed turn into a command otherwise it's 0;
	unsigned int next_command_num_;
};
