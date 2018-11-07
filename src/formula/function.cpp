/*
   Copyright (C) 2008 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "formula/function.hpp"

#include "color.hpp"
#include "formula/callable_objects.hpp"
#include "formula/debugger.hpp"
#include "game_config.hpp"
#include "game_display.hpp"
#include "log.hpp"

#include <boost/math/constants/constants.hpp>
#include <cctype>
#include <deque>

using namespace boost::math::constants;

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)
static lg::log_domain log_scripting_formula("scripting/formula");
#define LOG_SF LOG_STREAM(info, log_scripting_formula)
#define WRN_SF LOG_STREAM(warn, log_scripting_formula)
#define ERR_SF LOG_STREAM(err, log_scripting_formula)

namespace wfl
{
static std::deque<std::string> call_stack;

call_stack_manager::call_stack_manager(const std::string& str)
{
	call_stack.push_back(str);
}

call_stack_manager::~call_stack_manager()
{
	call_stack.pop_back();
}

std::string call_stack_manager::get()
{
	std::ostringstream res;
	for(const auto& frame : call_stack) {
		if(!frame.empty()) {
			res << "  " << frame << "\n";
		}
	}

	return res.str();
}

std::string function_expression::str() const
{
	std::stringstream s;
	s << get_name();
	s << '(';
	bool first_arg = true;
	for(expression_ptr a : args()) {
		if(!first_arg) {
			s << ',';
		} else {
			first_arg = false;
		}
		s << a->str();
	}
	s << ')';
	return s.str();
}

namespace builtins
{
DEFINE_WFL_FUNCTION(debug, 0, 1)
{
	std::shared_ptr<formula_debugger> fdbp;
	bool need_wrapper = false;

	if(fdb == nullptr) {
		fdbp.reset(new formula_debugger());
		fdb = fdbp.get();
		need_wrapper = true;
	}

	if(args().size() == 1) {
		if(!need_wrapper) {
			return args()[0]->evaluate(variables, fdb);
		} else {
			return wrapper_formula(args()[0]).evaluate(variables, fdb);
		}
	}

	return wrapper_formula().evaluate(variables, fdb);
}

DEFINE_WFL_FUNCTION(dir, 1, 1)
{
	variant var = args()[0]->evaluate(variables, fdb);

	auto callable = var.as_callable();
	formula_input_vector inputs = callable->inputs();

	std::vector<variant> res;
	for(std::size_t i = 0; i < inputs.size(); ++i) {
		const formula_input& input = inputs[i];
		res.emplace_back(input.name);
	}

	return variant(res);
}

DEFINE_WFL_FUNCTION(if, 2, -1)
{
	for(std::size_t n = 0; n < args().size() - 1; n += 2) {
		if(args()[n]->evaluate(variables, fdb).as_bool()) {
			return args()[n + 1]->evaluate(variables, fdb);
		}
	}

	if((args().size() % 2) != 0) {
		return args().back()->evaluate(variables, fdb);
	} else {
		return variant();
	}
}

DEFINE_WFL_FUNCTION(switch, 3, -1)
{
	variant var = args()[0]->evaluate(variables, fdb);

	for(std::size_t n = 1; n < args().size() - 1; n += 2) {
		variant val = args()[n]->evaluate(variables, fdb);

		if(val == var) {
			return args()[n + 1]->evaluate(variables, fdb);
		}
	}

	if((args().size() % 2) == 0) {
		return args().back()->evaluate(variables, fdb);
	} else {
		return variant();
	}
}

DEFINE_WFL_FUNCTION(abs, 1, 1)
{
	const variant input = args()[0]->evaluate(variables, fdb);
	if(input.is_decimal()) {
		const int n = input.as_decimal();
		return variant(n >= 0 ? n : -n, variant::DECIMAL_VARIANT);
	} else {
		const int n = input.as_int();
		return variant(n >= 0 ? n : -n);
	}
}

DEFINE_WFL_FUNCTION(min, 1, -1)
{
	variant res = args()[0]->evaluate(variables, fdb);
	if(res.is_list()) {
		if(res.is_empty()) {
			throw formula_error("min(list): list is empty", "", "", 0);
		}

		res = *std::min_element(res.begin(), res.end());
	}

	for(std::size_t n = 1; n < args().size(); ++n) {
		variant v = args()[n]->evaluate(variables, fdb);

		if(v.is_list()) {
			if(v.is_empty()) {
				continue;
			}

			v = *std::min_element(v.begin(), v.end());
		}

		if(res.is_null() || v < res) {
			res = v;
		}
	}

	return res;
}

DEFINE_WFL_FUNCTION(max, 1, -1)
{
	variant res = args()[0]->evaluate(variables, fdb);
	if(res.is_list()) {
		if(res.is_empty()) {
			throw formula_error("max(list): list is empty", "", "", 0);
		}

		res = *std::max_element(res.begin(), res.end());
	}

	for(std::size_t n = 1; n < args().size(); ++n) {
		variant v = args()[n]->evaluate(variables, fdb);

		if(v.is_list()) {
			if(v.is_empty()) {
				continue;
			}

			v = *std::max_element(v.begin(), v.end());
		}

		if(res.is_null() || v > res) {
			res = v;
		}
	}

	return res;
}

namespace
{
void display_float(const map_location& location, const std::string& text)
{
	game_display::get_singleton()->float_label(location, text, color_t(255, 0, 0));
}
} // end anon namespace

DEFINE_WFL_FUNCTION(debug_float, 2, 3)
{
	const args_list& arguments = args();
	const variant var0 = arguments[0]->evaluate(variables, fdb);
	const variant var1 = arguments[1]->evaluate(variables, fdb);

	const map_location location = var0.convert_to<location_callable>()->loc();
	std::string text;

	if(arguments.size() == 2) {
		text = var1.to_debug_string();
		display_float(location, text);
		return var1;
	} else {
		const variant var2 = arguments[2]->evaluate(variables, fdb);
		text = var1.string_cast() + var2.to_debug_string();
		display_float(location, text);
		return var2;
	}
}

DEFINE_WFL_FUNCTION(debug_print, 1, 2)
{
	const variant var1 = args()[0]->evaluate(variables, fdb);

	std::string str1, str2;

	if(args().size() == 1) {
		str1 = var1.to_debug_string(true);

		LOG_SF << str1 << std::endl;

		if(game_config::debug) {
			game_display::get_singleton()->get_chat_manager().add_chat_message(
				std::time(nullptr), "WFL", 0, str1, events::chat_handler::MESSAGE_PUBLIC, false);
		}

		return var1;
	} else {
		str1 = var1.string_cast();

		const variant var2 = args()[1]->evaluate(variables, fdb);
		str2 = var2.to_debug_string(true);

		LOG_SF << str1 << ": " << str2 << std::endl;

		if(game_config::debug && game_display::get_singleton()) {
			game_display::get_singleton()->get_chat_manager().add_chat_message(
				std::time(nullptr), str1, 0, str2, events::chat_handler::MESSAGE_PUBLIC, false);
		}

		return var2;
	}
}

DEFINE_WFL_FUNCTION(debug_profile, 1, 2)
{
	std::string speaker = "WFL";
	int i_value = 0;

	if(args().size() == 2) {
		speaker = args()[0]->evaluate(variables, fdb).string_cast();
		i_value = 1;
	}

	const variant value = args()[i_value]->evaluate(variables, fdb);
	long run_time = 0;

	for(int i = 1; i < 1000; i++) {
		const long start = SDL_GetTicks();
		args()[i_value]->evaluate(variables, fdb);
		run_time += SDL_GetTicks() - start;
	}

	std::ostringstream str;
	str << "Evaluated in " << (run_time / 1000.0) << " ms on average";

	LOG_SF << speaker << ": " << str.str() << std::endl;

	if(game_config::debug && game_display::get_singleton()) {
		game_display::get_singleton()->get_chat_manager().add_chat_message(
			std::time(nullptr), speaker, 0, str.str(), events::chat_handler::MESSAGE_PUBLIC, false);
	}

	return value;
}

DEFINE_WFL_FUNCTION(keys, 1, 1)
{
	const variant map = args()[0]->evaluate(variables, fdb);
	return map.get_keys();
}

DEFINE_WFL_FUNCTION(values, 1, 1)
{
	const variant map = args()[0]->evaluate(variables, fdb);
	return map.get_values();
}

DEFINE_WFL_FUNCTION(tolist, 1, 1)
{
	const variant var = args()[0]->evaluate(variables, fdb);

	std::vector<variant> tmp;
	for(variant_iterator it = var.begin(); it != var.end(); ++it) {
		tmp.push_back(*it);
	}

	return variant(tmp);
}

DEFINE_WFL_FUNCTION(tomap, 1, 2)
{
	const variant var_1 = args()[0]->evaluate(variables, fdb);

	std::map<variant, variant> tmp;

	if(args().size() == 2) {
		const variant var_2 = args()[1]->evaluate(variables, fdb);
		if(var_1.num_elements() != var_2.num_elements()) {
			return variant();
		}

		for(std::size_t i = 0; i < var_1.num_elements(); ++i) {
			tmp[var_1[i]] = var_2[i];
		}
	} else {
		for(variant_iterator it = var_1.begin(); it != var_1.end(); ++it) {
			if(auto kv = (*it).try_convert<key_value_pair>()) {
				tmp[kv->query_value("key")] = kv->query_value("value");
			} else {
				auto map_it = tmp.find(*it);

				if(map_it == tmp.end()) {
					tmp[*it] = variant(1);
				} else {
					map_it->second = variant(map_it->second.as_int() + 1);
				}
			}
		}
	}

	return variant(tmp);
}

DEFINE_WFL_FUNCTION(substring, 2, 3)
{
	std::string result = args()[0]->evaluate(variables, fdb).as_string();

	int offset = args()[1]->evaluate(variables, fdb).as_int();
	if(offset < 0) {
		offset += result.size();

		if(offset < 0) {
			offset = 0;
		}
	} else {
		if(static_cast<std::size_t>(offset) >= result.size()) {
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
	}

	return variant(result.substr(offset));
}

DEFINE_WFL_FUNCTION(replace, 3, 4)
{
	std::string result = args()[0]->evaluate(variables, fdb).as_string();
	std::string replacement = args().back()->evaluate(variables, fdb).as_string();

	int offset = args()[1]->evaluate(variables, fdb).as_int();
	if(offset < 0) {
		offset += result.size();

		if(offset < 0) {
			offset = 0;
		}
	} else {
		if(static_cast<std::size_t>(offset) >= result.size()) {
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
	}

	return variant(result.replace(offset, std::string::npos, replacement));
}

DEFINE_WFL_FUNCTION(insert, 3, 3)
{
	std::string result = args()[0]->evaluate(variables, fdb).as_string();
	std::string insert = args().back()->evaluate(variables, fdb).as_string();

	int offset = args()[1]->evaluate(variables, fdb).as_int();
	if(offset < 0) {
		offset += result.size();

		if(offset < 0) {
			offset = 0;
		}
	} else if(static_cast<std::size_t>(offset) >= result.size()) {
		return variant(result + insert);
	}

	return variant(result.insert(offset, insert));
}

DEFINE_WFL_FUNCTION(length, 1, 1)
{
	return variant(args()[0]->evaluate(variables, fdb).as_string().length());
}

DEFINE_WFL_FUNCTION(concatenate, 1, -1)
{
	std::string result;
	for(expression_ptr arg : args()) {
		result += arg->evaluate(variables, fdb).string_cast();
	}

	return variant(result);
}

DEFINE_WFL_FUNCTION(str_upper, 1, 1)
{
	std::string str = args()[0]->evaluate(variables, fdb).as_string();
	std::transform(str.begin(), str.end(), str.begin(), static_cast<int (*)(int)>(std::toupper));
	return variant(str);
}

DEFINE_WFL_FUNCTION(str_lower, 1, 1)
{
	std::string str = args()[0]->evaluate(variables, fdb).as_string();
	std::transform(str.begin(), str.end(), str.begin(), static_cast<int (*)(int)>(std::tolower));
	return variant(str);
}

DEFINE_WFL_FUNCTION(sin, 1, 1)
{
	const double angle = args()[0]->evaluate(variables, fdb).as_decimal() / 1000.0;
	const double result = std::sin(angle * pi<double>() / 180.0);
	return variant(result, variant::DECIMAL_VARIANT);
}

DEFINE_WFL_FUNCTION(cos, 1, 1)
{
	const double angle = args()[0]->evaluate(variables, fdb).as_decimal() / 1000.0;
	const double result = std::cos(angle * pi<double>() / 180.0);
	return variant(result, variant::DECIMAL_VARIANT);
}

DEFINE_WFL_FUNCTION(tan, 1, 1)
{
	const double angle = args()[0]->evaluate(variables, fdb).as_decimal() / 1000.0;
	const double result = std::tan(angle * pi<double>() / 180.0);
	if(std::isnan(result) || result <= INT_MIN || result >= INT_MAX) {
		return variant();
	}

	return variant(result, variant::DECIMAL_VARIANT);
}

DEFINE_WFL_FUNCTION(asin, 1, 1)
{
	const double num = args()[0]->evaluate(variables, fdb).as_decimal() / 1000.0;
	const double result = std::asin(num) * 180.0 / pi<double>();
	if(std::isnan(result)) {
		return variant();
	}

	return variant(result, variant::DECIMAL_VARIANT);
}

DEFINE_WFL_FUNCTION(acos, 1, 1)
{
	const double num = args()[0]->evaluate(variables, fdb).as_decimal() / 1000.0;
	const double result = std::acos(num) * 180.0 / pi<double>();
	if(std::isnan(result)) {
		return variant();
	}

	return variant(result, variant::DECIMAL_VARIANT);
}

DEFINE_WFL_FUNCTION(atan, 1, 1)
{
	const double num = args()[0]->evaluate(variables, fdb).as_decimal() / 1000.0;
	const double result = std::atan(num) * 180.0 / pi<double>();
	return variant(result, variant::DECIMAL_VARIANT);
}

DEFINE_WFL_FUNCTION(sqrt, 1, 1)
{
	const double num = args()[0]->evaluate(variables, fdb).as_decimal() / 1000.0;
	const double result = std::sqrt(num);
	if(std::isnan(result)) {
		return variant();
	}

	return variant(result, variant::DECIMAL_VARIANT);
}

DEFINE_WFL_FUNCTION(cbrt, 1, 1)
{
	const double num = args()[0]->evaluate(variables, fdb).as_decimal() / 1000.0;
	const double result = num < 0 ? -std::pow(-num, 1.0 / 3.0) : std::pow(num, 1.0 / 3.0);
	return variant(result, variant::DECIMAL_VARIANT);
}

DEFINE_WFL_FUNCTION(root, 2, 2)
{
	const double base = args()[0]->evaluate(variables, fdb).as_decimal() / 1000.0;
	const double root = args()[1]->evaluate(variables, fdb).as_decimal() / 1000.0;
	const double result = base < 0 && std::fmod(root, 2) == 1 ? -std::pow(-base, 1.0 / root) : std::pow(base, 1.0 / root);
	if(std::isnan(result)) {
		return variant();
	}

	return variant(result, variant::DECIMAL_VARIANT);
}

DEFINE_WFL_FUNCTION(log, 1, 2)
{
	const double num = args()[0]->evaluate(variables, fdb).as_decimal() / 1000.0;
	if(args().size() == 1) {
		const double result = std::log(num);
		if(std::isnan(result)) {
			return variant();
		}

		return variant(result, variant::DECIMAL_VARIANT);
	}

	const double base = args()[1]->evaluate(variables, fdb).as_decimal() / 1000.0;
	const double result = std::log(num) / std::log(base);
	if(std::isnan(result)) {
		return variant();
	}

	return variant(result, variant::DECIMAL_VARIANT);
}

DEFINE_WFL_FUNCTION(exp, 1, 1)
{
	const double num = args()[0]->evaluate(variables, fdb).as_decimal() / 1000.0;
	const double result = std::exp(num);
	if(result == 0 || result >= INT_MAX) {
		// These are range errors rather than NaNs,
		// but I figure it's better than returning INT_MIN.
		return variant();
	}

	return variant(result, variant::DECIMAL_VARIANT);
}

DEFINE_WFL_FUNCTION(pi, 0, 0)
{
	UNUSED(variables);
	UNUSED(fdb);
	return variant(pi<double>(), variant::DECIMAL_VARIANT);
}

DEFINE_WFL_FUNCTION(hypot, 2, 2)
{
	const double x = args()[0]->evaluate(variables, fdb).as_decimal() / 1000.0;
	const double y = args()[1]->evaluate(variables, fdb).as_decimal() / 1000.0;
	return variant(std::hypot(x, y), variant::DECIMAL_VARIANT);
}

DEFINE_WFL_FUNCTION(index_of, 2, 2)
{
	const variant value = args()[0]->evaluate(variables, fdb);
	const variant list = args()[1]->evaluate(variables, fdb);

	for(std::size_t i = 0; i < list.num_elements(); ++i) {
		if(list[i] == value) {
			return variant(i);
		}
	}

	return variant(-1);
}

DEFINE_WFL_FUNCTION(choose, 2, 3)
{
	const variant items = args()[0]->evaluate(variables, fdb);
	variant max_value;
	variant_iterator max;

	if(args().size() == 2) {
		for(variant_iterator it = items.begin(); it != items.end(); ++it) {
			const variant val = args().back()->evaluate(formula_variant_callable_with_backup(*it, variables), fdb);

			if(max == variant_iterator() || val > max_value) {
				max = it;
				max_value = val;
			}
		}
	} else {
		map_formula_callable self_callable;
		const std::string self = args()[1]->evaluate(variables, fdb).as_string();

		for(variant_iterator it = items.begin(); it != items.end(); ++it) {
			self_callable.add(self, *it);

			const variant val = args().back()->evaluate(
				formula_callable_with_backup(self_callable, formula_variant_callable_with_backup(*it, variables)), fdb);

			if(max == variant_iterator() || val > max_value) {
				max = it;
				max_value = val;
			}
		}
	}

	if(max == variant_iterator()) {
		return variant();
	}

	return *max;
}

DEFINE_WFL_FUNCTION(wave, 1, 1)
{
	const int value = args()[0]->evaluate(variables, fdb).as_int() % 1000;
	const double angle = 2.0 * pi<double>() * (static_cast<double>(value) / 1000.0);
	return variant(static_cast<int>(std::sin(angle) * 1000.0));
}

namespace
{
class variant_comparator : public formula_callable
{
public:
	variant_comparator(const expression_ptr& expr, const formula_callable& fallback)
		: expr_(expr)
		, fallback_(&fallback)
		, a_()
		, b_()
	{
	}

	bool operator()(const variant& a, const variant& b) const
	{
		a_ = a;
		b_ = b;
		return expr_->evaluate(*this).as_bool();
	}

private:
	variant get_value(const std::string& key) const
	{
		if(key == "a") {
			return a_;
		} else if(key == "b") {
			return b_;
		} else {
			return fallback_->query_value(key);
		}
	}

	void get_inputs(formula_input_vector& inputs) const
	{
		fallback_->get_inputs(inputs);
	}

	expression_ptr expr_;
	const formula_callable* fallback_;
	mutable variant a_, b_;
};
} // end anon namespace

DEFINE_WFL_FUNCTION(sort, 1, 2)
{
	variant list = args()[0]->evaluate(variables, fdb);

	std::vector<variant> vars;
	vars.reserve(list.num_elements());

	for(std::size_t n = 0; n != list.num_elements(); ++n) {
		vars.push_back(list[n]);
	}

	if(args().size() == 1) {
		std::sort(vars.begin(), vars.end());
	} else {
		std::sort(vars.begin(), vars.end(), variant_comparator(args()[1], variables));
	}

	return variant(vars);
}

DEFINE_WFL_FUNCTION(reverse, 1, 1)
{
	const variant& arg = args()[0]->evaluate(variables, fdb);

	if(arg.is_string()) {
		std::string str = args()[0]->evaluate(variables, fdb).as_string();
		std::reverse(str.begin(), str.end());

		return variant(str);
	} else if(arg.is_list()) {
		std::vector<variant> list = args()[0]->evaluate(variables, fdb).as_list();
		std::reverse(list.begin(), list.end());

		return variant(list);
	}

	return variant();
}

DEFINE_WFL_FUNCTION(contains_string, 2, 2)
{
	std::string str = args()[0]->evaluate(variables, fdb).as_string();
	std::string key = args()[1]->evaluate(variables, fdb).as_string();

	return variant(str.find(key) != std::string::npos);
}

DEFINE_WFL_FUNCTION(find_string, 2, 2)
{
	const std::string str = args()[0]->evaluate(variables, fdb).as_string();
	const std::string key = args()[1]->evaluate(variables, fdb).as_string();

	std::size_t pos = str.find(key);
	return variant(static_cast<int>(pos));
}

DEFINE_WFL_FUNCTION(filter, 2, 3)
{
	std::vector<variant> list_vars;
	std::map<variant, variant> map_vars;

	const variant items = args()[0]->evaluate(variables, fdb);

	if(args().size() == 2) {
		for(variant_iterator it = items.begin(); it != items.end(); ++it) {
			const variant val = args()[1]->evaluate(formula_variant_callable_with_backup(*it, variables), fdb);

			if(val.as_bool()) {
				if(items.is_map()) {
					map_vars[(*it).get_member("key")] = (*it).get_member("value");
				} else {
					list_vars.push_back(*it);
				}
			}
		}
	} else {
		map_formula_callable self_callable;
		const std::string self = args()[1]->evaluate(variables, fdb).as_string();

		for(variant_iterator it = items.begin(); it != items.end(); ++it) {
			self_callable.add(self, *it);

			const variant val = args()[2]->evaluate(
				formula_callable_with_backup(self_callable, formula_variant_callable_with_backup(*it, variables)), fdb);

			if(val.as_bool()) {
				if(items.is_map()) {
					map_vars[(*it).get_member("key")] = (*it).get_member("value");
				} else {
					list_vars.push_back(*it);
				}
			}
		}
	}

	if(items.is_map()) {
		return variant(map_vars);
	}

	return variant(list_vars);
}

DEFINE_WFL_FUNCTION(find, 2, 3)
{
	const variant items = args()[0]->evaluate(variables, fdb);

	if(args().size() == 2) {
		for(variant_iterator it = items.begin(); it != items.end(); ++it) {
			const variant val = args()[1]->evaluate(formula_variant_callable_with_backup(*it, variables), fdb);
			if(val.as_bool()) {
				return *it;
			}
		}
	} else {
		map_formula_callable self_callable;
		const std::string self = args()[1]->evaluate(variables, fdb).as_string();

		for(variant_iterator it = items.begin(); it != items.end(); ++it) {
			self_callable.add(self, *it);

			const variant val = args().back()->evaluate(
				formula_callable_with_backup(self_callable, formula_variant_callable_with_backup(*it, variables)), fdb);

			if(val.as_bool()) {
				return *it;
			}
		}
	}

	return variant();
}

DEFINE_WFL_FUNCTION(map, 2, 3)
{
	std::vector<variant> list_vars;
	std::map<variant, variant> map_vars;
	const variant items = args()[0]->evaluate(variables, fdb);

	if(args().size() == 2) {
		for(variant_iterator it = items.begin(); it != items.end(); ++it) {
			const variant val = args().back()->evaluate(formula_variant_callable_with_backup(*it, variables), fdb);
			if(items.is_map()) {
				map_vars[(*it).get_member("key")] = val;
			} else {
				list_vars.push_back(val);
			}
		}
	} else {
		map_formula_callable self_callable;
		const std::string self = args()[1]->evaluate(variables, fdb).as_string();

		for(variant_iterator it = items.begin(); it != items.end(); ++it) {
			self_callable.add(self, *it);

			const variant val = args().back()->evaluate(
				formula_callable_with_backup(self_callable, formula_variant_callable_with_backup(*it, variables)), fdb);

			if(items.is_map()) {
				map_vars[(*it).get_member("key")] = val;
			} else {
				list_vars.push_back(val);
			}
		}
	}

	if(items.is_map()) {
		return variant(map_vars);
	}

	return variant(list_vars);
}

DEFINE_WFL_FUNCTION(take_while, 2, 2)
{
	const variant& items = args()[0]->evaluate(variables, fdb);

	variant_iterator it = items.begin();
	for(; it != items.end(); ++it) {
		const variant matches = args().back()->evaluate(formula_variant_callable_with_backup(*it, variables), fdb);

		if(!matches.as_bool()) {
			break;
		}
	}

	std::vector<variant> result(items.begin(), it);
	return variant(result);
}

namespace
{
struct indexer
{
	explicit indexer(std::size_t i)
		: i(i)
	{
	}

	variant operator()(const variant& v) const
	{
		if(i >= v.num_elements()) {
			return variant();
		} else {
			return v[i];
		}
	}

	std::size_t i;
};

/** @todo: replace with lambda? */
struct comparator
{
	bool operator()(const variant& a, const variant& b) const
	{
		return a.num_elements() < b.num_elements();
	}
};

