/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2018 by Philippe Plantier <ayin@anathas.org>

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
#include "utils/const_clone.hpp"

#include <string>
#include <type_traits>

namespace variable_info_implementation
{
/**
 * The variable_info policy classes.
 *
 * Each of these classes describes a different behavior for reading data from a variable
 * and should implement two functions:
 *
 * - get_child_at           Describes the desired behavior when reading variable info.
 * - error_message          Error message regarding policy behavior.
 */

/** Takes a const reference and is guaranteed to not change the config. */
class vi_policy_const
{
public:
	static const config& get_child_at(const config& cfg, const std::string& key, int index)
	{
		assert(index >= 0);
		// cfg.child_or_empty does not support index parameter
		if(const config& child = cfg.child(key, index)) {
			return child;
		}

		static const config empty_const_cfg;
		return empty_const_cfg;
	}

	static std::string error_message(const std::string& name)
	{
		return "Cannot resolve variable '" + name + "' for reading.";
	}
};

/** Creates a child table when resolving name if it doesn't exist yet. */
class vi_policy_create
{
public:
	static config& get_child_at(config& cfg, const std::string& key, int index)
	{
		assert(index >= 0);
		// the 'create_if_not_existent' logic.
		while(static_cast<int>(cfg.child_count(key)) <= index) {
			cfg.add_child(key);
		}

		return cfg.child(key, index);
	}

	static std::string error_message(const std::string& name)
	{
		return "Cannot resolve variable '" + name + "' for writing.";
	}
};

/**
 * Will throw an exception when trying to access a nonexistent table.
 * Note that the other types can throw too if name is invlid, such as '..[[[a'.
 */
class vi_policy_throw
{
public:
	static config& get_child_at(config& cfg, const std::string& key, int index)
	{
		assert(index >= 0);
		if(config& child = cfg.child(key, index)) {
			return child;
		}

		throw invalid_variablename_exception();
	}

	static std::string error_message(const std::string& name)
	{
		return "Cannot resolve variable '" + name + "' for writing without creating new children.";
	}
};

// ==================================================================
// Other implementation details.
// ==================================================================

template<typename T, typename V>
struct maybe_const : public utils::const_clone<T, V>
{
	// Meta type aliases provided by const_clone
};

template<>
struct maybe_const<config::child_itors, const vi_policy_const>
{
	using type = config::const_child_itors;
};

enum variable_info_state_type {
	state_start = 0, /**< Represents the initial variable state before processing. */
	state_named,     /**< The result of .someval. This can either mean an attribute value or a child range. */
	state_indexed,   /**< The result of .someval[index]. This is never an attribute value and is always a single config. */
	state_temporary, /**< The result of .length. This value can never be written, it can only be read. */
};

template<typename V>
struct variable_info_state
{
	using child_t = typename maybe_const<config, V>::type;

	variable_info_state(child_t& vars)
		: child_(&vars)
		, key_()
		, index_(0)
		, temp_val_()
		, type_(state_start)
	{
		child_ = &vars;
	}

	// The meaning of the following 3 depends on 'type_', but the current config is usually
	// child_->child_at(key_, index_).
	child_t* child_;
	std::string key_;
	int index_;

	// If we have a temporary value like .length we store the result here.
	config::attribute_value temp_val_;

	// See @ref variable_info_state_type
	variable_info_state_type type_;
};
} // end namespace variable_info_implementation

/** Helper template alias for maybe_const, defined at global scope for convenience. */
template<typename T, typename V>
using maybe_const_t = typename variable_info_implementation::maybe_const<T, V>::type;
