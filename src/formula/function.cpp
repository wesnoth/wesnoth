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

#include "formula/callable_objects.hpp"
#include "formula/debugger.hpp"
#include "formula/function.hpp"
#include "game_display.hpp"
#include "game_config.hpp"
#include "log.hpp"
#include "color.hpp"

#include <boost/math/constants/constants.hpp>
#include <cctype>
using namespace boost::math::constants;

#ifdef HAVE_VISUAL_LEAK_DETECTOR
#include "vld.h"
#endif


static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)
static lg::log_domain log_scripting_formula("scripting/formula");
#define LOG_SF LOG_STREAM(info, log_scripting_formula)
#define WRN_SF LOG_STREAM(warn, log_scripting_formula)
#define ERR_SF LOG_STREAM(err, log_scripting_formula)

namespace game_logic {

std::string function_expression::str() const
{
	std::stringstream s;
	s << get_name();
	s << '(';
	bool first_arg = true;
	for (expression_ptr a : args()) {
		if (!first_arg) {
			s << ',';
		} else {
			first_arg = false;
		}
		s << a->str();
		}
	s << ')';
	return s.str();
}

namespace {

class debug_function : public function_expression {
public:
	explicit debug_function(const args_list& args)
		: function_expression("debug",args, 0, 1)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		std::shared_ptr<formula_debugger> fdbp;
		bool need_wrapper = false;
		if (fdb==nullptr) {
			fdbp = std::shared_ptr<formula_debugger>(new formula_debugger());
			fdb = &*fdbp;
			need_wrapper = true;

		}

		if (args().size()==1) {
			if (!need_wrapper) {
				return args()[0]->evaluate(variables,fdb);
			} else {
				return wrapper_formula(args()[0]).evaluate(variables,fdb);
			}
		} else {
			return wrapper_formula().evaluate(variables,fdb);
		}
	}
};


class dir_function : public function_expression {
public:
	explicit dir_function(const args_list& args)
	     : function_expression("dir", args, 1, 1)
	{}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		variant var = args()[0]->evaluate(variables, fdb);
		const formula_callable* callable = var.as_callable().get();
		std::vector<formula_input> inputs = callable->inputs();
		std::vector<variant> res;
		for(size_t i=0; i<inputs.size(); ++i) {
			const formula_input& input = inputs[i];
			res.push_back(variant(input.name));
		}

		return variant(res);
	}
};

class if_function : public function_expression {
public:
	explicit if_function(const args_list& args)
	     : function_expression("if", args, 2, -1)
	{}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		for(size_t n = 0; n < args().size()-1; n += 2) {
			if( args()[n]->evaluate(variables,fdb).as_bool() ) {
				return args()[n+1]->evaluate(variables,fdb);
			}
		}

		if((args().size()%2) != 0) {
			return args().back()->evaluate(variables,fdb);
		} else {
			return variant();
		}

	}
};

class switch_function : public function_expression {
public:
	explicit switch_function(const args_list& args)
	    : function_expression("switch", args, 3, -1)
	{}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		variant var = args()[0]->evaluate(variables,fdb);
		for(size_t n = 1; n < args().size()-1; n += 2) {
			variant val = args()[n]->evaluate(variables,fdb);
			if(val == var) {
				return args()[n+1]->evaluate(variables,fdb);
			}
		}

		if((args().size()%2) == 0) {
			return args().back()->evaluate(variables,fdb);
		} else {
			return variant();
		}
	}
};

class abs_function : public function_expression {
public:
	explicit abs_function(const args_list& args)
	     : function_expression("abs", args, 1, 1)
	{}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const variant input = args()[0]->evaluate(variables,fdb);
		if(input.is_decimal()) {
			const int n = input.as_decimal();
			return variant(n >= 0 ? n : -n, variant::DECIMAL_VARIANT);
		} else {
			const int n = input.as_int();
			return variant(n >= 0 ? n : -n);
		}
	}
};

class min_function : public function_expression {
public:
	explicit min_function(const args_list& args)
	     : function_expression("min", args, 1, -1)
	{}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		bool found = false;
		variant res(0);
		for(size_t n = 0; n != args().size(); ++n) {
			const variant v = args()[n]->evaluate(variables,fdb);
			if(v.is_list()) {
				for(size_t m = 0; m != v.num_elements(); ++m) {
					if(!found || v[m] < res) {
						res = v[m];
						found = true;
					}
				}
			} else if(v.is_int() || v.is_decimal()) {
				if(!found || v < res) {
					res = v;
					found = true;
				}
			}
		}

		return res;
	}
};

class max_function : public function_expression {
public:
	explicit max_function(const args_list& args)
	     : function_expression("max", args, 1, -1)
	{}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		bool found = false;
		variant res(0);
		for(size_t n = 0; n != args().size(); ++n) {
			const variant v = args()[n]->evaluate(variables,fdb);
			if(v.is_list()) {
				for(size_t m = 0; m != v.num_elements(); ++m) {
					if(!found || v[m] > res) {
						res = v[m];
						found = true;
					}
				}
			} else if(v.is_int() || v.is_decimal()) {
				if(!found || v > res) {
					res = v;
					found = true;
				}
			}
		}

		return res;
	}
};