std::vector<variant> get_input(
		const function_expression::args_list& args,
		const formula_callable& variables,
		formula_debugger* fdb)
{
	if(args.size() == 1) {
		const variant list = args[0]->evaluate(variables, fdb);
		return std::vector<variant>(list.begin(), list.end());
	} else {
		std::vector<variant> input;
		input.reserve(args.size());

		for(expression_ptr expr : args) {
			input.push_back(expr->evaluate(variables, fdb));
		}

		return input;
	}
}
} // end anon namespace

DEFINE_WFL_FUNCTION(zip, 1, -1)
{
	const std::vector<variant> input = get_input(args(), variables, fdb);
	std::vector<variant> output;

	// So basically this does [[a,b,c],[d,e,f],[x,y,z]] -> [[a,d,x],[b,e,y],[c,f,z]]
	// Or [[a,b,c,d],[x,y,z]] -> [[a,x],[b,y],[c,z],[d,null()]]
	std::size_t max_i = std::max_element(input.begin(), input.end(), comparator())->num_elements();
	output.reserve(max_i);

	for(std::size_t i = 0; i < max_i; i++) {
		std::vector<variant> elem(input.size());
		std::transform(input.begin(), input.end(), elem.begin(), indexer(i));
		output.emplace_back(elem);
	}

	return variant(output);
}

