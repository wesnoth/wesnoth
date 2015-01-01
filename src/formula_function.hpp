
/*
   Copyright (C) 2008 - 2015 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef FORMULA_FUNCTION_HPP_INCLUDED
#define FORMULA_FUNCTION_HPP_INCLUDED

#include "formula.hpp"
#include "formula_callable.hpp"

namespace game_logic {

class formula_expression {
public:
	formula_expression() : name_("") {}
	virtual ~formula_expression() {}
	variant evaluate(const formula_callable& variables, formula_debugger *fdb = NULL) const {
		call_stack_manager manager(name_);
		if (fdb!=NULL) {
			return evaluate_arg_callback(*fdb,*this,variables);
		} else {
			return execute(variables,fdb);
		}
	}
	void set_name(const char* name) { name_ = name; }

	const char* get_name() const { return name_; }
	virtual std::string str() const = 0;
private:
	virtual variant execute(const formula_callable& variables, formula_debugger *fdb = NULL) const = 0;
	const char* name_;
        friend class formula_debugger;
};

typedef boost::shared_ptr<formula_expression> expression_ptr;

class function_expression : public formula_expression {
public:
	typedef std::vector<expression_ptr> args_list;
	explicit function_expression(
	                    const std::string& name,
	                    const args_list& args,
	                    int min_args=-1, int max_args=-1)
	    : name_(name), args_(args)
	{
		set_name(name.c_str());
		if(min_args >= 0 && args_.size() < static_cast<size_t>(min_args)) {
			throw formula_error("Too few arguments", "", "", 0);
		}

		if(max_args >= 0 && args_.size() > static_cast<size_t>(max_args)) {
			throw formula_error("Too many arguments", "", "", 0);
		}
	}
	virtual std::string str() const;
protected:
	const args_list& args() const { return args_; }
private:
	std::string name_;
	args_list args_;
};

class key_value_pair : public formula_callable {
	variant key_;
	variant value_;

	variant get_value(const std::string& key) const;

	void get_inputs(std::vector<game_logic::formula_input>* inputs) const;
public:
	explicit key_value_pair(const variant& key, const variant& value) : key_(key), value_(value) {}
};

class formula_function_expression : public function_expression {
public:
	explicit formula_function_expression(const std::string& name, const args_list& args, const_formula_ptr formula, const_formula_ptr precondition, const std::vector<std::string>& arg_names);
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const;
	const_formula_ptr formula_;
	const_formula_ptr precondition_;
	std::vector<std::string> arg_names_;
	int star_arg_;
};

typedef boost::shared_ptr<function_expression> function_expression_ptr;

class formula_function {
	std::string name_;
	const_formula_ptr formula_;
	const_formula_ptr precondition_;
	std::vector<std::string> args_;
public:
	formula_function() :
		name_(),
		formula_(),
		precondition_(),
		args_()
	{
	}

	formula_function(const std::string& name, const_formula_ptr formula, const_formula_ptr precondition, const std::vector<std::string>& args) : name_(name), formula_(formula), precondition_(precondition), args_(args)
	{}

	function_expression_ptr generate_function_expression(const std::vector<expression_ptr>& args) const;
};

class function_symbol_table {
	std::map<std::string, formula_function> custom_formulas_;
public:
	function_symbol_table() :
		custom_formulas_()
	{
	}

	virtual ~function_symbol_table() {}
	virtual void add_formula_function(const std::string& name, const_formula_ptr formula, const_formula_ptr precondition, const std::vector<std::string>& args);
	virtual expression_ptr create_function(const std::string& fn,
					                       const std::vector<expression_ptr>& args) const;
	std::vector<std::string> get_function_names() const;
};

expression_ptr create_function(const std::string& fn,
                               const std::vector<expression_ptr>& args,
							   const function_symbol_table* symbols);
std::vector<std::string> builtin_function_names();


class wrapper_formula : public formula_expression {
public:
	wrapper_formula()
		: arg_()
	{
	}

	wrapper_formula(expression_ptr arg)
		: arg_(arg)
	{
	}

	virtual ~wrapper_formula()
	{
	}

	virtual std::string str() const
	{
		if (arg_) {
			return arg_->str();
		} else {
			return "";
		}
	}
private:
	virtual variant execute(const formula_callable& variables, formula_debugger *fdb = NULL) const
	{
		if (arg_) {
			return arg_->evaluate(variables,fdb);
		} else {
			return variant();
		}
	}
	expression_ptr arg_;
};

}

#endif