class debug_float_function : public function_expression {
public:
        explicit debug_float_function(const args_list& args)
          : function_expression("debug_float", args, 2, 3)
        {}

private:
        variant execute(const formula_callable& variables, formula_debugger *fdb) const {
                const args_list& arguments = args();
                const variant var0 = arguments[0]->evaluate(variables,fdb);
                const variant var1 = arguments[1]->evaluate(variables,fdb);

                const map_location location = convert_variant<location_callable>(var0)->loc();
                std::string text;

                if(arguments.size() == 2) {
                        text = var1.to_debug_string();
                        display_float(location,text);
                        return var1;
                } else {
                        const variant var2 = arguments[2]->evaluate(variables,fdb);
                        text = var1.string_cast() + var2.to_debug_string();
                        display_float(location,text);
                        return var2;
                }

        }

        void display_float(const map_location& location, const std::string& text) const{
                game_display::get_singleton()->float_label(location, text, color_t(255,0,0));
        }
};


class debug_print_function : public function_expression {
public:
	explicit debug_print_function(const args_list& args)
	     : function_expression("debug_print", args, 1, 2)
	{}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const variant var1 = args()[0]->evaluate(variables,fdb);

		std::string str1,str2;

		if( args().size() == 1)
		{
			str1 = var1.to_debug_string(nullptr, true);
			LOG_SF << str1 << std::endl;
			if(game_config::debug) {
				game_display::get_singleton()->get_chat_manager().add_chat_message(time(nullptr), "WFL", 0, str1, events::chat_handler::MESSAGE_PUBLIC, false);
			}
			return var1;
		} else {
			str1 = var1.string_cast();
			const variant var2 = args()[1]->evaluate(variables,fdb);
			str2 = var2.to_debug_string(nullptr, true);
			LOG_SF << str1 << str2 << std::endl;
			if(game_config::debug) {
				game_display::get_singleton()->get_chat_manager().add_chat_message(time(nullptr), str1, 0, str2, events::chat_handler::MESSAGE_PUBLIC, false);
			}
			return var2;
		}
	}
};

class debug_profile_function : public function_expression {
public:
	explicit debug_profile_function(const args_list& args)
	     : function_expression("debug_profile", args, 1, 2)
	{}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		std::string speaker = "WFL";
		int i_value = 0;
		if(args().size() == 2) {
			speaker = args()[0]->evaluate(variables, fdb).string_cast();
			i_value = 1;
		}

		const variant value = args()[i_value]->evaluate(variables,fdb);
		long run_time = 0;
		for(int i = 1; i < 1000; i++) {
			const long start = SDL_GetTicks();
			args()[i_value]->evaluate(variables,fdb);
			run_time += SDL_GetTicks() - start;
		}

		std::ostringstream str;
		str << "Evaluated in " << (run_time / 1000.0) << " ms on average";
		LOG_SF << speaker << ": " << str.str() << std::endl;
		if(game_config::debug) {
			game_display::get_singleton()->get_chat_manager().add_chat_message(time(nullptr), speaker, 0, str.str(), events::chat_handler::MESSAGE_PUBLIC, false);
		}
		return value;
	}
};

class keys_function : public function_expression {
public:
	explicit keys_function(const args_list& args)
	     : function_expression("keys", args, 1, 1)
	{}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const variant map = args()[0]->evaluate(variables,fdb);
		return map.get_keys();
	}
};

class values_function : public function_expression {
public:
	explicit values_function(const args_list& args)
	     : function_expression("values", args, 1, 1)
	{}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const variant map = args()[0]->evaluate(variables,fdb);
		return map.get_values();
	}
};

class tolist_function : public function_expression {
public:
	explicit tolist_function(const args_list& args)
	     : function_expression("tolist", args, 1, 1)
	{}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const variant var = args()[0]->evaluate(variables,fdb);

		std::vector<variant> tmp;

		for(variant_iterator it = var.begin(); it != var.end(); ++it) {
			tmp.push_back( *it );
		}

		return variant(tmp);
	}
};

class tomap_function : public function_expression {
public:
	explicit tomap_function(const args_list& args)
	     : function_expression("tomap", args, 1, 2)
	{}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const variant var_1 = args()[0]->evaluate(variables,fdb);

		std::map<variant, variant> tmp;

		if (args().size() == 2)
		{
			const variant var_2 = args()[1]->evaluate(variables,fdb);
			if ( var_1.num_elements() != var_2.num_elements() )
				return variant();
			for(size_t i = 0; i < var_1.num_elements(); ++i )
				tmp[ var_1[i] ] = var_2[i];
		} else
		{
			for(variant_iterator it = var_1.begin(); it != var_1.end(); ++it) {
				if (key_value_pair* kv = (*it).try_convert<key_value_pair>())
					tmp[kv->query_value("key")] = kv->query_value("value");
				else {
					std::map<variant, variant>::iterator map_it = tmp.find( *it );
					if (map_it == tmp.end()) {
						tmp[*it] = variant(1);
					} else {
						map_it->second = variant(map_it->second.as_int() + 1);
					}
				}
			}
		}

		return variant(tmp);
	}
};

