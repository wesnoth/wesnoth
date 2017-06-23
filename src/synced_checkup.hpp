/*
   Copyright (C) 2014 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

class config;
struct map_location;
/**
	A class to check whether the results that were calculated in the replay match the results calculated during the original game.
	note, that you shouldn't add new checkups to existent user actions or you might break replay compability by bringing the [checkups] tag of older saves in unorder.

	so if you really want to add new checkups, you should wrap your checkup_instance->... call in a if(resources::state_of_game->classification.version ....) or similar.
*/
class checkup
{
public:
	checkup();
	virtual ~checkup();
	/**
		Compares data to the results calculated during the original game.
		It's undefined whether this function also compares calculated results from different clients in a mp game.
		returns whether the two config objects are equal.
	*/
	virtual bool local_checkup(const config& expected_data, config& real_data) = 0;
};

/**
	This checkup compares whether the results calculated during the original game match the ones calculated during replay.
	Whether this checkup also compares the calculated results of different clients in a a mp game depends on whether
	there was already data sended about the current synced command.
*/
class synced_checkup : public checkup
{
public:
	synced_checkup(config& buffer);
	virtual ~synced_checkup();
	virtual bool local_checkup(const config& expected_data, config& real_data);
private:
	config& buffer_;
	unsigned int  pos_;
};

class ignored_checkup : public checkup
{
public:
	ignored_checkup();
	virtual ~ignored_checkup();
	/**
		always returns true
	*/
	virtual bool local_checkup(const config& expected_data, config& real_data);
};
/**
	This checkup always compares the results in from different clients in a mp game but it also causes more network overhead.
*/
class mp_debug_checkup : public checkup
{
public:
	mp_debug_checkup();
	virtual ~mp_debug_checkup();
	virtual bool local_checkup(const config& expected_data, config& real_data);
};

/*
	this is a synced_checkup during a synced context otherwise a invalid_checkup object.
*/

extern checkup* checkup_instance;
