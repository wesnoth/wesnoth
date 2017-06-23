/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#include "color.hpp"
#include "formula/callable.hpp"
#include "formula/formula.hpp"
#include "formula/function.hpp"
#include "gui/core/log.hpp"
#include "gui/widgets/helper.hpp"
#include "lexical_cast.hpp"
#include "serialization/string_utils.hpp"
#include "tstring.hpp"

#include <cassert>

namespace gui2
{

/**
 * Template class can hold a value or a formula to calculate the value.
 *
 * A string is a formula when it starts with a right paren, no other validation
 * is done by this function, leading whitespace is significant.
 *
 * Upon getting the value of the formula a variable map is send. The variables
 * in the map can be used in the formula. The 'owners' of the class need to
 * document the variables available.
 *
 * @tparam T                      The type of the formula. This type needs to
 *                                be constructable form a string, either by a
 *                                lexical_cast or a template specialization in
 *                                this header.
 */
template<typename T>
class typed_formula
{
public:
	/**
	 * Constructor.
	 *
	 * @param str                 The string used to initialize the class, this
	 *                            can either be a formula or a string which can
	 *                            be converted to the type T.
	 * @param value               The default value for the object.
	 */
	explicit typed_formula<T>(const std::string& str, const T value = T());

	/**
	 * Returns the value, can only be used if the data is no formula.
	 *
	 * Another option would be to cache the output of the formula in value_
	 * and always allow this function. But for now decided that the caller
	 * needs to do the caching. It might be changed later.
	 */
	T operator()() const
	{
		assert(!has_formula());
		return value_;
	}

	/**
	 * Returns the value, can always be used.
	 *
	 * @param variables           The state variables which might be used in
	 *                            the formula. For example, screen_width can
	 *                            be set so the formula can return the half
	 *                            width of the screen.
	 *
	 * @param functions           The variables, which can be called during the
	 *                            evaluation of the formula. (Note it is also
	 *                            possible to add extra functions to the table,
	 *                            when the variable is not @c nullptr.
	 *
	 * @returns                   The stored result or the result of the
	 *                            evaluation of the formula.
	 */
	T operator()(const wfl::map_formula_callable& variables,
				 wfl::function_symbol_table* functions = nullptr) const;

	/** Determine whether the class contains a formula. */
	bool has_formula() const
	{
		return !formula_.empty();
	}

private:
	/**
	 * Converts the string to the template type.
	 *
	 * This function is used by the constructor to convert the string to the
	 * wanted value, if not a formula.
	 *
	 * @param str                 The str send to the constructor.
	 */
	void convert(const std::string& str);

	/**
	 * Executes the formula.
	 *
	 * This function does the calculation and can only be called if the object
	 * contains a formula.
	 *
	 * @param v                   A variant object containing the evaluated value
	 *                            of the formula.
	 *
	 * @returns                   The calculated value.
	 */
	T execute(wfl::variant& v) const;

	/**
	 * Contains the formula for the variable.
	 *
	 * If the string is empty, there's no formula.
	 */
	std::string formula_;

	/** If there's no formula it contains the value. */
	T value_;
};

template<typename T>
typed_formula<T>::typed_formula(const std::string& str, const T value)
	: formula_(), value_(value)
{
	if(str.empty()) {
		return;
	}

	if(str[0] == '(') {
		formula_ = str;
	} else {
		convert(str);
	}
}

template<typename T>
inline T typed_formula<T>::
operator()(const wfl::map_formula_callable& variables, wfl::function_symbol_table* functions) const
{
	if(!has_formula()) {
		return value_;
	}

	wfl::variant v = wfl::formula(formula_, functions).evaluate(variables);
	const T& result = execute(v);

	LOG_GUI_D << "Formula: execute '" << formula_ << "' result '" << result << "'.\n";

	return result;
}

/**
 * Template specializations.
 *
 * Each type must have an @ref execute specialization, and optionally one for @ref convert.
 */

template<>
inline bool typed_formula<bool>::execute(wfl::variant& v) const
{
	return v.as_bool();
}

template<>
inline void typed_formula<bool>::convert(const std::string& str)
{
	value_ = utils::string_bool(str);
}


template<>
inline int typed_formula<int>::execute(wfl::variant& v) const
{
	return v.as_int();
}


template<>
inline unsigned typed_formula<unsigned>::execute(wfl::variant& v) const
{
	// FIXME: Validate this? As is, the formula could return a negative number which is blindly converted to unsigned.
	// Unfortunately, some places rely on this happening for diagnostic messages...
	return v.as_int();
}


template<>
inline std::string typed_formula<std::string>::execute(wfl::variant& v) const
{
	return v.as_string();
}

template<>
inline void typed_formula<std::string>::convert(const std::string& str)
{
	value_ = str;
}


template<>
inline t_string typed_formula<t_string>::execute(wfl::variant& v) const
{
	return v.as_string();
}

template<>
inline void typed_formula<t_string>::convert(const std::string& str)
{
	value_ = str;
}


template<>
inline PangoAlignment typed_formula<PangoAlignment>::execute(wfl::variant& v) const
{
	return decode_text_alignment(v.as_string());
}

template<>
inline void typed_formula<PangoAlignment>::convert(const std::string& str)
{
	value_ = decode_text_alignment(str);
}


template<>
inline color_t typed_formula<color_t>::execute(wfl::variant& v) const
{
	const auto& result = v.as_list();
	const int alpha = result.size() == 4 ? result[3].as_int() : ALPHA_OPAQUE;

	return color_t(
		result.at(0).as_int(),
		result.at(1).as_int(),
		result.at(2).as_int(),
		alpha
	);
}

template<>
inline void typed_formula<color_t>::convert(const std::string& str)
{
	value_ = color_t::from_rgba_string(str);
}


template<typename T>
inline T typed_formula<T>::execute(wfl::variant& /*v*/) const
{
	// Every type needs its own execute function avoid instantiation of the
	// default execute.
	static_assert(sizeof(T) == 0, "typed_formula: Missing execute specialization");
	return T();
}

template<class T>
inline void typed_formula<T>::convert(const std::string& str)
{
	value_ = lexical_cast_default<T>(str);
}

} // namespace gui2