DEFINE_WFL_FUNCTION(reduce, 2, 3)
{
	const variant items = args()[0]->evaluate(variables, fdb);
	const variant initial = args().size() == 2 ? variant() : args()[1]->evaluate(variables, fdb);

	if(items.num_elements() == 0) {
		return initial;
	}

	variant_iterator it = items.begin();
	variant res(initial.is_null() ? *it : initial);
	if(res != initial) {
		++it;
	}

	map_formula_callable self_callable;
	for(; it != items.end(); ++it) {
		self_callable.add("a", res);
		self_callable.add("b", *it);
		res = args().back()->evaluate(
			formula_callable_with_backup(self_callable, formula_variant_callable_with_backup(*it, variables)), fdb);
	}

	return res;
}

DEFINE_WFL_FUNCTION(sum, 1, 2)
{
	variant res(0);
	const variant items = args()[0]->evaluate(variables, fdb);
	if(items.num_elements() > 0) {
		if(items[0].is_list()) {
			std::vector<variant> tmp;
			res = variant(tmp);
			if(args().size() >= 2) {
				res = args()[1]->evaluate(variables, fdb);
				if(!res.is_list())
					return variant();
			}
		} else if(items[0].is_map()) {
			std::map<variant, variant> tmp;
			res = variant(tmp);
			if(args().size() >= 2) {
				res = args()[1]->evaluate(variables, fdb);
				if(!res.is_map())
					return variant();
			}
		} else {
			if(args().size() >= 2) {
				res = args()[1]->evaluate(variables, fdb);
			}
		}
	}

	for(std::size_t n = 0; n != items.num_elements(); ++n) {
		res = res + items[n];
	}

	return res;
}