class substring_function : public function_expression {
public:
	explicit substring_function(const args_list& args)
	     : function_expression("substring", args, 2, 3)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {

		std::string result = args()[0]->evaluate(variables, fdb).as_string();

		int offset = args()[1]->evaluate(variables, fdb).as_int();
		if(offset < 0) {
			offset += result.size();
			if(offset < 0) {
				offset = 0;
			}
		} else {
			if(static_cast<size_t>(offset) >= result.size()) {
				return variant(std::string());
			}
		}

		if(args().size() > 2) {
			int size = args()[2]->evaluate(variables, fdb).as_int();
			if(size < 0) {
				size = -size;
				offset = std::max(0, offset - size + 1);
			}
			return variant(result.substr(offset, size));
		} else {
			return variant(result.substr(offset));
		}
	}
};

class replace_function : public function_expression {
public:
	explicit replace_function(const args_list& args)
	     : function_expression("replace", args, 3, 4)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {

		std::string result = args()[0]->evaluate(variables, fdb).as_string();
		std::string replacement = args().back()->evaluate(variables, fdb).as_string();

		int offset = args()[1]->evaluate(variables, fdb).as_int();
		if(offset < 0) {
			offset += result.size();
			if(offset < 0) {
				offset = 0;
			}
		} else {
			if(static_cast<size_t>(offset) >= result.size()) {
				return variant(result);
			}
		}

		if(args().size() > 3) {
			int size = args()[2]->evaluate(variables, fdb).as_int();
			if(size < 0) {
				size = -size;
				offset = std::max(0, offset - size + 1);
			}
			return variant(result.replace(offset, size, replacement));
		} else {
			return variant(result.replace(offset, std::string::npos, replacement));
		}
	}
};

class length_function : public function_expression {
public:
	explicit length_function(const args_list& args)
	     : function_expression("length", args, 1, 1)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		return variant(args()[0]->evaluate(variables, fdb).as_string().length());
	}
};

class concatenate_function : public function_expression {
public:
		explicit concatenate_function(const args_list& args)
			: function_expression("concatenate", args, 1, -1)
		{}
private:
		variant execute(const formula_callable& variables, formula_debugger *fdb) const {
			std::string result;

			for(expression_ptr arg : args()) {
					result += arg->evaluate(variables, fdb).string_cast();
			}

			return variant(result);
		}
};

class str_upper_function : public function_expression {
public:
	explicit str_upper_function(const args_list& args)
		: function_expression("str_upper", args, 1, 1)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger* fdb) const {
		std::string str = args()[0]->evaluate(variables, fdb).as_string();
		std::transform(str.begin(), str.end(), str.begin(), static_cast<int(*)(int)>(std::toupper));
		return variant(str);
	}
};

class str_lower_function : public function_expression {
public:
	explicit str_lower_function(const args_list& args)
		: function_expression("str_lower", args, 1, 1)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger* fdb) const {
		std::string str = args()[0]->evaluate(variables, fdb).as_string();
		std::transform(str.begin(), str.end(), str.begin(), static_cast<int(*)(int)>(std::tolower));
		return variant(str);
	}
};

class sin_function : public function_expression {
public:
	explicit sin_function(const args_list& args)
	     : function_expression("sin", args, 1, 1)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const double angle = args()[0]->evaluate(variables,fdb).as_decimal() / 1000.0;
		const double result = sin(angle * pi<double>() / 180.0);
		return variant(result, variant::DECIMAL_VARIANT);
	}
};

class cos_function : public function_expression {
public:
	explicit cos_function(const args_list& args)
	     : function_expression("cos", args, 1, 1)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const double angle = args()[0]->evaluate(variables,fdb).as_decimal() / 1000.0;
		const double result = cos(angle * pi<double>() / 180.0);
		return variant(result, variant::DECIMAL_VARIANT);
	}
};

class tan_function : public function_expression {
public:
	explicit tan_function(const args_list& args)
	     : function_expression("tan", args, 1, 1)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const double angle = args()[0]->evaluate(variables,fdb).as_decimal() / 1000.0;
		const double result = tan(angle * pi<double>() / 180.0);
		if(result != result || result <= INT_MIN || result >= INT_MAX) {
			return variant();
		}
		return variant(result, variant::DECIMAL_VARIANT);
	}
};

class asin_function : public function_expression {
public:
	explicit asin_function(const args_list& args)
	     : function_expression("asin", args, 1, 1)
	{}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const double num = args()[0]->evaluate(variables,fdb).as_decimal() / 1000.0;
		const double result = asin(num) * 180.0 / pi<double>();
		if(result != result) {
			return variant();
		}
		return variant(result, variant::DECIMAL_VARIANT);
	}
};

class acos_function : public function_expression {
public:
	explicit acos_function(const args_list& args)
	     : function_expression("acos", args, 1, 1)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const double num = args()[0]->evaluate(variables,fdb).as_decimal() / 1000.0;
		const double result = acos(num) * 180.0 / pi<double>();
		if(result != result) {
			return variant();
		}
		return variant(result, variant::DECIMAL_VARIANT);
	}
};

class atan_function : public function_expression {
public:
	explicit atan_function(const args_list& args)
	     : function_expression("acos", args, 1, 1)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const double num = args()[0]->evaluate(variables,fdb).as_decimal() / 1000.0;
		const double result = atan(num) * 180.0 / pi<double>();
		return variant(result, variant::DECIMAL_VARIANT);
	}
};

