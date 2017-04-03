/*
   Copyright (C) 2009 - 2017 by Yurii Chernyi <terraninfo@terraninfo.net>
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
 * Formula AI debugger
 *
 */

#ifndef FORMULA_DEBUGGER_HPP_INCLUDED
#define FORMULA_DEBUGGER_HPP_INCLUDED

#include "formula/variant.hpp"
#include "formula/debugger_fwd.hpp"
#include <list>

namespace wfl {

class formula_expression;
class formula_callable;
class formula;
class formula_debugger;

class debug_info {
public:
	debug_info(int arg_number, int counter, int level, const std::string &name, const std::string &str, const variant &value, bool evaluated);
	virtual ~debug_info();
	int counter() const;
	int level() const;
	const std::string& name() const;
	const std::string& str() const;
	const variant& value() const;
	const std::string& value_str() const;
	bool evaluated() const;
	void set_evaluated(bool evaluated);
	void set_value(const variant &value);
private:
	int arg_number_;
	int counter_;
	int level_;
	std::string name_;
	std::string str_;
	variant value_;
	bool evaluated_;

};

class base_breakpoint {
public:
	base_breakpoint(formula_debugger &fdb, const std::string &name, bool one_time_only);
	virtual ~base_breakpoint();
	virtual bool is_break_now() const = 0;
	bool is_one_time_only() const;
	const std::string &name() const;
protected:
	formula_debugger &fdb_;
	std::string name_;
	bool one_time_only_;

};


class formula_debugger {
public:
	formula_debugger();


	virtual ~formula_debugger();


	void add_debug_info(int arg_number, const std::string& f_name);


	void call_stack_push(const std::string &str);


	void call_stack_pop();


	void call_stack_set_evaluated(bool evaluated);


	void call_stack_set_value(const variant &v);


	void check_breakpoints();


	const std::list<debug_info>& get_call_stack() const;


	const breakpoint_ptr get_current_breakpoint() const;


	const std::list<debug_info>& get_execution_trace() const;


	variant evaluate_arg_callback(const formula_expression &expression, const formula_callable &variables);


	variant evaluate_formula_callback(const formula &f, const formula_callable &variables);


	variant evaluate_formula_callback(const formula &f);


	void show_gui();


	void add_breakpoint_continue_to_end();


	void add_breakpoint_step_into();


	void add_breakpoint_step_out();


	void add_breakpoint_next();


	//static functions

	static formula_debugger* add_debug_info(formula_debugger *fdb, int arg_number, const std::string& f_name)
	{
		if (fdb==nullptr) {
			return nullptr;
		}
		fdb->add_debug_info(arg_number,f_name);
		return fdb;
	}

private:
	std::list<debug_info> call_stack_;
	int counter_;
	breakpoint_ptr current_breakpoint_;
	std::list< breakpoint_ptr > breakpoints_;
	std::list<debug_info> execution_trace_;
	int arg_number_extra_debug_info;
	std::string f_name_extra_debug_info;


};



} // end of namespace wfl

#endif
