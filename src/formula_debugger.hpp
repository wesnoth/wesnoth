/* $Id$ */
/*
   Copyright (C) 2009 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file formula_debugger.hpp
 * Formula AI debugger
 *
 */


#ifndef FORMULA_DEBUGGER_HPP_INCLUDED
#define FORMULA_DEBUGGER_HPP_INCLUDED

#include "global.hpp"

#include "variant.hpp"

namespace game_logic {

class formula_expression;
class formula_callable;
class formula;

class formula_debugger {
public:
	formula_debugger();

	virtual ~formula_debugger();

	virtual variant evaluate_arg_callback(const formula_expression &expression, const formula_callable &variables);

	virtual variant evaluate_formula_callback(const formula &f, const formula_callable &variables);

	virtual variant evaluate_formula_callback(const formula &f);
private:
	int counter_;
};

} // end of namespace game_logic

#endif