class sqrt_function : public function_expression {
public:
	explicit sqrt_function(const args_list& args)
	     : function_expression("sqrt", args, 1, 1)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const double num = args()[0]->evaluate(variables,fdb).as_decimal() / 1000.0;
		const double result = sqrt(num);
		if(result != result) {
			return variant();
		}
		return variant(result, variant::DECIMAL_VARIANT);
	}
};

class cbrt_function : public function_expression {
public:
	explicit cbrt_function(const args_list& args)
	     : function_expression("cbrt", args, 1, 1)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const double num = args()[0]->evaluate(variables,fdb).as_decimal() / 1000.0;
		const double result = num < 0 ? -pow(-num, 1.0 / 3.0) : pow(num, 1.0 / 3.0);
		return variant(result, variant::DECIMAL_VARIANT);
	}
};

class root_function : public function_expression {
public:
	explicit root_function(const args_list& args)
	     : function_expression("root", args, 2, 2)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const double base = args()[0]->evaluate(variables,fdb).as_decimal() / 1000.0;
		const double root = args()[1]->evaluate(variables,fdb).as_decimal() / 1000.0;
		const double result = base < 0 && fmod(root,2) == 1 ? -pow(-base, 1.0 / root) : pow(base, 1.0 / root);
		if(result != result) {
			return variant();
		}
		return variant(result, variant::DECIMAL_VARIANT);
	}
};

class log_function : public function_expression {
public:
	explicit log_function(const args_list& args)
	: function_expression("log", args, 1, 2)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const double num = args()[0]->evaluate(variables,fdb).as_decimal() / 1000.0;
		if(args().size() == 1) {
			const double result = log(num);
			if(result != result) {
				return variant();
			}
			return variant(result, variant::DECIMAL_VARIANT);
		} else {
			const double base = args()[1]->evaluate(variables,fdb).as_decimal() / 1000.0;
			const double result = log(num) / log(base);
			if(result != result) {
				return variant();
			}
			return variant(result, variant::DECIMAL_VARIANT);
		}
	}
};

class exp_function : public function_expression {
public:
	explicit exp_function(const args_list& args)
	     : function_expression("exp", args, 1, 1)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const double num = args()[0]->evaluate(variables,fdb).as_decimal() / 1000.0;
		const double result = exp(num);
		if(result == 0 || result >= INT_MAX) {
			// These are range errors rather than NaNs,
			// but I figure it's better than returning INT_MIN.
			return variant();
		}
		return variant(result, variant::DECIMAL_VARIANT);
	}
};

class pi_function : public function_expression {
public:
	explicit pi_function(const args_list& args)
	     : function_expression("pi", args, 0, 0)
	{}
private:
	variant execute(const formula_callable&, formula_debugger*) const {
		return variant(pi<double>(), variant::DECIMAL_VARIANT);
	}
};

class hypot_function : public function_expression {
public:
	explicit hypot_function(const args_list& args)
	     : function_expression("hypot", args, 2, 2)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const double x = args()[0]->evaluate(variables,fdb).as_decimal() / 1000.0;
		const double y = args()[1]->evaluate(variables,fdb).as_decimal() / 1000.0;
		return variant(hypot(x,y), variant::DECIMAL_VARIANT);
	}
};

class index_of_function : public function_expression {
public:
	explicit index_of_function(const args_list& args)
	     : function_expression("index_of", args, 2, 2)
	{}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const variant value = args()[0]->evaluate(variables,fdb);
		const variant list = args()[1]->evaluate(variables,fdb);

		for(size_t i = 0; i < list.num_elements(); ++i ) {
			if( list[i] == value) {
				return variant(i);
			}
		}

		return variant( -1 );
	}
};


class choose_function : public function_expression {
public:
	explicit choose_function(const args_list& args)
	     : function_expression("choose", args, 2, 3)
	{}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const variant items = args()[0]->evaluate(variables,fdb);
		variant max_value;
		variant_iterator max;

		if(args().size() == 2) {
			for(variant_iterator it = items.begin(); it != items.end(); ++it) {
				const variant val = args().back()->evaluate(formula_variant_callable_with_backup(*it, variables),fdb);
				if(max == variant_iterator() || val > max_value) {
					max = it;
					max_value = val;
				}
			}
		} else {
			map_formula_callable self_callable;
			const std::string self = args()[1]->evaluate(variables,fdb).as_string();
			for(variant_iterator it = items.begin(); it != items.end(); ++it) {
				self_callable.add(self, *it);
				const variant val = args().back()->evaluate(formula_callable_with_backup(self_callable, formula_variant_callable_with_backup(*it, variables)),fdb);
				if(max == variant_iterator() || val > max_value) {
					max = it;
					max_value = val;
				}
			}
		}

		if(max == variant_iterator() ) {
			return variant();
		} else {
			return *max;
		}
	}
};

class wave_function : public function_expression {
public:
	explicit wave_function(const args_list& args)
	     : function_expression("wave", args, 1, 1)
	{}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const int value = args()[0]->evaluate(variables,fdb).as_int()%1000;
		const double angle = 2.0 * pi<double>() * (static_cast<double>(value) / 1000.0);
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
	variant_comparator(const expression_ptr& expr, const formula_callable& fallback) :
		expr_(expr),
		fallback_(&fallback),
		a_(),
		b_()
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
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		variant list = args()[0]->evaluate(variables,fdb);
		std::vector<variant> vars;
		vars.reserve(list.num_elements());
		for(size_t n = 0; n != list.num_elements(); ++n) {
			vars.push_back(list[n]);
		}

