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
 * @file formula_debugger.cpp
 * Formula debugger - implementation
 * */


#include "formula_debugger.hpp"
#include "formula.hpp"
#include "formula_function.hpp"
#include "log.hpp"

#include <boost/lexical_cast.hpp>

static lg::log_domain log_formula_debugger("ai/debug/formula");
#define DBG_FDB LOG_STREAM(debug, log_formula_debugger)
#define LOG_FDB LOG_STREAM(info, log_formula_debugger)
#define WRN_FDB LOG_STREAM(warn, log_formula_debugger)
#define ERR_FDB LOG_STREAM(err, log_formula_debugger)

namespace game_logic {

formula_debugger::formula_debugger()
	: counter_(0)
{
}


formula_debugger::~formula_debugger()
{
}


variant formula_debugger::evaluate_arg_callback(formula_expression &/*expression*/, const formula_callable &/*variables*/)
{
	int counter = counter_++;
	DBG_FDB << "#"<< counter <<": evaluating \"" << /*expression.to_debug_string() << */  "\"" <<std::endl;
	variant v; //= expression.evaluate(variables,this); //work-in-progress
	DBG_FDB << "#"<< counter <<": evaluated  \""  << /*expression.to_debug_string() << */ "\" to " << v.to_debug_string(NULL,true) << std::endl;
	return v;
}

} // end of namespace game_logic
