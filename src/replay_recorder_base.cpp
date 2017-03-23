/*
   Copyright (C) 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "replay_recorder_base.hpp"
#include "serialization/binary_or_text.hpp"

replay_recorder_base::replay_recorder_base(void)
	: upload_log_()
	, commands_()
	, pos_(0)
{

}


replay_recorder_base::~replay_recorder_base(void)
{
}


void replay_recorder_base::swap(replay_recorder_base& other)
{
	commands_.swap(other.commands_);
	std::swap(pos_, other.pos_);
	upload_log_.swap(other.upload_log_);
}

int replay_recorder_base::get_pos() const
{
	return pos_;
}

int replay_recorder_base::size() const
{
	return commands_.size();
}

config& replay_recorder_base::get_command_at(int pos)
{
	assert(pos < size());
	return commands_[pos];
}

config& replay_recorder_base::add_child()
{
	assert(pos_ <= size());
	commands_.insert(commands_.begin() + pos_, new config());
	++pos_;
	return commands_[pos_ - 1];
}
void replay_recorder_base::set_pos(int pos)
{
	assert(pos <= size());
	pos_ = pos;
}
void replay_recorder_base::set_to_end()
{
	pos_ = size();
}
config& replay_recorder_base::get_upload_log()
{
	return upload_log_;
}

void replay_recorder_base::remove_command(int index)
{
	assert(index < size());
	commands_.erase(commands_.begin() + index);
	if(index < pos_)
	{
		--pos_;
	}
}

config& replay_recorder_base::insert_command(int index)
{
	assert(index <= size());
	if(index < pos_)
	{
		++pos_;
	}
	return *commands_.insert(commands_.begin() + index, new config());
}


void replay_recorder_base::append_config(const config& data)
{
	if(const config& upload_log = data.child("upload_log"))
	{
		upload_log_ = upload_log;
	}
	for(const config& command : data.child_range("command"))
	{
		commands_.push_back(new config(command));
	}
}

void replay_recorder_base::append_config(config& data)
{
	if(config& upload_log = data.child("upload_log"))
	{
		upload_log_.swap(upload_log);
	}
	for(config& command : data.child_range("command"))
	{
		config* new_config = new config();
		new_config->swap(command);
		commands_.push_back(new_config);
	}
}

void replay_recorder_base::write(config_writer& out) const
{
	out.write_child("upload_log", upload_log_);
	for(int i = 0; i < pos_; ++i)
	{
		out.write_child("command", commands_[i]);
	}
}

void replay_recorder_base::write(config& out) const
{
	out.add_child("upload_log", upload_log_);
	for(int i = 0; i < pos_; ++i)
	{
		out.add_child("command", commands_[i]);
	}
}
void replay_recorder_base::delete_upcoming_commands()
{
	commands_.resize(pos_);
}