		if(args().size() == 1) {
			std::sort(vars.begin(), vars.end());
		} else {
			std::sort(vars.begin(), vars.end(), variant_comparator(args()[1], variables));
		}

		return variant(vars);
	}
};

class reverse_function : public function_expression {
public:
	explicit reverse_function(const args_list& args)
	     : function_expression("reverse", args, 1, 1)
	{}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const variant& arg = args()[0]->evaluate(variables,fdb);
		if(arg.is_string()) {
			std::string str = args()[0]->evaluate(variables,fdb).as_string();
			std::reverse(str.begin(), str.end());
			return variant(str);
		} else if(arg.is_list()) {
			std::vector<variant> list = args()[0]->evaluate(variables,fdb).as_list();
			std::reverse(list.begin(), list.end());
			return variant(list);
		}
		return variant();
	}
};

class contains_string_function : public function_expression {
public:
	explicit contains_string_function(const args_list& args)
	     : function_expression("contains_string", args, 2, 2)
	{}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		std::string str = args()[0]->evaluate(variables,fdb).as_string();
		std::string key = args()[1]->evaluate(variables,fdb).as_string();

		return variant(str.find(key) != std::string::npos);
	}
};

class find_string_function : public function_expression {
public:
	explicit find_string_function(const args_list& args)
	     : function_expression("find_string", args, 2, 2)
	{}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const std::string str = args()[0]->evaluate(variables,fdb).as_string();
		const std::string key = args()[1]->evaluate(variables,fdb).as_string();

		size_t pos = str.find(key);
		return variant(static_cast<int>(pos));
	}
};

class filter_function : public function_expression {
public:
	explicit filter_function(const args_list& args)
	    : function_expression("filter", args, 2, 3)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		std::vector<variant> list_vars;
		std::map<variant,variant> map_vars;

		const variant items = args()[0]->evaluate(variables,fdb);

		if(args().size() == 2) {
			for(variant_iterator it = items.begin(); it != items.end(); ++it)  {
				const variant val = args()[1]->evaluate(formula_variant_callable_with_backup(*it, variables),fdb);
				if(val.as_bool()) {
					if (items.is_map() )
						map_vars[ (*it).get_member("key") ] = (*it).get_member("value");
					else
						list_vars.push_back(*it);
				}
			}
		} else {
			map_formula_callable self_callable;
			const std::string self = args()[1]->evaluate(variables,fdb).as_string();
			for(variant_iterator it = items.begin(); it != items.end(); ++it) {
				self_callable.add(self, *it);
				const variant val = args()[2]->evaluate(formula_callable_with_backup(self_callable, formula_variant_callable_with_backup(*it, variables)),fdb);
				if(val.as_bool()) {
					if (items.is_map() )
						map_vars[ (*it).get_member("key") ] = (*it).get_member("value");
					else
						list_vars.push_back(*it);
				}
			}
		}
		if (items.is_map() )
			return variant(map_vars);
		return variant(list_vars);
	}
};

class find_function : public function_expression {
public:
	explicit find_function(const args_list& args)
	    : function_expression("find", args, 2, 3)
	{}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const variant items = args()[0]->evaluate(variables,fdb);

