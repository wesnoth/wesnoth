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

class debug_info {
public:
	debug_info();
	debug_info(int arg_number, const char *f_name, bool valid);
	virtual ~debug_info();
	const char* name();
	void invalidate();
private:
	int arg_number_;
	const char *f_name_;
	bool valid_;
};

class formula_debugger {
public:
	formula_debugger();

	virtual ~formula_debugger();


	virtual void add_debug_info(int arg_number, const char *f_name);


	virtual variant evaluate_arg_callback(const formula_expression &expression, const formula_callable &variables);


	virtual variant evaluate_formula_callback(const formula &f, const formula_callable &variables);


	virtual variant evaluate_formula_callback(const formula &f);


	static formula_debugger* add_debug_info(formula_debugger *fdb, int arg_number, const char *f_name)
	{
		if (fdb==NULL) {
			return NULL;
		}
		fdb->add_debug_info(arg_number,f_name);
		return fdb;
	}

private:
	int counter_;
	debug_info info_;
};



} // end of namespace game_logic

#endif
