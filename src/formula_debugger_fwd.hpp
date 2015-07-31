/*
   Copyright (C) 2009 - 2015 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Formula AI debugger, forward
 *
 */


#ifndef FORMULA_DEBUGGER_FWD_HPP_INCLUDED
#define FORMULA_DEBUGGER_FWD_HPP_INCLUDED

#include "global.hpp"

#include <boost/shared_ptr.hpp>


class variant;

namespace game_logic {

class formula_debugger;
class debug_info;
class formula_expression;
class formula;
class formula_callable;

class base_breakpoint;

typedef boost::shared_ptr<base_breakpoint> breakpoint_ptr;

formula_debugger* add_debug_info(formula_debugger *fdb, int arg_number, const char *f_name);

variant evaluate_arg_callback(formula_debugger &fdb, const formula_expression &expression, const formula_callable &variables);

variant evaluate_formula_callback(formula_debugger &fdb, const formula &f, const formula_callable &variables);

variant evaluate_formula_callback(formula_debugger &fdb, const formula &f);


} // end of namespace game_logic

#endif