		if(args().size() == 2) {
			for(variant_iterator it = items.begin(); it != items.end(); ++it) {
				const variant val = args()[1]->evaluate(formula_variant_callable_with_backup(*it, variables),fdb);
				if(val.as_bool()) {
					return *it;
				}
			}
		} else {
			map_formula_callable self_callable;
			const std::string self = args()[1]->evaluate(variables,fdb).as_string();
			for(variant_iterator it = items.begin(); it != items.end(); ++it){
				self_callable.add(self, *it);
				const variant val = args().back()->evaluate(formula_callable_with_backup(self_callable, formula_variant_callable_with_backup(*it, variables)),fdb);
				if(val.as_bool()) {
					return *it;
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
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		std::vector<variant> list_vars;
		std::map<variant,variant> map_vars;
		const variant items = args()[0]->evaluate(variables,fdb);

		if(args().size() == 2) {
			for(variant_iterator it = items.begin(); it != items.end(); ++it) {
				const variant val = args().back()->evaluate(formula_variant_callable_with_backup(*it, variables),fdb);
				if (items.is_map() )
					map_vars[ (*it).get_member("key") ] = val;
				else
					list_vars.push_back(val);
			}
		} else {
			map_formula_callable self_callable;
			const std::string self = args()[1]->evaluate(variables,fdb).as_string();
			for(variant_iterator it = items.begin(); it != items.end(); ++it) {
				self_callable.add(self, *it);
				const variant val = args().back()->evaluate(formula_callable_with_backup(self_callable, formula_variant_callable_with_backup(*it, variables)),fdb);
				if (items.is_map() )
					map_vars[ (*it).get_member("key") ] = val;
				else
					list_vars.push_back(val);
			}
		}
		if (items.is_map() )
			return variant(map_vars);
		return variant(list_vars);
	}
};

class take_while_function : public function_expression {
public:
	explicit take_while_function(const args_list& args)
		: function_expression("take_while", args, 2, 2)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const variant& items = args()[0]->evaluate(variables, fdb);
		variant_iterator it = items.begin();
		for(; it != items.end(); ++it) {
			const variant matches = args().back()->evaluate(formula_variant_callable_with_backup(*it, variables),fdb);
			if (!matches.as_bool())
				break;
		}
		std::vector<variant> result(items.begin(), it);
		return variant(result);
	}
};

class zip_function : public function_expression {
public:
	explicit zip_function(const args_list& args)
	    : function_expression("zip", args, 1, -1)
	{}
private:
	struct indexer {
		size_t i;
		explicit indexer(size_t i) : i(i) {}
		variant operator()(const variant& v) {
			if(i >= v.num_elements()) {
				return variant();
			} else {
				return v[i];
			}
		}
	};
	struct comparator {
		bool operator()(const variant& a, const variant& b) {
			return a.num_elements() < b.num_elements();
		}
	};
	std::vector<variant> get_input(const formula_callable& variables, formula_debugger* fdb) const {
		if(args().size() == 1) {
			const variant list = args()[0]->evaluate(variables, fdb);
			return std::vector<variant>(list.begin(), list.end());
		} else {
			std::vector<variant> input;
			input.reserve(args().size());
			for(expression_ptr expr : args()) {
				input.push_back(expr->evaluate(variables, fdb));
			}
			return input;
		}
	}
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const std::vector<variant> input = get_input(variables, fdb);
		std::vector<variant> output;
		// So basically this does [[a,b,c],[d,e,f],[x,y,z]] -> [[a,d,x],[b,e,y],[c,f,z]]
		// Or [[a,b,c,d],[x,y,z]] -> [[a,x],[b,y],[c,z],[d,null()]]
		size_t max_i = std::max_element(input.begin(), input.end(), comparator())->num_elements();
		output.reserve(max_i);
		for(size_t i = 0; i < max_i; i++) {
			std::vector<variant> elem(input.size());
			std::transform(input.begin(), input.end(), elem.begin(), indexer(i));
			output.push_back(variant(elem));
		}
		return variant(output);
	}
};

class reduce_function : public function_expression {
public:
	explicit reduce_function(const args_list& args)
	    : function_expression("reduce", args, 2, 3)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const variant items = args()[0]->evaluate(variables,fdb);
		const variant initial = args().size() == 2 ? variant() : args()[1]->evaluate(variables,fdb);

		if(items.num_elements() == 0)
			return initial;

		variant_iterator it = items.begin();
		variant res(initial.is_null() ? *it : initial);
		if(res != initial) {
			++it;
		}
		map_formula_callable self_callable;
		for(; it != items.end(); ++it) {
			self_callable.add("a", res);
			self_callable.add("b", *it);
			res = args().back()->evaluate(formula_callable_with_backup(self_callable, formula_variant_callable_with_backup(*it, variables)),fdb);
		}
		return res;
	}
};

class sum_function : public function_expression {
public:
	explicit sum_function(const args_list& args)
	    : function_expression("sum", args, 1, 2)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		variant res(0);
		const variant items = args()[0]->evaluate(variables,fdb);
		if (items.num_elements() > 0)
		{
			if (items[0].is_list() )
			{
				std::vector<variant> tmp;
				res = variant(tmp);
				if(args().size() >= 2) {
					res = args()[1]->evaluate(variables,fdb);
					if(!res.is_list())
						return variant();
				}
			} else if( items[0].is_map() )
			{
				std::map<variant,variant> tmp;
				res = variant(tmp);
				if(args().size() >= 2) {
					res = args()[1]->evaluate(variables,fdb);
					if(!res.is_map())
						return variant();
				}
			} else
			{
				if(args().size() >= 2) {
					res = args()[1]->evaluate(variables,fdb);
				}
			}
		}

		for(size_t n = 0; n != items.num_elements(); ++n) {
			res = res + items[n];
		}

		return res;
	}
};

class head_function : public function_expression {
public:
	explicit head_function(const args_list& args)
	    : function_expression("head", args, 1, 2)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const variant items = args()[0]->evaluate(variables,fdb);
		variant_iterator it = items.begin();
		if(it == items.end()) {
			return variant();
		}
		if(args().size() == 1) {
			return *it;
		}
		const int n = items.num_elements(), req = args()[1]->evaluate(variables,fdb).as_int();
		const int count = req < 0 ? n - std::min(-req, n) : std::min(req, n);
		variant_iterator end = it;
		std::advance(end, count);
		std::vector<variant> res;
		std::copy(it, end, std::back_inserter(res));
		return variant(res);
	}
};

class tail_function : public function_expression {
public:
	explicit tail_function(const args_list& args)
	    : function_expression("tail", args, 1, 2)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const variant items = args()[0]->evaluate(variables,fdb);
		variant_iterator it = items.end();
		if(it == items.begin()) {
			return variant();
		}
		if(args().size() == 1) {
			return *--it;
		}
		const int n = items.num_elements(), req = args()[1]->evaluate(variables,fdb).as_int();
		const int count = req < 0 ? n - std::min(-req, n) : std::min(req, n);
		std::advance(it, -count);
		std::vector<variant> res;
		std::copy(it, items.end(), std::back_inserter(res));
		return variant(res);
	}
};

