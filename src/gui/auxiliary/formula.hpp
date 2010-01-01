/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_FORMULA_HPP_INCLUDED
#define GUI_WIDGETS_FORMULA_HPP_INCLUDED

#include "formula_callable.hpp"
#include "../../formula.hpp"
#include "serialization/string_utils.hpp"
#include "util.hpp"

#include <boost/static_assert.hpp>

#include <cassert>

namespace gui2{

/** Template class can hold a value or a formula calculating the value. */
template <class T>
class tformula
{
public:
	tformula<T>(const std::string& str, const T value = T());

	/**
	 * Returns the value, can only be used it the data is no formula.
	 *
	 *  Another option would be to cache the output of the formula in value_
	 *  and always allow this function. But for now decided that the caller
	 *  needs to do the caching. It might be changed later.
	 */
	T operator()() const
	{
		assert(!has_formula());
		return value_;
	}

	/** Returns the value, can always be used. */
	T operator() (const game_logic::map_formula_callable& variables) const;

	/** Determine whether the class contains a formula. */
	bool has_formula() const { return !formula_.empty(); }

private:

	/** Converts the string ot the template value. */
	void convert(const std::string& str);

	T execute(const game_logic::map_formula_callable& variables) const;

	/** If there is a formula it's stored in this string, empty if no formula. */
	std::string formula_;

	/** If no formula it contains the value. */
	T value_;
};

template<class T>
tformula<T>::tformula(const std::string& str, const T value) :
	formula_(),
	value_(value)
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

template<class T>
inline T tformula<T>::operator() (const game_logic::map_formula_callable& variables) const
{
	if(has_formula()) {
		const T& result = execute(variables);
#if 0
		LOG_GUI_D << "Formula: execute '" << formula_
			<< "' result '" << result
			<< "'.\n";
#endif
		return result;
	} else {
		return value_;
	}
}

template<>
inline bool tformula<bool>::execute(const game_logic::map_formula_callable& variables) const
{
	return game_logic::formula(formula_).evaluate(variables).as_bool();
}

template<>
inline int tformula<int>::execute(const game_logic::map_formula_callable& variables) const
{
	return game_logic::formula(formula_).evaluate(variables).as_int();
}

template<>
inline unsigned tformula<unsigned>::execute(const game_logic::map_formula_callable& variables) const
{
	return game_logic::formula(formula_).evaluate(variables).as_int();
}

template<>
inline std::string tformula<std::string>::execute(const game_logic::map_formula_callable& variables) const
{
	return game_logic::formula(formula_).evaluate(variables).as_string();
}

template<>
inline t_string tformula<t_string>::execute(const game_logic::map_formula_callable& variables) const
{
	return game_logic::formula(formula_).evaluate(variables).as_string();
}

template<class T>
inline T tformula<T>::execute(const game_logic::map_formula_callable& variables) const
{
	// Every type needs it's own execute function avoid instantiation of the
	// default execute.
	BOOST_STATIC_ASSERT(sizeof(T) == 0);
	return T();
}

template<>
inline void tformula<bool>::convert(const std::string& str)
{
	value_ = utils::string_bool(str);
}

template<>
inline void tformula<std::string>::convert(const std::string& str)
{
	value_ = str;
}

template<>
inline void tformula<t_string>::convert(const std::string& str)
{
	value_ = str;
}

template<class T>
inline void tformula<T>::convert(const std::string& str)
{
	value_ = lexical_cast_default<T>(str);
}

} // namespace gui2

#endif
