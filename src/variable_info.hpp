/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2014 by Philippe Plantier <ayin@anathas.org>

   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** Information on a WML variable. */

#include "config.hpp"
#include <string>

struct variable_info
{
	typedef config::child_itors array_range;

	/**
	 * TYPE: the correct variable type should be decided by the user of the info structure
	 * Note: an Array can also be considered a Container, since index 0 will be used by default
	 */
	enum TYPE { TYPE_SCALAR,    //a Scalar variable resolves to a t_string attribute of *vars
	            TYPE_ARRAY,     //an Array variable is a series of Containers
	            TYPE_CONTAINER, //a Container is a specific index of an Array (contains Scalars)
	            TYPE_UNSPECIFIED };

	variable_info(config& source, const std::string& varname, bool force_valid=true,
		TYPE validation_type=TYPE_UNSPECIFIED);

	TYPE vartype; //default is TYPE_UNSPECIFIED
	bool is_valid;
	std::string key; //the name of the internal attribute or child
	bool explicit_index; //true if query ended in [...] specifier
	size_t index; //the index of the child
	config *vars; //the containing node in game_data s variables
	/**
	 * Results: after deciding the desired type, these methods can retrieve the result
	 * Note: first you should force_valid or check is_valid, otherwise these may fail
	 */
	config::attribute_value &as_scalar();
	config& as_container();
	array_range as_array(); //range may be empty
	static config& get_temporaries();

	void set_range(config& data, std::string mode);

	void clear(bool only_tables);
};