class size_function : public function_expression {
public:
	explicit size_function(const args_list& args)
	    : function_expression("size", args, 1, 1)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const variant items = args()[0]->evaluate(variables,fdb);
		return variant(static_cast<int>(items.num_elements()));
	}
};

class null_function : public function_expression {
public:
	explicit null_function(const args_list& args)
	    : function_expression("null", args, 0, -1)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		if( args().size() != 0 ) {
			for( size_t i = 0; i < args().size() ; ++i)
				args()[i]->evaluate(variables,fdb);
		}

		return variant();
	}
};


class ceil_function : public function_expression {
public:
	explicit ceil_function(const args_list& args)
	    : function_expression("ceil", args, 1, 1)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		variant decimal = args()[0]->evaluate(variables,fdb);

		int d = decimal.as_decimal();

		if( (d>=0) && (d%1000 != 0) ) {
			d/=1000;
			return variant( ++d );
		} else {
			d/=1000;
			return variant( d );
		}
	}
};

class round_function : public function_expression {
public:
	explicit round_function(const args_list& args)
	    : function_expression("round", args, 1, 1)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		variant decimal = args()[0]->evaluate(variables,fdb);

		int d = decimal.as_decimal();

		int f = d%1000;

		if( f >= 500 ) {
			d/=1000;
			return variant( ++d );
		} else if( f <= -500 ) {
			d/=1000;
			return variant( --d );
		} else {
			d/=1000;
			return variant( d );
		}
	}
};

class floor_function : public function_expression {
public:
	explicit floor_function(const args_list& args)
	    : function_expression("floor", args, 1, 1)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		variant decimal = args()[0]->evaluate(variables,fdb);

		int d = decimal.as_decimal();

		if( (d<0) && (d%1000 != 0) ) {
			d/=1000;
			return variant( --d );
		} else {
			d/=1000;
			return variant( d );
		}
	}
};

class trunc_function : public function_expression {
public:
	explicit trunc_function(const args_list& args)
	    : function_expression("trunc", args, 1, 1)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		variant decimal = args()[0]->evaluate(variables,fdb);

		int d = decimal.as_int();

		return variant( d );
	}
};

class frac_function : public function_expression {
public:
	explicit frac_function(const args_list& args)
	    : function_expression("frac", args, 1, 1)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		variant decimal = args()[0]->evaluate(variables,fdb);

		int d = decimal.as_decimal();

		d%=1000;
		return variant( d, variant::DECIMAL_VARIANT );
	}
};

class sgn_function : public function_expression {
public:
	explicit sgn_function(const args_list& args)
	    : function_expression("sgn", args, 1, 1)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		variant decimal = args()[0]->evaluate(variables,fdb);

		int d = decimal.as_decimal();

		if( d != 0 ) {
			d = d>0 ? 1 : -1;
		}

		return variant( d );
	}
};

class as_decimal_function : public function_expression {
public:
	explicit as_decimal_function(const args_list& args)
	    : function_expression("as_decimal", args, 1, 1)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		variant decimal = args()[0]->evaluate(variables,fdb);

		int d = decimal.as_decimal();

		return variant( d , variant::DECIMAL_VARIANT );
	}
};

class loc_function : public function_expression {
public:
	explicit loc_function(const args_list& args)
	  : function_expression("loc", args, 2, 2)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		return variant(new location_callable(map_location(
							     args()[0]->evaluate(variables,add_debug_info(fdb,0,"loc:x")).as_int(),
							     args()[1]->evaluate(variables,add_debug_info(fdb,1,"loc:y")).as_int(), wml_loc())));
	}
};

class pair_function : public function_expression {
public:
	explicit pair_function(const args_list& args)
	  : function_expression("pair", args, 2, 2)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		return variant(new key_value_pair(
			args()[0]->evaluate(variables,add_debug_info(fdb,0,"pair:key")),
			args()[1]->evaluate(variables,add_debug_info(fdb,1,"pair_value"))
		));
	}
};

class distance_between_function : public function_expression {
public:
	explicit distance_between_function(const args_list& args)
	: function_expression("distance_between", args, 2, 2)
	{}

private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const map_location loc1 = convert_variant<location_callable>(args()[0]->evaluate(variables,add_debug_info(fdb,0,"distance_between:location_A")))->loc();
		const map_location loc2 = convert_variant<location_callable>(args()[1]->evaluate(variables,add_debug_info(fdb,1,"distance_between:location_B")))->loc();
		return variant(distance_between(loc1, loc2));
	}
};


class type_function : public function_expression {
public:
	explicit type_function(const args_list& args)
	    : function_expression("type", args, 1, 1)
	{}
private:
	variant execute(const formula_callable& variables, formula_debugger *fdb) const {
		const variant& v = args()[0]->evaluate(variables, fdb);
		return variant(v.type_string());
	}
};

}

variant key_value_pair::get_value(const std::string& key) const
{
	if(key == "key")
	{
		return key_;
	} else
		if(key == "value")
		{
			return value_;
		} else
			return variant();
}

void key_value_pair::get_inputs(formula_input_vector* inputs) const {
		add_input(inputs, "key");
		add_input(inputs, "value");
}


void key_value_pair::serialize_to_string(std::string& str) const {
	str += "pair(";
	str += key_.serialize_to_string();
	str += ",";
	str += value_.serialize_to_string();
	str += ")";
}

