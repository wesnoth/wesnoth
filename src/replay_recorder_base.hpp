/*
   Copyright (C) 2017-2018 by the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once
#include <cassert>
#include <boost/ptr_container/ptr_vector.hpp>

#include "config.hpp"

class config_writer;
class replay_recorder_base
{
public:
	replay_recorder_base();
	~replay_recorder_base();

	void swap(replay_recorder_base& other);
	void swap(replay_recorder_base&& other) {swap(other);}
	int get_pos() const;

	int size() const;

	config& get_command_at(int pos);

	config& add_child();

	config& get_upload_log();

	void remove_command(int index);

	config& insert_command(int index);

	void set_to_end();

	void set_pos(int pos);

	void append_config(const config& data);
	/// Clears the passed config.
	void append_config(config& data);

	void write(config_writer& out) const;

	void write(config& out) const;

	void delete_upcoming_commands();
protected:
	config upload_log_;
	boost::ptr_vector<config> commands_;
	int pos_;
};

/** Implement non-member swap function for std::swap (calls @ref replay_recorder_base::swap). */
void swap(replay_recorder_base& lhs, replay_recorder_base& rhs);
