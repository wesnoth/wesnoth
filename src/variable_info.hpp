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

#include "config.hpp"

#include <string>
#include <type_traits>

class invalid_variablename_exception : public std::exception
{
public:
	invalid_variablename_exception() : std::exception() {}

	const char* what() const NOEXCEPT
	{
		return "invalid_variablename_exception";
	}
};

// NOTE: the detail file needs invalid_variablename_exception to be available,
// so include this after declaring it.
#include "variable_info_detail.hpp"

/** Information on a WML variable. */
template<typename V>
class variable_info
{
public:
	variable_info(const std::string& varname, maybe_const_t<config, V>& vars) NOEXCEPT;

	std::string get_error_message() const;

	bool explicit_index() const NOEXCEPT;

	/** @throws                 invalid_variablename_exception */
	bool exists_as_attribute() const;

	/** @throws                 invalid_variablename_exception */
	bool exists_as_container() const;

	/**
	 * If instantiated with vi_policy_const, the lifetime of the returned
	 * const attribute_value reference might end with the lifetime of this object.
	 *
	 * @throws                  invalid_variablename_exception
	 */
	maybe_const_t<config::attribute_value, V>& as_scalar() const;

	/**
	 * If instantiated with vi_policy_const, the lifetime of the returned
	 * const attribute_value reference might end with the lifetime of this object.
	 *
	 * @throws                  invalid_variablename_exception
	 */
	maybe_const_t<config, V>& as_container() const;

	/**
	 * If instantiated with vi_policy_const, the lifetime of the returned
	 * const attribute_value reference might end with the lifetime of this object.
	 *
	 * Range may be empty
	 * @throws                  invalid_variablename_exception
	 */
	maybe_const_t<config::child_itors, V> as_array() const;

protected:
	std::string name_;
	variable_info_implementation::variable_info_state<V> state_;
	bool valid_;

	void throw_on_invalid() const;
	void calculate_value();
};

/**
 * Additional functionality for a non-const variable_info.
 * @todo: should these functions take a reference?
 */
template<typename V>
class variable_info_mutable : public variable_info<V>
{
public:
	variable_info_mutable(const std::string& name, config& game_vars)
		: variable_info<V>(name, game_vars)
	{
		static_assert(!std::is_same<
			variable_info_implementation::vi_policy_const, typename std::remove_const<V>::type>::value,
			"variable_info_mutable cannot be specialized with 'vi_policy_const'"
		);
	}

	/**
	 * @returns                 The new appended range.
	 * @throws                  invalid_variablename_exception
	 */
	config::child_itors append_array(std::vector<config> children) const;

	/**
	 * @returns                 The new inserted range.
	 * @throws                  invalid_variablename_exception
	 */
	config::child_itors insert_array(std::vector<config> children) const;

	/**
	 * @returns                 The new range.
	 * @throws                  invalid_variablename_exception
	 */
	config::child_itors replace_array(std::vector<config> children) const;

	/**
	 * @throws                  invalid_variablename_exception
	 */
	void merge_array(std::vector<config> children) const;

	/**
	 * Clears the value this object points to.
	 *
	 * @param only_tables       If true, will not clear attribute values.
	 * @throws                  invalid_variablename_exception
	 */
	void clear(bool only_tables = false) const;
};

/** 'Create if nonexistent' access. */
using variable_access_create = variable_info_mutable<variable_info_implementation::vi_policy_create>;

/** 'Throw if nonexistent' access. */
using variable_access_throw  = variable_info_mutable<variable_info_implementation::vi_policy_throw>;

/**
 * Read-only access.
 *
 * NOTE: in order to easily mark certain types in this specialization as const we specify
 * the policy as const here. This allows the use of const_clone.
 */
using variable_access_const  = variable_info<const variable_info_implementation::vi_policy_const>;
