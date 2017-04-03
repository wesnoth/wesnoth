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

#ifndef GUI_WIDGETS_FORMULA_HPP_INCLUDED
#define GUI_WIDGETS_FORMULA_HPP_INCLUDED

#include "formula/callable.hpp"
#include "formula/function.hpp"
#include "formula/formula.hpp"
#include "gui/core/log.hpp"
#include "gui/widgets/helper.hpp"
#include "lexical_cast.hpp"
#include "serialization/string_utils.hpp"
#include "tstring.hpp"
#include "color.hpp"

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
template <class T>
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
	 * @param variables           The variables, which can be used during the
	 *                            evaluation of the formula.
	 * @param functions           The variables, which can be called during the
	 *                            evaluation of the formula. (Note is is also
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
	 * @param variables           The state variables which might be used in
	 *                            the formula. For example a screen_width can
	 *                            be set so the formula can return the half
	 *                            width of the screen.
	 * @param functions           The variables, which can be called during the
	 *                            evaluation of the formula. (Note is is also
	 *                            possible to add extra functions to the table,
	 *                            when the variable is not @c nullptr.
	 *
	 * @returns                   The calculated value.
	 */
	T execute(const wfl::map_formula_callable& variables,
			  wfl::function_symbol_table* functions) const;

	/**
	 * Contains the formula for the variable.
	 *
	 * If the string is empty, there's no formula.
	 */
	std::string formula_;

	/** If there's no formula it contains the value. */
	T value_;
};

template <class T>
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

template <class T>
inline T typed_formula<T>::
operator()(const wfl::map_formula_callable& variables,
		   wfl::function_symbol_table* functions) const
{
	if(has_formula()) {
		const T& result = execute(variables, functions);
		LOG_GUI_D << "Formula: execute '" << formula_ << "' result '" << result
				  << "'.\n";
		return result;
	} else {
		return value_;
	}
}

template <>
inline bool
typed_formula<bool>::execute(const wfl::map_formula_callable& variables,
						wfl::function_symbol_table* functions) const
{
	return wfl::formula(formula_, functions)
			.evaluate(variables)
			.as_bool();
}

template <>
inline int
typed_formula<int>::execute(const wfl::map_formula_callable& variables,
					   wfl::function_symbol_table* functions) const
{
	return wfl::formula(formula_, functions)
			.evaluate(variables)
			.as_int();
}

template <>
inline unsigned
typed_formula<unsigned>::execute(const wfl::map_formula_callable& variables,
							wfl::function_symbol_table* functions) const
{
	return wfl::formula(formula_, functions)
			.evaluate(variables)
			.as_int();
}

template <>
inline std::string typed_formula<std::string>::execute(
		const wfl::map_formula_callable& variables,
		wfl::function_symbol_table* functions) const
{
	return wfl::formula(formula_, functions)
			.evaluate(variables)
			.as_string();
}

template <>
inline t_string
typed_formula<t_string>::execute(const wfl::map_formula_callable& variables,
							wfl::function_symbol_table* functions) const
{
	return wfl::formula(formula_, functions)
			.evaluate(variables)
			.as_string();
}

template <>
inline PangoAlignment typed_formula<PangoAlignment>::execute(
		const wfl::map_formula_callable& variables,
		wfl::function_symbol_table* functions) const
{
	return decode_text_alignment(wfl::formula(formula_, functions)
										 .evaluate(variables)
										 .as_string());
}

template<>
inline color_t typed_formula<color_t>::execute(
		const wfl::map_formula_callable& variables,
		wfl::function_symbol_table* functions) const
{
	const wfl::variant v = wfl::formula(formula_, functions).evaluate(variables);
	const auto& result = v.as_list();
	int alpha = result.size() == 4 ? result[3].as_int() : ALPHA_OPAQUE;
	return color_t(result.at(0).as_int(), result.at(1).as_int(), result.at(2).as_int(), alpha);
}

template <class T>
inline T
typed_formula<T>::execute(const wfl::map_formula_callable& /*variables*/,
					 wfl::function_symbol_table* /*functions*/) const
{
	// Every type needs its own execute function avoid instantiation of the
	// default execute.
	static_assert(sizeof(T) == 0, "typed_formula: Missing execute specialization");
	return T();
}

template <>
inline void typed_formula<bool>::convert(const std::string& str)
{
	value_ = utils::string_bool(str);
}

template <>
inline void typed_formula<std::string>::convert(const std::string& str)
{
	value_ = str;
}

template <>
inline void typed_formula<t_string>::convert(const std::string& str)
{
	value_ = str;
}

template <>
inline void typed_formula<PangoAlignment>::convert(const std::string& str)
{
	value_ = decode_text_alignment(str);
}

template <>
inline void typed_formula<color_t>::convert(const std::string& str)
{
	value_ = color_t::from_rgba_string(str);
}

template <class T>
inline void typed_formula<T>::convert(const std::string& str)
{
	value_ = lexical_cast_default<T>(str);
}

} // namespace gui2

#endif
