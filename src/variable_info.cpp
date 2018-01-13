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

#include "variable_info.hpp"
#include "variable_info_private.hpp"

#include <utility>

namespace variable_info_implementation
{
/**
 * Helper function to apply the result of a specified visitor to a variable_info object.
 *
 * @tparam V                   Visitor type.
 * @tparam T                   Visitor argument parameter pack.
 *
 * @param state                Info state (the actual variable data).
 * @param args                 Arguments to forward to visitor constructor.
 *
 * @returns                    Visitor output in its specified type.
 * @throws std::range_error    If @a state has an invalid type_ field.
 */
template<typename V, typename... T>
typename V::result_t apply_visitor(typename V::param_t state, T&&... args)
{
	static_assert(std::is_base_of<
		info_visitor_base<
			typename V::result_t,
			utils::remove_reference_t<typename V::param_t>>,
		V>::value, "Invalid visitor type.");

	// Create the visitor.
	V visitor(std::forward<T>(args)...);

	switch(state.type_) {
	case state_start:
		return visitor.from_start(state);
	case state_named:
		return visitor.from_named(state);
	case state_indexed:
		return visitor.from_indexed(state);
	case state_temporary:
		return visitor.from_temporary(state);
	}

	throw std::range_error("Failed to convert the TVisitor::param_t type");
}
} // end namespace variable_info_implementation

using namespace variable_info_implementation;

template<typename V>
variable_info<V>::variable_info(const std::string& varname, maybe_const_t<config, V>& vars) NOEXCEPT
	: name_(varname)
	, state_(vars)
	, valid_(true)
{
	try {
		calculate_value();
	} catch(const invalid_variablename_exception&) {
		valid_ = false;
	}
}

template<typename V>
void variable_info<V>::calculate_value()
{
	size_t previous_index = 0, name_size = name_.size();

	for(size_t loop_index = 0; loop_index < name_size; loop_index++) {
		switch(name_[loop_index]) {
		case '.':
		case '[':
			/* '.' and '[' mark the end of a string key.
			 * The result is obviously that '.' and '[' are
			 * treated equally so 'aaa.9].bbbb[zzz.uu.7]'
			 * is interpreted as  'aaa[9].bbbb.zzz.uu[7]'
			 * Use is_valid_variable function for stricter variable name checking.
			 */
			apply_visitor<get_variable_key_visitor<V>>(
				state_, name_.substr(previous_index, loop_index - previous_index));

			previous_index = loop_index + 1;
			break;
		case ']':
			// ']' marks the end of an integer key.
			apply_visitor<get_variable_index_visitor<V>>(state_, parse_index(&name_[previous_index]));

			// After ']' we always expect a '.' or the end of the string
			// Ignore the next char which is a '.'
			loop_index++;
			if(loop_index < name_.length() && name_[loop_index] != '.') {
				throw invalid_variablename_exception();
			}

			previous_index = loop_index + 1;
			break;
		default:
			break;
		}
	}

	if(previous_index != name_.length() + 1) {
		// The string didn't end with ']'
		// In this case we still didn't add the key behind the last '.'
		apply_visitor<get_variable_key_visitor<V>>(state_, name_.substr(previous_index));
	}
}

template<typename V>
bool variable_info<V>::explicit_index() const
{
	throw_on_invalid();
	return state_.type_ == state_start || state_.type_ == state_indexed;
}

template<typename V>
maybe_const_t<config::attribute_value, V>& variable_info<V>::as_scalar() const
{
	throw_on_invalid();
	return apply_visitor<as_scalar_visitor<V>>(state_);
}

template<typename V>
maybe_const_t<config, V>& variable_info<V>::as_container() const
{
	throw_on_invalid();
	return apply_visitor<as_container_visitor<V>>(state_);
}

template<typename V>
maybe_const_t<config::child_itors, V> variable_info<V>::as_array() const
{
	throw_on_invalid();
	return apply_visitor<as_array_visitor<V>>(state_);
}

template<typename V>
void variable_info<V>::throw_on_invalid() const
{
	if(!valid_) {
		throw invalid_variablename_exception();
	}
}

template<typename V>
std::string variable_info<V>::get_error_message() const
{
	return V::error_message(name_);
}

template<typename V>
bool variable_info<V>::exists_as_attribute() const
{
	throw_on_invalid();
	return (state_.type_ == state_temporary)
	   || ((state_.type_ == state_named) && state_.child_->has_attribute(state_.key_));
}

template<typename V>
bool variable_info<V>::exists_as_container() const
{
	throw_on_invalid();
	return apply_visitor<exists_as_container_visitor<V>>(state_);
}

template<typename V>
void variable_info_mutable<V>::clear(bool only_tables) const
{
	this->throw_on_invalid();
	return apply_visitor<clear_value_visitor<V>>(this->state_, only_tables);
}

/**
 * In order to allow the appropriate 'children' argument to forward through to the appropriate
 * ctor of type T in as_range_visitor_base via apply_visitor, we need to specify the argument
 * type as an rvalue reference to lvalue reference in order to collapse to an lvalue reference.
 *
 * Using a convenient template alias for convenience.
 */
template<typename V, typename T>
using range_visitor_wrapper = as_range_visitor_base<V, T, std::vector<config>&>;

template<typename V>
config::child_itors variable_info_mutable<V>::append_array(std::vector<config> children) const
{
	this->throw_on_invalid();
	return apply_visitor<range_visitor_wrapper<V, append_range_h>>(this->state_, children);
}

template<typename V>
config::child_itors variable_info_mutable<V>::insert_array(std::vector<config> children) const
{
	this->throw_on_invalid();
	return apply_visitor<range_visitor_wrapper<V, insert_range_h>>(this->state_, children);
}

template<typename V>
config::child_itors variable_info_mutable<V>::replace_array(std::vector<config> children) const
{
	this->throw_on_invalid();
	return apply_visitor<range_visitor_wrapper<V, replace_range_h>>(this->state_, children);
}

template<typename V>
void variable_info_mutable<V>::merge_array(std::vector<config> children) const
{
	this->throw_on_invalid();
	apply_visitor<range_visitor_wrapper<V, merge_range_h>>(this->state_, children);
}

// Force compilation of the following template instantiations
template class variable_info<const vi_policy_const>;
template class variable_info<vi_policy_create>;
template class variable_info<vi_policy_throw>;

template class variable_info_mutable<vi_policy_create>;
template class variable_info_mutable<vi_policy_throw>;