DEFINE_WFL_FUNCTION(head, 1, 2)
{
	const variant items = args()[0]->evaluate(variables, fdb);
	variant_iterator it = items.begin();
	if(it == items.end()) {
		return variant();
	}

	if(args().size() == 1) {
		return *it;
	}

	const int n = items.num_elements(), req = args()[1]->evaluate(variables, fdb).as_int();
	const int count = req < 0 ? n - std::min(-req, n) : std::min(req, n);

	variant_iterator end = it;
	std::advance(end, count);

	std::vector<variant> res;
	std::copy(it, end, std::back_inserter(res));
	return variant(res);
}

DEFINE_WFL_FUNCTION(tail, 1, 2)
{
	const variant items = args()[0]->evaluate(variables, fdb);
	variant_iterator it = items.end();
	if(it == items.begin()) {
		return variant();
	}

	if(args().size() == 1) {
		return *--it;
	}

	const int n = items.num_elements(), req = args()[1]->evaluate(variables, fdb).as_int();
	const int count = req < 0 ? n - std::min(-req, n) : std::min(req, n);

	std::advance(it, -count);
	std::vector<variant> res;

	std::copy(it, items.end(), std::back_inserter(res));
	return variant(res);
}

DEFINE_WFL_FUNCTION(size, 1, 1)
{
	const variant items = args()[0]->evaluate(variables, fdb);
	return variant(static_cast<int>(items.num_elements()));
}

