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

debug_info::debug_info()
	: arg_number_(-1),f_name_(""),valid_(false)
{
}


debug_info::debug_info(int arg_number, const char *f_name, bool valid)
	: arg_number_(arg_number), f_name_(f_name), valid_(valid)
{
}


debug_info::~debug_info()
{
}


const char* debug_info::name()
{
	if (valid_ && (f_name_!=NULL) ) {
		return f_name_;
	} else {
		return "";
	}
}


void debug_info::invalidate()
{
	valid_ = false;
}


formula_debugger::formula_debugger()
	: counter_(0), info_()
{
}


formula_debugger::~formula_debugger()
{
}


static void msg(int counter, const char *act, const char *name, const char *formula_str, const char *to="", const char *result = "")
{
	DBG_FDB << "#" << counter << act << std::endl <<"     \""<< name << "\"='" << formula_str << "' " << to << result << std::endl;
}


void formula_debugger::add_debug_info(int arg_number, const char *f_name)
{
	info_ = debug_info(arg_number,f_name, true);
}


variant formula_debugger::evaluate_arg_callback(const formula_expression &expression, const formula_callable &variables)
{
	int counter = counter_++;
	debug_info i = info_;
	info_.invalidate();
	msg(counter," evaluating expression: ",i.name(),"");
	variant v = expression.execute(variables,this); //work-in-progress
	msg(counter," evaluated expression: ",i.name(),""," to ",v.to_debug_string(NULL,true).c_str());
	return v;
}


variant formula_debugger::evaluate_formula_callback(const formula &f, const formula_callable &variables)
{
	int counter = counter_++;
	debug_info i = info_;
	info_.invalidate();
	msg(counter," evaluating formula: ",i.name(),f.str().c_str());
	variant v = f.execute(variables,this); //work-in-progress
	msg(counter," evaluated formula: ",i.name(),f.str().c_str()," to ",v.to_debug_string(NULL,true).c_str());
	return v;
}


variant formula_debugger::evaluate_formula_callback(const formula &f)
{
	int counter = counter_++;
	debug_info i = info_;
	info_.invalidate();
	msg(counter," evaluating formula without variables: ",i.name(),f.str().c_str());
	variant v = f.execute(this); //work-in-progress
	msg(counter," evaluating formula without variables: ",i.name(),f.str().c_str(),v.to_debug_string(NULL,true).c_str());
	return v;
}


} // end of namespace game_logic
