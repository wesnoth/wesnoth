/*
   Copyright (C) 2008 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include <cassert>
#include <cmath>
#include <iostream>
#include <cstring>
#include <stack>

#include "formatter.hpp"
#include "formula/function.hpp"
#include "utils/math.hpp"
#include "log.hpp"

static lg::log_domain log_scripting_formula("scripting/formula");
#define DBG_SF LOG_STREAM(debug, log_scripting_formula)
#define LOG_SF LOG_STREAM(info, log_scripting_formula)
#define WRN_SF LOG_STREAM(warn, log_scripting_formula)
#define ERR_SF LOG_STREAM(err, log_scripting_formula)

#include <cassert>
#include <cmath>
#include <memory>

namespace wfl
{

// Static value to initialize null variants to ensure its value is never nullptr.
static value_base_ptr null_value(new variant_value_base);

static std::string variant_type_to_string(VARIANT_TYPE type)
{
	return VARIANT_TYPE::enum_to_string(type);
}

// Small helper function to get a standard type error message.
static std::string was_expecting(const std::string& message, const variant& v)
{
	std::ostringstream ss;

	ss << "TYPE ERROR: expected " << message << " but found "
	   << v.type_string() << " (" << v.to_debug_string() << ")";

	return ss.str();
}

type_error::type_error(const std::string& str) : game::error(str)
{
	std::cerr << "ERROR: " << message << "\n" << call_stack_manager::get();
}

variant_iterator::variant_iterator()
	: type_(VARIANT_TYPE::TYPE_NULL)
	, container_(nullptr)
	, iter_()
{
}

variant_iterator::variant_iterator(const variant_value_base* value, const boost::any& iter)
	: type_(value->get_type())
	, container_(value)
	, iter_(iter)
{
}

variant variant_iterator::operator*() const
{
	if(!container_) {
		return variant();
	}

	return container_->deref_iterator(iter_);
}

variant_iterator& variant_iterator::operator++()
{
	if(container_) {
		container_->iterator_inc(iter_);
	}

	return *this;
}

variant_iterator variant_iterator::operator++(int)
{
	variant_iterator temp(*this);
	if(container_) {
		container_->iterator_inc(iter_);
	}

	return temp;
}

variant_iterator& variant_iterator::operator--()
{
	if(container_) {
		container_->iterator_dec(iter_);
	}

	return *this;
}

variant_iterator variant_iterator::operator--(int)
{
	variant_iterator temp(*this);
	if(container_) {
		container_->iterator_dec(iter_);
	}

	return temp;
}

bool variant_iterator::operator==(const variant_iterator& that) const
{
	if(!container_ && !that.container_) {
		return true;
	}

	if(container_ == that.container_) {
		return container_->iterator_equals(iter_, that.iter_);
	}

	return false;
}

bool variant_iterator::operator!=(const variant_iterator& that) const
{
	return !operator==(that);
}


variant::variant()
	: value_(null_value)
{}

variant::variant(int n)
	: value_(std::make_shared<variant_int>(n))
{
	assert(value_.get());
}

variant::variant(int n, variant::DECIMAL_VARIANT_TYPE)
	: value_(std::make_shared<variant_decimal>(n))
{
	assert(value_.get());
}

variant::variant(double n, variant::DECIMAL_VARIANT_TYPE)
	: value_(std::make_shared<variant_decimal>(n))
{
	assert(value_.get());
}

variant::variant(const std::vector<variant>& vec)
    : value_((std::make_shared<variant_list>(vec)))
{
	assert(value_.get());
}

variant::variant(const std::string& str)
	: value_(std::make_shared<variant_string>(str))
{
	assert(value_.get());
}

variant::variant(const std::map<variant,variant>& map)
	: value_((std::make_shared<variant_map>(map)))
{
	assert(value_.get());
}

variant& variant::operator=(const variant& v)
{
	value_ = v.value_;
	return *this;
}

variant variant::operator[](size_t n) const
{
	if(is_callable()) {
		return *this;
	}

	must_be(VARIANT_TYPE::TYPE_LIST);

	try {
		return value_cast<variant_list>()->get_container()[n];
	} catch(std::out_of_range&) {
		throw type_error("invalid index");
	}
}

variant variant::operator[](const variant& v) const
{
	if(is_callable()) {
		return *this;
	}

	if(is_map()) {
		auto& map = value_cast<variant_map>()->get_container();

		auto i = map.find(v);
		if(i == map.end()) {
			return variant();
		}

		return i->second;
	} else if(is_list()) {
		if(v.is_list()) {
			std::vector<variant> slice;
			for(size_t i = 0; i < v.num_elements(); ++i) {
				slice.push_back((*this)[v[i]]);
			}

			return variant(slice);
		} else if(v.as_int() < 0) {
			return operator[](num_elements() + v.as_int());
		}

		return operator[](v.as_int());
	}

	throw type_error(was_expecting("a list or a map", *this));
}

variant variant::get_keys() const
{
	must_be(VARIANT_TYPE::TYPE_MAP);

	std::vector<variant> tmp;
	for(const auto& i : value_cast<variant_map>()->get_container()) {
		tmp.push_back(i.first);
	}

	return variant(tmp);
}

variant variant::get_values() const
{
	must_be(VARIANT_TYPE::TYPE_MAP);

	std::vector<variant> tmp;
	for(const auto& i : value_cast<variant_map>()->get_container()) {
		tmp.push_back(i.second);
	}

	return variant(tmp);
}

variant_iterator variant::begin() const
{
	return value_->make_iterator().begin();
}

variant_iterator variant::end() const
{
	return value_->make_iterator().end();
}

bool variant::is_empty() const
{
	return value_->is_empty();
}

size_t variant::num_elements() const
{
	if(!is_list() && !is_map()) {
		throw type_error(was_expecting("a list or a map", *this));
	}

	return value_->num_elements();
}

variant variant::get_member(const std::string& name) const
{
	if(is_callable()) {
		return value_cast<variant_callable>()->get_callable()->query_value(name);
	}

	if(name == "self") {
		return *this;
	}

	return variant();
}

int variant::as_int() const
{
	if(is_null())    { return 0; }
	if(is_decimal()) { return as_decimal() / 1000; }

	must_be(VARIANT_TYPE::TYPE_INT);
	return value_cast<variant_int>()->get_numeric_value();
}

int variant::as_decimal() const
{
	if(is_decimal()) {
		return value_cast<variant_decimal>()->get_numeric_value();
	} else if(is_int()) {
		return value_cast<variant_int>()->get_numeric_value() * 1000;
	} else if(is_null()) {
		return 0;
	}

	throw type_error(was_expecting("an integer or a decimal", *this));
}

bool variant::as_bool() const
{
	return value_->as_bool();
}

const std::string& variant::as_string() const
{
	must_be(VARIANT_TYPE::TYPE_STRING);
	return value_cast<variant_string>()->get_string();
}

const std::vector<variant>& variant::as_list() const
{
	must_be(VARIANT_TYPE::TYPE_LIST);
	return value_cast<variant_list>()->get_container();
}

const std::map<variant, variant>& variant::as_map() const
{
	must_be(VARIANT_TYPE::TYPE_MAP);
	return value_cast<variant_map>()->get_container();
}

variant variant::operator+(const variant& v) const
{
	if(is_list() && v.is_list()) {
		auto& list = value_cast<variant_list>()->get_container();
		auto& other_list = v.value_cast<variant_list>()->get_container();

		std::vector<variant> res;
		res.reserve(list.size() + other_list.size());

		for(const auto& member : list) {
			res.push_back(member);
		}

		for(const auto& member : other_list) {
			res.push_back(member);
		}

		return variant(res);
	}

	if(is_map() && v.is_map()) {
		std::map<variant, variant> res = value_cast<variant_map>()->get_container();

		for(const auto& member : v.value_cast<variant_map>()->get_container()) {
			res[member.first] = member.second;
		}

		return variant(res);
	}

	if(is_decimal() || v.is_decimal()) {
		return variant(as_decimal() + v.as_decimal() , DECIMAL_VARIANT);
	}

	return variant(as_int() + v.as_int());
}

variant variant::operator-(const variant& v) const
{
	if(is_decimal() || v.is_decimal()) {
		return variant(as_decimal() - v.as_decimal() , DECIMAL_VARIANT);
	}

	return variant(as_int() - v.as_int());
}

variant variant::operator*(const variant& v) const
{
	if(is_decimal() || v.is_decimal()) {

		long long long_int = as_decimal();

		long_int *= v.as_decimal();

		long_int /= 100;

		if(long_int%10 >= 5) {
			long_int /= 10;
			++long_int;
		} else {
			long_int/=10;
		}

		return variant(static_cast<int>(long_int) , DECIMAL_VARIANT );
	}

	return variant(as_int() * v.as_int());
}

variant variant::operator/(const variant& v) const
{
	if(is_decimal() || v.is_decimal()) {
		int denominator = v.as_decimal();

		if(denominator == 0) {
			throw type_error("divide by zero error");
		}

		long long long_int = as_decimal();

		long_int *= 10000;

		long_int /= denominator;

		if(long_int%10 >= 5) {
			long_int /= 10;
			++long_int;
		} else {
			long_int/=10;
		}

		return variant(static_cast<int>(long_int), DECIMAL_VARIANT);
	}

	const int numerator = as_int();
	const int denominator = v.as_int();

	if(denominator == 0) {
		throw type_error("divide by zero error");
	}

	return variant(numerator / denominator);
}

variant variant::operator%(const variant& v) const
{
	if(is_decimal() || v.is_decimal()) {
		const int numerator = as_decimal();
		const int denominator = v.as_decimal();
		if(denominator == 0) {
			throw type_error("divide by zero error");
		}

		return variant(numerator % denominator, DECIMAL_VARIANT);
	} else {
		const int numerator = as_int();
		const int denominator = v.as_int();
		if(denominator == 0) {
			throw type_error("divide by zero error");
		}

		return variant(numerator % denominator);
	}
}

variant variant::operator^(const variant& v) const
{
	if(is_decimal() || v.is_decimal()) {

		double res = pow(as_decimal() / 1000.0 , v.as_decimal() / 1000.0);

		if(std::isnan(res)) {
			return variant();
		}

		return variant(res, DECIMAL_VARIANT);
	}

	return variant(static_cast<int>(round_portable(pow(static_cast<double>(as_int()), v.as_int()))));
}

variant variant::operator-() const
{
	if(is_decimal()) {
		return variant(-as_decimal(), DECIMAL_VARIANT);
	}

	return variant(-as_int());
}

bool variant::operator==(const variant& v) const
{
	if(type() != v.type()) {
		if(is_decimal() || v.is_decimal()) {
			return as_decimal() == v.as_decimal();
		}

		return false;
	}

	return value_->equals(*v.value_);
}

bool variant::operator!=(const variant& v) const
{
	return !operator==(v);
}

bool variant::operator<(const variant& v) const
{
	if(type() != v.type()) {
		if(is_decimal() && v.is_int()) {
			return as_decimal() < v.as_decimal();
		}

		if(v.is_decimal() && is_int()) {
			return as_decimal() < v.as_decimal();
		}

		return type() < v.type();
	}

	return value_->less_than(*v.value_);
}

bool variant::operator>=(const variant& v) const
{
	return !(*this < v);
}

bool variant::operator<=(const variant& v) const
{
	return !(v < *this);
}

bool variant::operator>(const variant& v) const
{
	return v < *this;
}

variant variant::list_elements_add(const variant& v) const
{
	must_both_be(VARIANT_TYPE::TYPE_LIST, v);
	return value_cast<variant_list>()->list_op(v.value_, std::plus<variant>());
}

variant variant::list_elements_sub(const variant& v) const
{
	must_both_be(VARIANT_TYPE::TYPE_LIST, v);
	return value_cast<variant_list>()->list_op(v.value_, std::minus<variant>());
}

variant variant::list_elements_mul(const variant& v) const
{
	must_both_be(VARIANT_TYPE::TYPE_LIST, v);
	return value_cast<variant_list>()->list_op(v.value_, std::multiplies<variant>());
}

variant variant::list_elements_div(const variant& v) const
{
	must_both_be(VARIANT_TYPE::TYPE_LIST, v);
	return value_cast<variant_list>()->list_op(v.value_, std::divides<variant>());
}

variant variant::concatenate(const variant& v) const
{
	if(is_list()) {
		v.must_be(VARIANT_TYPE::TYPE_LIST);

		std::vector<variant> res;
		res.reserve(num_elements() + v.num_elements());

		for(size_t i = 0; i < num_elements(); ++i) {
			res.push_back((*this)[i]);
		}

		for(size_t i = 0; i < v.num_elements(); ++i) {
			res.push_back(v[i]);
		}

		return variant(res);
	} else if(is_string()) {
		v.must_be(VARIANT_TYPE::TYPE_STRING);
		std::string res = as_string() + v.as_string();
		return variant(res);
	}

	throw type_error(was_expecting("a list or a string", *this));
}

variant variant::build_range(const variant& v) const
{
	must_both_be(VARIANT_TYPE::TYPE_INT, v);

	return value_cast<variant_int>()->build_range_variant(v.as_int());
}

bool variant::contains(const variant& v) const
{
	if(!is_list() && !is_map()) {
		throw type_error(was_expecting("a list or a map", *this));
	}

	if(is_list()) {
		return value_cast<variant_list>()->contains(v);
	} else {
		return value_cast<variant_map>()->contains(v);
	}
}

void variant::must_be(VARIANT_TYPE t) const
{
	if(type() != t) {
		throw type_error(was_expecting(variant_type_to_string(t), *this));
	}
}

void variant::must_both_be(VARIANT_TYPE t, const variant& second) const
{
	if(type() != t || second.type() != t) {
		throw type_error(formatter() << "TYPE ERROR: expected two "
			<< variant_type_to_string(t) << " but found "
			<<        type_string() << " (" <<        to_debug_string() << ")" << " and "
			<< second.type_string() << " (" << second.to_debug_string() << ")");
	}
}

std::string variant::serialize_to_string() const
{
	return value_->get_serialized_string();
}

void variant::serialize_from_string(const std::string& str)
{
	try {
		*this = formula(str).evaluate();
	} catch(...) {
		*this = variant(str);
	}
}

std::string variant::string_cast() const
{
	return value_->string_cast();
}

std::string variant::to_debug_string(bool verbose, formula_seen_stack* seen) const
{
	if(!seen) {
		formula_seen_stack seen_stack;
		return value_->get_debug_string(seen_stack, verbose);
	}

	return value_->get_debug_string(*seen, verbose);
}

variant variant::execute_variant(const variant& var)
{
	std::stack<variant> vars;
	if(var.is_list()) {
		for(size_t n = 1; n <= var.num_elements(); ++n) {
			vars.push(var[var.num_elements() - n]);
		}
	} else {
		vars.push(var);
	}

	std::vector<variant> made_moves;

	while(!vars.empty()) {

		if(vars.top().is_null()) {
			vars.pop();
			continue;
		}

		if(auto action = vars.top().try_convert<action_callable>()) {
			variant res = action->execute_self(*this);
			if(res.is_int() && res.as_bool()) {
				made_moves.push_back(vars.top());
			}
		} else if(vars.top().is_string() && vars.top().as_string() == "continue") {
//			if(infinite_loop_guardian_.continue_check()) {
				made_moves.push_back(vars.top());
//			} else {
				//too many calls in a row - possible infinite loop
//				ERR_SF << "ERROR #5001 while executing 'continue' formula keyword" << std::endl;

//				if(safe_call)
//					error = variant(new game_logic::safe_call_result(nullptr, 5001));
//			}
		} else if(vars.top().is_string() && (vars.top().as_string() == "end_turn" || vars.top().as_string() == "end")) {
			break;
		} else {
			//this information is unneeded when evaluating formulas from commandline
			ERR_SF << "UNRECOGNIZED MOVE: " << vars.top().to_debug_string() << std::endl;
		}

		vars.pop();
	}

	return variant(made_moves);
}

}