DEFINE_WFL_FUNCTION(null, 0, -1)
{
	if(!args().empty()) {
		for(std::size_t i = 0; i < args().size(); ++i) {
			args()[i]->evaluate(variables, fdb);
		}
	}

	return variant();
}

DEFINE_WFL_FUNCTION(ceil, 1, 1)
{
	variant decimal = args()[0]->evaluate(variables, fdb);
	int d = decimal.as_decimal();

	if((d >= 0) && (d % 1000 != 0)) {
		d /= 1000;
		return variant(++d);
	} else {
		d /= 1000;
		return variant(d);
	}
}

DEFINE_WFL_FUNCTION(round, 1, 1)
{
	variant decimal = args()[0]->evaluate(variables, fdb);
	int d = decimal.as_decimal();
	int f = d % 1000;

	if(f >= 500) {
		d /= 1000;
		return variant(++d);
	} else if(f <= -500) {
		d /= 1000;
		return variant(--d);
	} else {
		d /= 1000;
		return variant(d);
	}
}

DEFINE_WFL_FUNCTION(floor, 1, 1)
{
	variant decimal = args()[0]->evaluate(variables, fdb);
	int d = decimal.as_decimal();

	if((d < 0) && (d % 1000 != 0)) {
		d /= 1000;
		return variant(--d);
	} else {
		d /= 1000;
		return variant(d);
	}
}

