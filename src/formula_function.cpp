/* $Id$ */
/*
   Copyright (C) 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include <iostream>
#include <math.h>

//#include "foreach.hpp"
#include "formula_callable.hpp"
#include "formula_function.hpp"

#include "SDL.h"

namespace game_logic {

namespace {

class dir_function : public function_expression {
public:
	explicit dir_function(const args_list& args)
	     : function_expression("dir", args, 1, 1)
	{}

private:
	variant execute(const formula_callable& variables) const {
		variant var = args()[0]->evaluate(variables);
		const formula_callable* callable = var.as_callable();
		std::vector<formula_input> inputs = callable->inputs();
		std::vector<variant> res;
		for(int i=0; i<inputs.size(); ++i) {
			const formula_input& input = inputs[i];
			res.push_back(variant(input.name));
		}

		return variant(&res);
	}
};

class if_function : public function_expression {
public:
	explicit if_function(const args_list& args)
	     : function_expression("if", args, 3, 3)
	{}

private:
	variant execute(const formula_callable& variables) const {
		const int i = args()[0]->evaluate(variables).as_bool() ? 1 : 2;
		return args()[i]->evaluate(variables);
	}
};

class switch_function : public function_expression {
public:
	explicit switch_function(const args_list& args)
	    : function_expression("switch", args, 4, -1)
	{}

private:
	variant execute(const formula_callable& variables) const {
		variant var = args()[0]->evaluate(variables);
		for(int n = 0; n < args().size()-1; n += 2) {
			variant val = args()[n]->evaluate(variables);
			if(val == var) {
				return args()[n+1]->evaluate(variables);
			}
		}

		if((args().size()%2) == 0) {
			return args().back()->evaluate(variables);
		} else {
			return variant();
		}
	}
};

class rgb_function : public function_expression {
public:
	explicit rgb_function(const args_list& args)
	     : function_expression("rgb", args, 3, 3)
	{}

private:
	variant execute(const formula_callable& variables) const {
		return variant(10000*
		 std::min<int>(99,std::max<int>(0,args()[0]->evaluate(variables).as_int())) +
		 std::min<int>(99,std::max<int>(0,args()[1]->evaluate(variables).as_int()))*100+
		 std::min<int>(99,std::max<int>(0,args()[2]->evaluate(variables).as_int())));
	}
};

namespace {
int transition(int begin, int val1, int end, int val2, int value) {
	if(value < begin || value > end) {
		return 0;
	}

	if(value == begin) {
		return val1;
	} else if(value == end) {
		return val2;
	}

	const int comp1 = val1*(end - value);
	const int comp2 = val2*(value - begin);
	return (comp1 + comp2)/(end - begin);
}
}

class transition_function : public function_expression {
public:
	explicit transition_function(const args_list& args)
			: function_expression("transition", args, 5, 5)
	{}
private:
	variant execute(const formula_callable& variables) const {
		const int value = args()[0]->evaluate(variables).as_int();
		const int begin = args()[1]->evaluate(variables).as_int();
		const int end = args()[3]->evaluate(variables).as_int();
		if(value < begin || value > end) {
			return variant(0);
		}
		const int val1 = args()[2]->evaluate(variables).as_int();
		const int val2 = args()[4]->evaluate(variables).as_int();
		return variant(transition(begin, val1, end, val2, value));
	}
};

class color_transition_function : public function_expression {
public:
	explicit color_transition_function(const args_list& args)
			: function_expression("color_transition", args, 5)
	{}
private:
	variant execute(const formula_callable& variables) const {
		const int value = args()[0]->evaluate(variables).as_int();
		int begin = args()[1]->evaluate(variables).as_int();
		int end = -1;
		int n = 3;
		while(n < args().size()) {
			end = args()[n]->evaluate(variables).as_int();
			if(value >= begin && value <= end) {
				break;
			}

			begin = end;
			n += 2;
		}

		if(value < begin || value > end) {
			return variant(0);
		}
		const int val1 = args()[n-1]->evaluate(variables).as_int();
		const int val2 = args()[n+1 < args().size() ? n+1 : n]->
		                               evaluate(variables).as_int();
		const int r1 = (val1/10000)%100;
		const int g1 = (val1/100)%100;
		const int b1 = (val1)%100;
		const int r2 = (val2/10000)%100;
		const int g2 = (val2/100)%100;
		const int b2 = (val2)%100;

		const int r = transition(begin,r1,end,r2,value);
		const int g = transition(begin,g1,end,g2,value);
		const int b = transition(begin,b1,end,b2,value);
		return variant(
		       std::min<int>(99,std::max<int>(0,r))*100*100 +
		       std::min<int>(99,std::max<int>(0,g))*100+
		       std::min<int>(99,std::max<int>(0,b)));
	}
};


class abs_function : public function_expression {
public:
	explicit abs_function(const args_list& args)
	     : function_expression("abs", args, 1, 1)
	{}

private:
	variant execute(const formula_callable& variables) const {
		const int n = args()[0]->evaluate(variables).as_int();
		return variant(n >= 0 ? n : -n);
	}
};

class min_function : public function_expression {
public:
	explicit min_function(const args_list& args)
	     : function_expression("min", args, 1, -1)
	{}

private:
	variant execute(const formula_callable& variables) const {
		bool found = false;
		int res = 0;
		for(int n = 0; n != args().size(); ++n) {
			const variant v = args()[n]->evaluate(variables);
			if(v.is_list()) {
				for(int m = 0; m != v.num_elements(); ++m) {
					if(!found || v[m].as_int() < res) {
						res = v[m].as_int();
						found = true;
					}
				}
			} else if(v.is_int()) {
				if(!found || v.as_int() < res) {
					res = v.as_int();
					found = true;
				}
			}
		}

		return variant(res);
	}
};

class max_function : public function_expression {
public:
	explicit max_function(const args_list& args)
	     : function_expression("max", args, 1, -1)
	{}

private:
	variant execute(const formula_callable& variables) const {
		bool found = false;
		int res = 0;
		for(int n = 0; n != args().size(); ++n) {
			const variant v = args()[n]->evaluate(variables);
			if(v.is_list()) {
				for(int m = 0; m != v.num_elements(); ++m) {
					if(!found || v[m].as_int() > res) {
						res = v[m].as_int();
						found = true;
					}
				}
			} else if(v.is_int()) {
				if(!found || v.as_int() > res) {
					res = v.as_int();
					found = true;
				}
			}
		}

		return variant(res);
	}
};

class choose_element_function : public function_expression {
public:
	explicit choose_element_function(const args_list& args)
	     : function_expression("choose_element", args, 2, 2)
	{}

private:
	variant execute(const formula_callable& variables) const {
		const variant items = args()[0]->evaluate(variables);
		int max_index = -1;
		variant max_value;
		for(int n = 0; n != items.num_elements(); ++n) {
			const variant val = args()[1]->evaluate(formula_callable_with_backup(*items[n].as_callable(), variables));
			if(max_index == -1 || val > max_value) {
				max_index = n;
				max_value = val;
			}
		}

		if(max_index == -1) {
			return variant(0);
		} else {
			return items[max_index];
		}
	}
};

class wave_function : public function_expression {
public:
	explicit wave_function(const args_list& args)
	     : function_expression("wave", args, 1, 1)
	{}

private:
	variant execute(const formula_callable& variables) const {
		const int value = args()[0]->evaluate(variables).as_int()%1000;
		const double angle = 2.0*3.141592653589*(static_cast<double>(value)/1000.0);
		return variant(static_cast<int>(sin(angle)*1000.0));
	}
};

namespace {
class variant_comparator : public formula_callable {
	expression_ptr expr_;
	const formula_callable* fallback_;
	mutable variant a_, b_;
	variant get_value(const std::string& key) const {
		if(key == "a") {
			return a_;
		} else if(key == "b") {
			return b_;
		} else {
			return fallback_->query_value(key);
		}
	}

	void get_inputs(std::vector<formula_input>* inputs) const {
		fallback_->get_inputs(inputs);
	}
public:
	variant_comparator(const expression_ptr& expr, const formula_callable& fallback) : expr_(expr), fallback_(&fallback)
	{}

	bool operator()(const variant& a, const variant& b) const {
		a_ = a;
		b_ = b;
		return expr_->evaluate(*this).as_bool();
	}
};
}

class sort_function : public function_expression {
public:
	explicit sort_function(const args_list& args)
	     : function_expression("sort", args, 1, 2)
	{}

private:
	variant execute(const formula_callable& variables) const {
		variant list = args()[0]->evaluate(variables);
		std::vector<variant> vars;
		vars.reserve(list.num_elements());
		for(int n = 0; n != list.num_elements(); ++n) {
			vars.push_back(list[n]);
		}

		if(args().size() == 1) {
			std::sort(vars.begin(), vars.end());
		} else {
			std::sort(vars.begin(), vars.end(), variant_comparator(args()[1], variables));
		}

		return variant(&vars);
	}
};

class filter_function : public function_expression {
public:
	explicit filter_function(const args_list& args)
	    : function_expression("filter", args, 2, 3)
	{}
private:
	variant execute(const formula_callable& variables) const {
		std::vector<variant> vars;
		const variant items = args()[0]->evaluate(variables);
		if(args().size() == 2) {
			for(int n = 0; n != items.num_elements(); ++n) {
				const variant val = args()[1]->evaluate(formula_callable_with_backup(*items[n].as_callable(), variables));
				if(val.as_bool()) {
					vars.push_back(items[n]);
				}
			}
		} else {
			map_formula_callable self_callable;
			const std::string self = args()[1]->evaluate(variables).as_string();
			for(int n = 0; n != items.num_elements(); ++n) {
				self_callable.add(self, items[n]);
				const variant val = args()[2]->evaluate(formula_callable_with_backup(self_callable, formula_callable_with_backup(*items[n].as_callable(), variables)));
				if(val.as_bool()) {
					vars.push_back(items[n]);
				}
			}
		}

		return variant(&vars);
	}
};

class find_element_function : public function_expression {
public:
	explicit find_element_function(const args_list& args)
	    : function_expression("find_element", args, 2, 3)
	{}

private:
	variant execute(const formula_callable& variables) const {
		const variant items = args()[0]->evaluate(variables);

		if(args().size() == 2) {
			for(int n = 0; n != items.num_elements(); ++n) {
				const variant val = args()[1]->evaluate(formula_callable_with_backup(*items[n].as_callable(), variables));
				if(val.as_bool()) {
					return items[n];
				}
			}
		} else {
			map_formula_callable self_callable;
			const std::string self = args()[1]->evaluate(variables).as_string();
			for(int n = 0; n != items.num_elements(); ++n) {
				self_callable.add(self, items[n]);
				const variant val = args().back()->evaluate(formula_callable_with_backup(self_callable, formula_callable_with_backup(*items[n].as_callable(), variables)));
				if(val.as_bool()) {
					return items[n];
				}
			}
		}

		return variant();
	}
};

class map_function : public function_expression {
public:
	explicit map_function(const args_list& args)
	    : function_expression("map", args, 2, 3)
	{}
private:
	variant execute(const formula_callable& variables) const {
		std::vector<variant> vars;
		const variant items = args()[0]->evaluate(variables);

		if(args().size() == 2) {
			for(int n = 0; n != items.num_elements(); ++n) {
				const variant val = args().back()->evaluate(formula_callable_with_backup(*items[n].as_callable(), variables));
				vars.push_back(val);
			}
		} else {
			map_formula_callable self_callable;
			const std::string self = args()[1]->evaluate(variables).as_string();
			for(int n = 0; n != items.num_elements(); ++n) {
				self_callable.add(self, items[n]);
				const variant val = args().back()->evaluate(formula_callable_with_backup(self_callable, formula_callable_with_backup(*items[n].as_callable(), variables)));
				vars.push_back(val);
			}
		}

		return variant(&vars);
	}
};

class sum_function : public function_expression {
public:
	explicit sum_function(const args_list& args)
	    : function_expression("sum", args, 1, 2)
	{}
private:
	variant execute(const formula_callable& variables) const {
		variant res(0);
		const variant items = args()[0]->evaluate(variables);
		if(args().size() >= 2) {
			res = args()[1]->evaluate(variables);
		}
		for(int n = 0; n != items.num_elements(); ++n) {
			res = res + items[n];
		}

		return res;
	}
};

class head_function : public function_expression {
public:
	explicit head_function(const args_list& args)
	    : function_expression("head", args, 1, 1)
	{}
private:
	variant execute(const formula_callable& variables) const {
		const variant items = args()[0]->evaluate(variables);
		return items[0];
	}
};

class size_function : public function_expression {
public:
	explicit size_function(const args_list& args)
	    : function_expression("size", args, 1, 1)
	{}
private:
	variant execute(const formula_callable& variables) const {
		const variant items = args()[0]->evaluate(variables);
		return variant(static_cast<int>(items.num_elements()));
	}
};

class null_function : public function_expression {
public:
	explicit null_function(const args_list& args)
	    : function_expression("null", args, 0, 0)
	{}
private:
	variant execute(const formula_callable& variables) const {
		return variant();
	}
};

class refcount_function : public function_expression {
public:
	explicit refcount_function(const args_list& args)
	    : function_expression("refcount", args, 1, 1)
	{}
private:
	variant execute(const formula_callable& variables) const {
		return variant(args()[0]->evaluate(variables).refcount());
	}
};

}

formula_function_expression::formula_function_expression(const std::string& name, const args_list& args, const_formula_ptr formula, const_formula_ptr precondition, const std::vector<std::string>& arg_names)
  : function_expression(name, args, arg_names.size(), arg_names.size()),
    formula_(formula), precondition_(precondition), arg_names_(arg_names), star_arg_(-1)
{
	for(int n = 0; n != arg_names_.size(); ++n) {
		if(arg_names_.empty() == false && arg_names_[n][arg_names_[n].size()-1] == '*') {
			arg_names_[n].resize(arg_names_[n].size()-1);
			star_arg_ = n;
			break;
		}
	}
}

variant formula_function_expression::execute(const formula_callable& variables) const
{
	static std::string indent;
	indent += "  ";
	std::cerr << indent << "executing '" << formula_->str() << "'\n";
	const int begin_time = SDL_GetTicks();
	map_formula_callable callable;
	for(int n = 0; n != arg_names_.size(); ++n) {
		variant var = args()[n]->evaluate(variables);
		callable.add(arg_names_[n], var);
		if(n == star_arg_) {
			callable.set_fallback(var.as_callable());
		}
	}

	if(precondition_) {
		if(!precondition_->execute(callable).as_bool()) {
			std::cerr << "FAILED function precondition for function '" << formula_->str() << "' with arguments: ";
			for(int n = 0; n != arg_names_.size(); ++n) {
				std::cerr << "  arg " << (n+1) << ": " << args()[n]->evaluate(variables).to_debug_string() << "\n";
			}
		}
	}

	variant res = formula_->execute(callable);
	const int taken = SDL_GetTicks() - begin_time;
	std::cerr << indent << "returning: " << taken << "\n";
	indent.resize(indent.size() - 2);

	return res;
}

function_expression_ptr formula_function::generate_function_expression(const std::vector<expression_ptr>& args) const
{
	return function_expression_ptr(new formula_function_expression(name_, args, formula_, precondition_, args_));
}

void function_symbol_table::add_formula_function(const std::string& name, const_formula_ptr formula, const_formula_ptr precondition, const std::vector<std::string>& args)
{
	custom_formulas_[name] = formula_function(name, formula, precondition, args);
}

expression_ptr function_symbol_table::create_function(const std::string& fn, const std::vector<expression_ptr>& args) const
{
	const std::map<std::string, formula_function>::const_iterator i = custom_formulas_.find(fn);
	if(i != custom_formulas_.end()) {
		return i->second.generate_function_expression(args);
	}

	return expression_ptr();
}

std::vector<std::string> function_symbol_table::get_function_names() const
{
	std::vector<std::string> res;
	for(std::map<std::string, formula_function>::const_iterator iter = custom_formulas_.begin(); iter != custom_formulas_.end(); iter++ ) {
		res.push_back((*iter).first);
	}
	return res;
}

expression_ptr create_function(const std::string& fn,
                               const std::vector<expression_ptr>& args,
							   const function_symbol_table* symbols)
{
	if(symbols) {
		expression_ptr res(symbols->create_function(fn, args));
		if(res) {
			return res;
		}
	}

	std::cerr << "FN: '" << fn << "' " << fn.size() << "\n";
	if(fn == "dir") {
		return expression_ptr(new dir_function(args));
	} else if(fn == "if") {
		return expression_ptr(new if_function(args));
	} else if(fn == "switch") {
		return expression_ptr(new switch_function(args));
	} else if(fn == "abs") {
		return expression_ptr(new abs_function(args));
	} else if(fn == "min") {
		return expression_ptr(new min_function(args));
	} else if(fn == "max") {
		return expression_ptr(new max_function(args));
	} else if(fn == "choose") {
		return expression_ptr(new choose_element_function(args));
	} else if(fn == "wave") {
		return expression_ptr(new wave_function(args));
	} else if(fn == "sort") {
		return expression_ptr(new sort_function(args));
	} else if(fn == "filter") {
		return expression_ptr(new filter_function(args));
	} else if(fn == "find") {
		return expression_ptr(new find_element_function(args));
	} else if(fn == "map") {
		return expression_ptr(new map_function(args));
	} else if(fn == "sum") {
		return expression_ptr(new sum_function(args));
	} else if(fn == "head") {
		return expression_ptr(new head_function(args));
	} else if(fn == "rgb") {
		return expression_ptr(new rgb_function(args));
	} else if(fn == "transition") {
		return expression_ptr(new transition_function(args));
	} else if(fn == "color_transition") {
		return expression_ptr(new color_transition_function(args));
	} else if(fn == "size") {
		return expression_ptr(new size_function(args));
	} else if(fn == "null") {
		return expression_ptr(new null_function(args));
	} else if(fn == "refcount") {
		return expression_ptr(new refcount_function(args));
	} else {
		std::cerr << "no function '" << fn << "'\n";
		throw formula_error();
	}
}

}
