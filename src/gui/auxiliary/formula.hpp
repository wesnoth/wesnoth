/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
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
#include "serialization/string_utils.hpp"
#include "util.hpp"
#include "tstring.hpp"

#include <boost/static_assert.hpp>

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
class tformula
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
	explicit tformula<T>(const std::string& str, const T value = T());

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
	 *                            when the variable is not @c NULL.
	 *
	 * @returns                   The stored result or the result of the
	 *                            evaluation of the formula.
	 */
	T operator()(const game_logic::map_formula_callable& variables,
				 game_logic::function_symbol_table* functions = NULL) const;

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
	 *                            when the variable is not @c NULL.
	 *
	 * @returns                   The calculated value.
	 */
	T execute(const game_logic::map_formula_callable& variables,
			  game_logic::function_symbol_table* functions) const;

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
tformula<T>::tformula(const std::string& str, const T value)
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
inline T tformula<T>::
operator()(const game_logic::map_formula_callable& variables,
		   game_logic::function_symbol_table* functions) const
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
tformula<bool>::execute(const game_logic::map_formula_callable& variables,
						game_logic::function_symbol_table* functions) const
{
	return game_logic::formula(formula_, functions)
			.evaluate(variables)
			.as_bool();
}

template <>
inline int
tformula<int>::execute(const game_logic::map_formula_callable& variables,
					   game_logic::function_symbol_table* functions) const
{
	return game_logic::formula(formula_, functions)
			.evaluate(variables)
			.as_int();
}

template <>
inline unsigned
tformula<unsigned>::execute(const game_logic::map_formula_callable& variables,
							game_logic::function_symbol_table* functions) const
{
	return game_logic::formula(formula_, functions)
			.evaluate(variables)
			.as_int();
}

template <>
inline std::string tformula<std::string>::execute(
		const game_logic::map_formula_callable& variables,
		game_logic::function_symbol_table* functions) const
{
	return game_logic::formula(formula_, functions)
			.evaluate(variables)
			.as_string();
}

template <>
inline t_string
tformula<t_string>::execute(const game_logic::map_formula_callable& variables,
							game_logic::function_symbol_table* functions) const
{
	return game_logic::formula(formula_, functions)
			.evaluate(variables)
			.as_string();
}

template <>
inline PangoAlignment tformula<PangoAlignment>::execute(
		const game_logic::map_formula_callable& variables,
		game_logic::function_symbol_table* functions) const
{
	return decode_text_alignment(game_logic::formula(formula_, functions)
										 .evaluate(variables)
										 .as_string());
}

template <class T>
inline T
tformula<T>::execute(const game_logic::map_formula_callable& /*variables*/
					 ,
					 game_logic::function_symbol_table* /*functions*/) const
{
	// Every type needs its own execute function avoid instantiation of the
	// default execute.
	BOOST_STATIC_ASSERT(sizeof(T) == 0);
	return T();
}

template <>
inline void tformula<bool>::convert(const std::string& str)
{
	value_ = utils::string_bool(str);
}

template <>
inline void tformula<std::string>::convert(const std::string& str)
{
	value_ = str;
}

template <>
inline void tformula<t_string>::convert(const std::string& str)
{
	value_ = str;
}

template <>
inline void tformula<PangoAlignment>::convert(const std::string& str)
{
	value_ = decode_text_alignment(str);
}

template <class T>
inline void tformula<T>::convert(const std::string& str)
{
	value_ = lexical_cast_default<T>(str);
}

} // namespace gui2

#endif