DEFINE_WFL_FUNCTION(trunc, 1, 1)
{
	variant decimal = args()[0]->evaluate(variables, fdb);
	int d = decimal.as_int();

	return variant(d);
}

DEFINE_WFL_FUNCTION(frac, 1, 1)
{
	variant decimal = args()[0]->evaluate(variables, fdb);
	int d = decimal.as_decimal();

	d %= 1000;
	return variant(d, variant::DECIMAL_VARIANT);
}

DEFINE_WFL_FUNCTION(sgn, 1, 1)
{
	variant decimal = args()[0]->evaluate(variables, fdb);
	int d = decimal.as_decimal();

	if(d != 0) {
		d = d > 0 ? 1 : -1;
	}

	return variant(d);
}

DEFINE_WFL_FUNCTION(as_decimal, 1, 1)
{
	variant decimal = args()[0]->evaluate(variables, fdb);
	int d = decimal.as_decimal();

	return variant(d, variant::DECIMAL_VARIANT);
}

DEFINE_WFL_FUNCTION(loc, 2, 2)
{
	return variant(std::make_shared<location_callable>(map_location(
		args()[0]->evaluate(variables, add_debug_info(fdb, 0, "loc:x")).as_int(),
		args()[1]->evaluate(variables, add_debug_info(fdb, 1, "loc:y")).as_int(), wml_loc()
	)));
}

DEFINE_WFL_FUNCTION(pair, 2, 2)
{
	return variant(std::make_shared<key_value_pair>(
		args()[0]->evaluate(variables, add_debug_info(fdb, 0, "pair:key")),
		args()[1]->evaluate(variables, add_debug_info(fdb, 1, "pair_value"))
	));
}

DEFINE_WFL_FUNCTION(distance_between, 2, 2)
{
	const map_location loc1 = args()[0]
		->evaluate(variables, add_debug_info(fdb, 0, "distance_between:location_A"))
		.convert_to<location_callable>()
		->loc();

	const map_location loc2 = args()[1]
		->evaluate(variables, add_debug_info(fdb, 1, "distance_between:location_B"))
		.convert_to<location_callable>()
		->loc();

	return variant(distance_between(loc1, loc2));
}

DEFINE_WFL_FUNCTION(adjacent_locs, 1, 1)
{
	const map_location loc = args()[0]
		->evaluate(variables, add_debug_info(fdb, 0, "adjacent_locs:location"))
		.convert_to<location_callable>()
		->loc();

	adjacent_loc_array_t adj;
	get_adjacent_tiles(loc, adj.data());

	std::vector<variant> v;
	for(unsigned n = 0; n < adj.size(); ++n) {
		v.emplace_back(std::make_shared<location_callable>(adj[n]));
	}

	return variant(v);
}

