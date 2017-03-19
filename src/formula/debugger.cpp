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
 * Formula debugger - implementation
 * */


#include "formula/debugger.hpp"
#include "formula/formula.hpp"
#include "formula/function.hpp"
#include "game_display.hpp"
#include "log.hpp"
#include "resources.hpp"
#include "gui/dialogs/formula_debugger.hpp"
#include "gui/widgets/settings.hpp"

static lg::log_domain log_formula_debugger("scripting/formula/debug");
#define DBG_FDB LOG_STREAM(debug, log_formula_debugger)
#define LOG_FDB LOG_STREAM(info, log_formula_debugger)
#define WRN_FDB LOG_STREAM(warn, log_formula_debugger)
#define ERR_FDB LOG_STREAM(err, log_formula_debugger)

namespace game_logic {


debug_info::debug_info(int arg_number, int counter, int level, const std::string &name, const std::string &str, const variant &value, bool evaluated)
	: arg_number_(arg_number), counter_(counter), level_(level), name_(name), str_(str), value_(value), evaluated_(evaluated)
{
}


debug_info::~debug_info()
{
}


int debug_info::level() const
{
	return level_;
}

const std::string& debug_info::name() const
{
	return name_;
}


int debug_info::counter() const
{
	return counter_;
}


const variant& debug_info::value() const
{
	return value_;
}


void debug_info::set_value(const variant &value)
{
	value_ = value;
}


bool debug_info::evaluated() const
{
	return evaluated_;
}


void debug_info::set_evaluated(bool evaluated)
{
	evaluated_ = evaluated;
}


const std::string& debug_info::str() const
{
	return str_;
}


formula_debugger::formula_debugger()
	: call_stack_(), counter_(0), current_breakpoint_(), breakpoints_(), execution_trace_(),arg_number_extra_debug_info(-1), f_name_extra_debug_info("")
{
	add_breakpoint_step_into();
	add_breakpoint_continue_to_end();
}


formula_debugger::~formula_debugger()
{
}


static void msg(const char *act, debug_info &i, const char *to="", const char *result = "")
{
	DBG_FDB << "#" << i.counter() << act << std::endl <<"     \""<< i.name() << "\"='" << i.str() << "' " << to << result << std::endl;
}


void formula_debugger::add_debug_info(int arg_number, const std::string& f_name)
{
	arg_number_extra_debug_info = arg_number;
	f_name_extra_debug_info = f_name;
}


const std::deque<debug_info>& formula_debugger::get_call_stack() const
{
	return call_stack_;
}


const breakpoint_ptr formula_debugger::get_current_breakpoint() const
{
	return current_breakpoint_;
}

const std::deque<debug_info>& formula_debugger::get_execution_trace() const
{
	return execution_trace_;
}

void formula_debugger::check_breakpoints()
{
	for( std::deque< breakpoint_ptr >::iterator b = breakpoints_.begin(); b!= breakpoints_.end(); ++b) {
		if ((*b)->is_break_now()){
			current_breakpoint_ = (*b);
			show_gui();
			current_breakpoint_ = breakpoint_ptr();
			if ((*b)->is_one_time_only()) {
				breakpoints_.erase(b);
			}
			break;
		}
	}
}

void formula_debugger::show_gui()
{
	if (resources::screen == nullptr) {
		WRN_FDB << "do not showing debug window due to nullptr gui" << std::endl;
		return;
	}
	if (game_config::debug) {
		gui2::dialogs::formula_debugger debug_dialog(*this);
		debug_dialog.show(resources::screen->video());
	} else {
		WRN_FDB << "do not showing debug window due to disabled --new-widgets"<< std::endl;
	}
}

void formula_debugger::call_stack_push(const std::string &str)
{
	call_stack_.push_back(debug_info(arg_number_extra_debug_info,counter_++,call_stack_.size(),f_name_extra_debug_info,str,variant(),false));
	arg_number_extra_debug_info = -1;
	f_name_extra_debug_info = "";
	execution_trace_.push_back(call_stack_.back());
}


void formula_debugger::call_stack_pop()
{
	execution_trace_.push_back(call_stack_.back());
	call_stack_.pop_back();
}


void formula_debugger::call_stack_set_evaluated(bool evaluated)
{
	call_stack_.back().set_evaluated(evaluated);
}

void formula_debugger::call_stack_set_value(const variant &v)
{
	call_stack_.back().set_value(v);
}

variant formula_debugger::evaluate_arg_callback(const formula_expression &expression, const formula_callable &variables)
{
	call_stack_push(expression.str());
	check_breakpoints();
	msg(" evaluating expression: ",call_stack_.back());
	variant v = expression.execute(variables,this);
	call_stack_set_value(v);
	call_stack_set_evaluated(true);
	msg(" evaluated expression: ",call_stack_.back()," to ",v.to_debug_string(nullptr,true).c_str());
	check_breakpoints();
	call_stack_pop();
	return v;
}


variant formula_debugger::evaluate_formula_callback(const formula &f, const formula_callable &variables)
{
	call_stack_push(f.str());
	check_breakpoints();
	msg(" evaluating formula: ",call_stack_.back());
	variant v = f.execute(variables,this);
	call_stack_set_value(v);
	call_stack_set_evaluated(true);
	msg(" evaluated formula: ",call_stack_.back()," to ",v.to_debug_string(nullptr,true).c_str());
	check_breakpoints();
	call_stack_pop();
	return v;
}


variant formula_debugger::evaluate_formula_callback(const formula &f)
{
	call_stack_push(f.str());
	check_breakpoints();
	msg(" evaluating formula without variables: ",call_stack_.back());
	variant v = f.execute(this);
	call_stack_set_value(v);
	call_stack_set_evaluated(true);
	msg(" evaluated formula without variables: ",call_stack_.back()," to ",v.to_debug_string(nullptr,true).c_str());
	check_breakpoints();
	call_stack_pop();
	return v;
}


base_breakpoint::base_breakpoint(formula_debugger &fdb, const std::string &name, bool one_time_only)
	: fdb_(fdb), name_(name), one_time_only_(one_time_only)
{

}


base_breakpoint::~base_breakpoint()
{
}


bool base_breakpoint::is_one_time_only() const
{
	return one_time_only_;
}


const std::string& base_breakpoint::name() const
{
	return name_;
}

class end_breakpoint : public base_breakpoint {
public:
	end_breakpoint(formula_debugger &fdb)
		: base_breakpoint(fdb,"End", true)
	{
	}