formula_function_expression::formula_function_expression(const std::string& name, const args_list& args, const_formula_ptr formula, const_formula_ptr precondition, const std::vector<std::string>& arg_names)
  : function_expression(name, args, arg_names.size(), arg_names.size()),
    formula_(formula), precondition_(precondition), arg_names_(arg_names), star_arg_(-1)
{
	for(size_t n = 0; n != arg_names_.size(); ++n) {
		if(arg_names_.empty() == false && arg_names_[n][arg_names_[n].size()-1] == '*') {
			arg_names_[n].resize(arg_names_[n].size()-1);
			star_arg_ = n;
			break;
		}
	}
}

variant formula_function_expression::execute(const formula_callable& variables, formula_debugger *fdb) const
{
	static std::string indent;
	indent += "  ";
	DBG_NG << indent << "executing '" << formula_->str() << "'\n";
	const int begin_time = SDL_GetTicks();
	map_formula_callable callable;
	for(size_t n = 0; n != arg_names_.size(); ++n) {
		variant var = args()[n]->evaluate(variables,fdb);
		callable.add(arg_names_[n], var);
		if(static_cast<int>(n) == star_arg_) {
			callable.set_fallback(var.as_callable().get());
		}
	}

	if(precondition_) {
		if(!precondition_->evaluate(callable,fdb).as_bool()) {
			DBG_NG << "FAILED function precondition for function '" << formula_->str() << "' with arguments: ";
			for(size_t n = 0; n != arg_names_.size(); ++n) {
				DBG_NG << "  arg " << (n+1) << ": " << args()[n]->evaluate(variables,fdb).to_debug_string() << "\n";
			}
		}
	}

	variant res = formula_->evaluate(callable,fdb);
	const int taken = SDL_GetTicks() - begin_time;
	DBG_NG << indent << "returning: " << taken << "\n";
	indent.resize(indent.size() - 2);

	return res;
}

function_expression_ptr user_formula_function::generate_function_expression(const std::vector<expression_ptr>& args) const
{
	return function_expression_ptr(new formula_function_expression(name_, args, formula_, precondition_, args_));
}

void function_symbol_table::add_function(const std::string& name, formula_function_ptr fcn)
{
	custom_formulas_[name] = fcn;
}

expression_ptr function_symbol_table::create_function(const std::string& fn, const std::vector<expression_ptr>& args) const
{
	const functions_map::const_iterator i = custom_formulas_.find(fn);
	if(i != custom_formulas_.end()) {
		return i->second->generate_function_expression(args);
	}

	return expression_ptr();
}

std::vector<std::string> function_symbol_table::get_function_names() const
{
	std::vector<std::string> res;
	for(functions_map::const_iterator iter = custom_formulas_.begin(); iter != custom_formulas_.end(); ++iter ) {
		res.push_back((*iter).first);
	}
	return res;
}

namespace {

function_symbol_table& get_functions_map() {
	static function_symbol_table functions_table;

	if(functions_table.empty()) {
#define FUNCTION(name) functions_table.add_function(#name, \
			formula_function_ptr(new builtin_formula_function<name##_function>(#name)))
		FUNCTION(debug);
		FUNCTION(dir);
		FUNCTION(if);
		FUNCTION(switch);
		FUNCTION(abs);
		FUNCTION(min);
		FUNCTION(max);
		FUNCTION(choose);
		FUNCTION(debug_float);
		FUNCTION(debug_print);
		FUNCTION(debug_profile);
		FUNCTION(wave);
		FUNCTION(sort);
		FUNCTION(contains_string);
		FUNCTION(find_string);
		FUNCTION(reverse);
		FUNCTION(filter);
		FUNCTION(find);
		FUNCTION(map);
		FUNCTION(zip);
		FUNCTION(take_while);
		FUNCTION(reduce);
		FUNCTION(sum);
		FUNCTION(head);
		FUNCTION(tail);
		FUNCTION(size);
		FUNCTION(null);
		FUNCTION(ceil);
		FUNCTION(floor);
		FUNCTION(trunc);
		FUNCTION(frac);
		FUNCTION(sgn);
		FUNCTION(round);
		FUNCTION(as_decimal);
		FUNCTION(pair);
		FUNCTION(loc);
		FUNCTION(distance_between);
		FUNCTION(index_of);
		FUNCTION(keys);
		FUNCTION(values);
		FUNCTION(tolist);
		FUNCTION(tomap);
		FUNCTION(substring);
		FUNCTION(replace);
		FUNCTION(length);
		FUNCTION(concatenate);
		FUNCTION(sin);
		FUNCTION(cos);
		FUNCTION(tan);
		FUNCTION(asin);
		FUNCTION(acos);
		FUNCTION(atan);
		FUNCTION(sqrt);
		FUNCTION(cbrt);
		FUNCTION(root);
		FUNCTION(log);
		FUNCTION(exp);
		FUNCTION(pi);
		FUNCTION(hypot);
		FUNCTION(type);
#undef FUNCTION
	}

	return functions_table;
}

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

	expression_ptr res(get_functions_map().create_function(fn, args));
	if(!res) {
		throw formula_error("Unknown function: " + fn, "", "", 0);
	}

	return res;
}

std::vector<std::string> builtin_function_names()
{
	return get_functions_map().get_function_names();
}

}