DEFINE_WFL_FUNCTION(are_adjacent, 2, 2)
{
	const map_location loc1 = args()[0]
		->evaluate(variables, add_debug_info(fdb, 0, "are_adjacent:location_A"))
		.convert_to<location_callable>()
		->loc();

	const map_location loc2 = args()[1]
		->evaluate(variables, add_debug_info(fdb, 1, "are_adjacent:location_B"))
		.convert_to<location_callable>()
		->loc();

	return variant(tiles_adjacent(loc1, loc2) ? 1 : 0);
}

DEFINE_WFL_FUNCTION(relative_dir, 2, 2)
{
	const map_location loc1 = args()[0]
		->evaluate(variables, add_debug_info(fdb, 0, "relative_dir:location_A"))
		.convert_to<location_callable>()
		->loc();

	const map_location loc2 = args()[1]
		->evaluate(variables, add_debug_info(fdb, 1, "relative_dir:location_B"))
		.convert_to<location_callable>()
		->loc();

	return variant(map_location::write_direction(loc1.get_relative_dir(loc2)));
}

DEFINE_WFL_FUNCTION(direction_from, 2, 3)
{
	const map_location loc = args()[0]
		->evaluate(variables, add_debug_info(fdb, 0, "direction_from:location"))
		.convert_to<location_callable>()
		->loc();

	const std::string dir_str =
		args()[1]->evaluate(variables, add_debug_info(fdb, 1, "direction_from:dir")).as_string();

	int n = args().size() == 3
		? args()[2]->evaluate(variables, add_debug_info(fdb, 2, "direction_from:count")).as_int()
		: 1;

	return variant(std::make_shared<location_callable>(loc.get_direction(map_location::parse_direction(dir_str), n)));
}

DEFINE_WFL_FUNCTION(rotate_loc_around, 2, 3)
{
	const map_location center = args()[0]
		->evaluate(variables, add_debug_info(fdb, 0, "direction_from:center"))
		.convert_to<location_callable>()
		->loc();

	const map_location loc = args()[0]
		->evaluate(variables, add_debug_info(fdb, 1, "direction_from:location"))
		.convert_to<location_callable>()
		->loc();

	int n = args().size() == 3
		? args()[2]->evaluate(variables, add_debug_info(fdb, 2, "direction_from:count")).as_int()
		: 1;

	return variant(std::make_shared<location_callable>(loc.rotate_right_around_center(center, n)));
}

DEFINE_WFL_FUNCTION(type, 1, 1)
{
	const variant& v = args()[0]->evaluate(variables, fdb);
	return variant(v.type_string());
}

} // namespace builtins

namespace actions
{
DEFINE_WFL_FUNCTION(safe_call, 2, 2)
{
	const variant main = args()[0]->evaluate(variables, fdb);
	const expression_ptr backup_formula = args()[1];

	return variant(std::make_shared<safe_call_callable>(main, backup_formula));
}

DEFINE_WFL_FUNCTION(set_var, 2, 2)
{
	return variant(std::make_shared<set_var_callable>(
	args()[0]->evaluate(variables, add_debug_info(fdb, 0, "set_var:key")).as_string(),
	args()[1]->evaluate(variables, add_debug_info(fdb, 1, "set_var:value"))));
}

} // namespace actions

variant key_value_pair::get_value(const std::string& key) const
{
	if(key == "key") {
		return key_;
	} else if(key == "value") {
		return value_;
	}

	return variant();
}

void key_value_pair::get_inputs(formula_input_vector& inputs) const
{
	add_input(inputs, "key");
	add_input(inputs, "value");
}

void key_value_pair::serialize_to_string(std::string& str) const
{
	str += "pair(";
	str += key_.serialize_to_string();
	str += ",";
	str += value_.serialize_to_string();
	str += ")";
}

formula_function_expression::formula_function_expression(const std::string& name,
		const args_list& args,
		const_formula_ptr formula,
		const_formula_ptr precondition,
		const std::vector<std::string>& arg_names)
	: function_expression(name, args, arg_names.size(), arg_names.size())
	, formula_(formula)
	, precondition_(precondition)
	, arg_names_(arg_names)
	, star_arg_(-1)
{
	for(std::size_t n = 0; n != arg_names_.size(); ++n) {
		if(arg_names_[n].empty() == false && arg_names_[n].back() == '*') {
			arg_names_[n].resize(arg_names_[n].size() - 1);
			star_arg_ = n;
			break;
		}
	}
}

variant formula_function_expression::execute(const formula_callable& variables, formula_debugger* fdb) const
{
	static std::string indent;
	indent += "  ";

	DBG_NG << indent << "executing '" << formula_->str() << "'\n";

	const int begin_time = SDL_GetTicks();
	map_formula_callable callable;

	for(std::size_t n = 0; n != arg_names_.size(); ++n) {
		variant var = args()[n]->evaluate(variables, fdb);
		callable.add(arg_names_[n], var);

		if(static_cast<int>(n) == star_arg_) {
			callable.set_fallback(var.as_callable());
		}
	}

	if(precondition_) {
		if(!precondition_->evaluate(callable, fdb).as_bool()) {
			DBG_NG << "FAILED function precondition for function '" << formula_->str() << "' with arguments: ";

			for(std::size_t n = 0; n != arg_names_.size(); ++n) {
				DBG_NG << "  arg " << (n + 1) << ": " << args()[n]->evaluate(variables, fdb).to_debug_string() << "\n";
			}
		}
	}

	variant res = formula_->evaluate(callable, fdb);

	const int taken = SDL_GetTicks() - begin_time;
	DBG_NG << indent << "returning: " << taken << "\n";

	indent.resize(indent.size() - 2);

	return res;
}

