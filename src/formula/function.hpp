
/*
   Copyright (C) 2008 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "formula/formula.hpp"
#include "formula/callable.hpp"

#include <set>
#include <string>

namespace wfl {

struct call_stack_manager {
	explicit call_stack_manager(const std::string& str);
	~call_stack_manager();
	static std::string get();
};

class formula_expression {
public:
	explicit formula_expression(const std::string& name = "") : name_(name) {}
	virtual ~formula_expression() {}
	variant evaluate(const formula_callable& variables, formula_debugger *fdb = nullptr) const {
		call_stack_manager manager(name_);
		if (fdb!=nullptr) {
			return evaluate_arg_callback(*fdb,*this,variables);
		} else {
			return execute(variables,fdb);
		}
	}

	std::string get_name() const { return name_; }
	virtual std::string str() const = 0;
private:
	virtual variant execute(const formula_callable& variables, formula_debugger *fdb = nullptr) const = 0;
	const std::string name_;
	friend class formula_debugger;
};

typedef std::shared_ptr<formula_expression> expression_ptr;

class function_expression : public formula_expression {
public:
	typedef std::vector<expression_ptr> args_list;
	explicit function_expression(
	                    const std::string& name,
	                    const args_list& args,
	                    int min_args=-1, int max_args=-1)
	    : formula_expression(name), args_(args)
	{
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
	args_list args_;
};

class key_value_pair : public formula_callable {
	variant key_;
	variant value_;

	variant get_value(const std::string& key) const override;

	void get_inputs(formula_input_vector& inputs) const override;
public:
	explicit key_value_pair(const variant& key, const variant& value) : key_(key), value_(value) {}

	void serialize_to_string(std::string& str) const override;
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

typedef std::shared_ptr<function_expression> function_expression_ptr;

class formula_function {
protected:
	std::string name_;
public:
	formula_function(const std::string name) : name_(name) {}
	virtual function_expression_ptr generate_function_expression(const std::vector<expression_ptr>& args) const = 0;
	virtual ~formula_function() {}
};

class user_formula_function : public formula_function {
	const_formula_ptr formula_;
	const_formula_ptr precondition_;
	std::vector<std::string> args_;
public:
	user_formula_function(const std::string& name, const_formula_ptr formula, const_formula_ptr precondition, const std::vector<std::string>& args)
		: formula_function(name)
		, formula_(formula)
		, precondition_(precondition)
		, args_(args)
	{}

	function_expression_ptr generate_function_expression(const std::vector<expression_ptr>& args) const;
};

template<typename T>
class builtin_formula_function : public formula_function {
public:
	builtin_formula_function(const std::string& name) : formula_function(name) {}
	function_expression_ptr generate_function_expression(const std::vector<expression_ptr>& args) const {
		return function_expression_ptr(new T(args));
	}
};

typedef std::shared_ptr<formula_function> formula_function_ptr;
typedef std::map<std::string, formula_function_ptr> functions_map;

class function_symbol_table {
	std::shared_ptr<function_symbol_table> parent;
	functions_map custom_formulas_;
	enum builtins_tag_t {builtins_tag};
	function_symbol_table(builtins_tag_t) {}
public:
	explicit function_symbol_table(std::shared_ptr<function_symbol_table> parent = nullptr);
	void add_function(const std::string& name, formula_function_ptr fcn);
	expression_ptr create_function(const std::string& fn, const std::vector<expression_ptr>& args) const;
	std::set<std::string> get_function_names() const;
	bool empty() {return custom_formulas_.empty() && (parent == nullptr || parent->empty());}
	static std::shared_ptr<function_symbol_table> get_builtins();
};

class action_function_symbol_table : public function_symbol_table {
public:
	action_function_symbol_table();
};

class wrapper_formula : public formula_expression {
public:
	wrapper_formula()
		: arg_()
	{
	}

	wrapper_formula(expression_ptr arg)
		: formula_expression(arg ? arg->get_name() : ""), arg_(arg)
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
	virtual variant execute(const formula_callable& variables, formula_debugger *fdb = nullptr) const
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
