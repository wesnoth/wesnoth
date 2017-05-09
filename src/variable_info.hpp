/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2017 by Philippe Plantier <ayin@anathas.org>

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

/** Information on a WML variable. */
#include <string>
#include "config.hpp"
#include "variable_info_detail.hpp"

class invalid_variablename_exception : public std::exception
{
public:
	invalid_variablename_exception() : std::exception() {}
	const char* what() const NOEXCEPT
	{
		return "invalid_variablename_exception";
	}
};

template<const variable_info_detail::variable_info_type vit>
class variable_info
{
public:

	typedef typename variable_info_detail::maybe_const<vit,config>::type config_var;
	/// Doesn't throw
	variable_info(const std::string& varname, config_var& vars);
	~variable_info();
	std::string get_error_message() const;
	/// Doesn't throw
	bool explicit_index() const;
	/// might throw invalid_variablename_exception
	bool exists_as_attribute() const;
	/// might throw invalid_variablename_exception
	bool exists_as_container() const;

	/**
		might throw invalid_variablename_exception
		NOTE:
			If vit == vit_const, then the lifime of the returned const attribute_value& might end with the lifetime of this object.
	*/
	typename variable_info_detail::maybe_const<vit, config::attribute_value>::type &as_scalar() const;
	/// might throw invalid_variablename_exception
	typename variable_info_detail::maybe_const<vit, config>::type & as_container() const;
	/// might throw invalid_variablename_exception
	typename variable_info_detail::maybe_const<vit, config::child_itors>::type as_array() const; //range may be empty

protected:
	std::string name_;
	variable_info_detail::variable_info_state<vit> state_;
	void throw_on_invalid() const;
	bool valid_;
	void calculate_value();
};

/// Extends variable_info with methods that can only be applied if vit != vit_const
template<const variable_info_detail::variable_info_type vit>
class non_const_variable_info : public variable_info<vit>, variable_info_detail::enable_if_non_const<vit>::type
{
public:
	non_const_variable_info(const std::string& name, config& game_vars) : variable_info<vit>(name, game_vars) {}
	~non_const_variable_info() {}

	/// clears the vale this object points to
	/// if only_tables = true it will not clear attribute values.
	/// might throw invalid_variablename_exception
	void clear(bool only_tables = false) const;

	// the following 4 functions are used by [set_variables]
	// they destroy the passed vector. (make it empty).

	/// @return: the new appended range
	/// might throw invalid_variablename_exception
	config::child_itors append_array(std::vector<config> childs) const;
	/// @return: the new inserted range
	/// might throw invalid_variablename_exception
	config::child_itors insert_array(std::vector<config> childs) const;
	/// @return: the new range
	/// might throw invalid_variablename_exception
	config::child_itors replace_array(std::vector<config> childs) const;
	/// merges
	/// might throw invalid_variablename_exception
	void merge_array(std::vector<config> childs) const;
};


/**
	this variable accessor will create a childtable when resolving name if it doesn't exist yet.
*/
typedef non_const_variable_info<variable_info_detail::vit_create_if_not_existent> variable_access_create;
/**
	this variable accessor will throw an exception when trying to access a non existent table.
	Note that the other types can throw too if name is invlid like '..[[[a'.
*/
typedef non_const_variable_info<variable_info_detail::vit_throw_if_not_existent>  variable_access_throw;
/**
	this variable accessor is takes a const reference and is guaranteed to not change the config.
*/
typedef variable_info<variable_info_detail::vit_const>                            variable_access_const;