	virtual ~end_breakpoint()
	{
	}

	virtual bool is_break_now() const
	{
		const std::deque<debug_info> &call_stack = fdb_.get_call_stack();
		if ((call_stack.size() == 1) && (call_stack[0].evaluated()) ) {
			return true;
		}
		return false;
	}
};


class step_in_breakpoint : public base_breakpoint {
public:
	step_in_breakpoint(formula_debugger &fdb)
		: base_breakpoint(fdb,"Step",true)
	{
	}

	virtual ~step_in_breakpoint()
	{
	}

	virtual bool is_break_now() const
	{
		const std::deque<debug_info> &call_stack = fdb_.get_call_stack();
		if (call_stack.empty() || call_stack.back().evaluated()) {
			return false;
		}

		return true;
	}
};


class step_out_breakpoint : public base_breakpoint {
public:
	step_out_breakpoint(formula_debugger &fdb)
		: base_breakpoint(fdb,"Step out",true), level_(fdb.get_call_stack().size()-1)
	{
	}

	virtual ~step_out_breakpoint()
	{
	}

	virtual bool is_break_now() const
	{
		const std::deque<debug_info> &call_stack = fdb_.get_call_stack();
		if (call_stack.empty() || call_stack.back().evaluated()) {
			return false;
		}

		if (call_stack.size() == level_) {
			return true;
		}
		return false;
	}
private:
	size_t level_;
};


class next_breakpoint : public base_breakpoint {
public:
	next_breakpoint(formula_debugger &fdb)
		: base_breakpoint(fdb,"Next",true), level_(fdb.get_call_stack().size())
	{
	}

	virtual ~next_breakpoint()
	{
	}

	virtual bool is_break_now() const
	{
		const std::deque<debug_info> &call_stack = fdb_.get_call_stack();
		if (call_stack.empty() || call_stack.back().evaluated()) {
			return false;
		}
		if (call_stack.size() == level_) {
			return true;
		}
		return false;
	}
private:
	size_t level_;
};


void formula_debugger::add_breakpoint_continue_to_end()
{
	breakpoints_.push_back(breakpoint_ptr(new end_breakpoint(*this)));
	LOG_FDB << "added 'end' breakpoint"<< std::endl;
}


void formula_debugger::add_breakpoint_step_into()
{
	breakpoints_.push_back(breakpoint_ptr(new step_in_breakpoint(*this)));
	LOG_FDB << "added 'step into' breakpoint"<< std::endl;
}


void formula_debugger::add_breakpoint_step_out()
{
	breakpoints_.push_back(breakpoint_ptr(new step_out_breakpoint(*this)));
	LOG_FDB << "added 'step out' breakpoint"<< std::endl;
}


void formula_debugger::add_breakpoint_next()
{
	breakpoints_.push_back(breakpoint_ptr(new next_breakpoint(*this)));
	LOG_FDB << "added 'next' breakpoint"<< std::endl;
}


} // end of namespace game_logic