function_expression_ptr user_formula_function::generate_function_expression(
		const std::vector<expression_ptr>& args) const
{
	return std::make_shared<formula_function_expression>(name_, args, formula_, precondition_, args_);
}

function_symbol_table::function_symbol_table(std::shared_ptr<function_symbol_table> parent)
	: parent(parent ? parent : get_builtins())
{
}

void function_symbol_table::add_function(const std::string& name, formula_function_ptr&& fcn)
{
	custom_formulas_.emplace(name, std::move(fcn));
}

expression_ptr function_symbol_table::create_function(
		const std::string& fn, const std::vector<expression_ptr>& args) const
{
	const auto i = custom_formulas_.find(fn);
	if(i != custom_formulas_.end()) {
		return i->second->generate_function_expression(args);
	}

	if(parent) {
		expression_ptr res(parent->create_function(fn, args));
		if(res) {
			return res;
		}
	}

	throw formula_error("Unknown function: " + fn, "", "", 0);
}

std::set<std::string> function_symbol_table::get_function_names() const
{
	std::set<std::string> res;
	if(parent) {
		res = parent->get_function_names();
	}

	for(const auto& formula : custom_formulas_) {
		res.insert(formula.first);
	}

	return res;
}

std::shared_ptr<function_symbol_table> function_symbol_table::get_builtins()
{
	static function_symbol_table functions_table(builtins_tag);

	if(functions_table.empty()) {
		functions_table.parent = nullptr;

		using namespace builtins;
		DECLARE_WFL_FUNCTION(debug);
		DECLARE_WFL_FUNCTION(dir);
		DECLARE_WFL_FUNCTION(if);
		DECLARE_WFL_FUNCTION(switch);
		DECLARE_WFL_FUNCTION(abs);
		DECLARE_WFL_FUNCTION(min);
		DECLARE_WFL_FUNCTION(max);
		DECLARE_WFL_FUNCTION(choose);
		DECLARE_WFL_FUNCTION(debug_float);
		DECLARE_WFL_FUNCTION(debug_print);
		DECLARE_WFL_FUNCTION(debug_profile);
		DECLARE_WFL_FUNCTION(wave);
		DECLARE_WFL_FUNCTION(sort);
		DECLARE_WFL_FUNCTION(contains_string);
		DECLARE_WFL_FUNCTION(find_string);
		DECLARE_WFL_FUNCTION(reverse);
		DECLARE_WFL_FUNCTION(filter);
		DECLARE_WFL_FUNCTION(find);
		DECLARE_WFL_FUNCTION(map);
		DECLARE_WFL_FUNCTION(zip);
		DECLARE_WFL_FUNCTION(take_while);
		DECLARE_WFL_FUNCTION(reduce);
		DECLARE_WFL_FUNCTION(sum);
		DECLARE_WFL_FUNCTION(head);
		DECLARE_WFL_FUNCTION(tail);
		DECLARE_WFL_FUNCTION(size);
		DECLARE_WFL_FUNCTION(null);
		DECLARE_WFL_FUNCTION(ceil);
		DECLARE_WFL_FUNCTION(floor);
		DECLARE_WFL_FUNCTION(trunc);
		DECLARE_WFL_FUNCTION(frac);
		DECLARE_WFL_FUNCTION(sgn);
		DECLARE_WFL_FUNCTION(round);
		DECLARE_WFL_FUNCTION(as_decimal);
		DECLARE_WFL_FUNCTION(pair);
		DECLARE_WFL_FUNCTION(loc);
		DECLARE_WFL_FUNCTION(distance_between);
		DECLARE_WFL_FUNCTION(adjacent_locs);
		DECLARE_WFL_FUNCTION(are_adjacent);
		DECLARE_WFL_FUNCTION(relative_dir);
		DECLARE_WFL_FUNCTION(direction_from);
		DECLARE_WFL_FUNCTION(rotate_loc_around);
		DECLARE_WFL_FUNCTION(index_of);
		DECLARE_WFL_FUNCTION(keys);
		DECLARE_WFL_FUNCTION(values);
		DECLARE_WFL_FUNCTION(tolist);
		DECLARE_WFL_FUNCTION(tomap);
		DECLARE_WFL_FUNCTION(substring);
		DECLARE_WFL_FUNCTION(replace);
		DECLARE_WFL_FUNCTION(length);
		DECLARE_WFL_FUNCTION(concatenate);
		DECLARE_WFL_FUNCTION(sin);
		DECLARE_WFL_FUNCTION(cos);
		DECLARE_WFL_FUNCTION(tan);
		DECLARE_WFL_FUNCTION(asin);
		DECLARE_WFL_FUNCTION(acos);
		DECLARE_WFL_FUNCTION(atan);
		DECLARE_WFL_FUNCTION(sqrt);
		DECLARE_WFL_FUNCTION(cbrt);
		DECLARE_WFL_FUNCTION(root);
		DECLARE_WFL_FUNCTION(log);
		DECLARE_WFL_FUNCTION(exp);
		DECLARE_WFL_FUNCTION(pi);
		DECLARE_WFL_FUNCTION(hypot);
		DECLARE_WFL_FUNCTION(type);
	}

	return std::shared_ptr<function_symbol_table>(&functions_table, [](function_symbol_table*) {});
}

action_function_symbol_table::action_function_symbol_table(std::shared_ptr<function_symbol_table> parent)
	: function_symbol_table(parent)
{
	using namespace actions;
	function_symbol_table& functions_table = *this;
	DECLARE_WFL_FUNCTION(safe_call);
	DECLARE_WFL_FUNCTION(set_var);
}
}
