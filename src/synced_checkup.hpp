/*
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SYNCED_CHECKUP_H_INCLUDED
#define SYNCED_CHECKUP_H_INCLUDED

#include "config.hpp"
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
	/**
		compares data on all clients in a networked game, the disadvantage is,
		that the clients have to communicate more which  might be not wanted if some persons have laggy inet.
		returns whether the two config objects are equal.

		This is currently not used.
		This is currently not implemented.

		TODO: we might want to change the design to have a 'local_checkup' subclass (the normal case)
		and a 'networked_checkup' subclass (for network OOS debugging) of the checkup class that then replace
		the synced_checkup class. Instead of having 2 different methods local_checkup,networked_checkup.
	*/
	virtual bool networked_checkup(const config& expected_data, config& real_data) = 0;
};

class synced_checkup : public checkup
{
public:
	synced_checkup(config& buffer);
	virtual ~synced_checkup();
	virtual bool local_checkup(const config& expected_data, config& real_data);
	virtual bool networked_checkup(const config& expected_data, config& real_data);
private:
	config& buffer_;
	unsigned int  pos_;
};

/*
	the only purpose of these function isto thro OOS erros, because they should never be called.
*/

class ignored_checkup : public checkup
{
public:
	ignored_checkup();
	virtual ~ignored_checkup();
	/**
		always returns true
	*/
	virtual bool local_checkup(const config& expected_data, config& real_data);
	/**
		always returns true
	*/
	virtual bool networked_checkup(const config& expected_data, config& real_data);
};

/*
	this is a synced_checkup during a synced context otherwise a invalid_checkup object.
*/

extern checkup* checkup_instance;

#endif
