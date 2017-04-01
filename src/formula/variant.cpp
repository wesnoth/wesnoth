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

#include "formatter.hpp"
#include "formula/function.hpp"
#include "utils/math.hpp"

#include <cassert>
#include <cmath>
#include <memory>

namespace {

// Static value to initialize null variants to ensure its value is never nullptr.
game_logic::value_base_ptr null_value(new game_logic::variant_value_base);

std::string variant_type_to_string(VARIANT_TYPE type)
{
	return VARIANT_TYPE::enum_to_string(type);
}

std::vector<const char*> call_stack;

}

void push_call_stack(const char* str)
{
	call_stack.push_back(str);
}

void pop_call_stack()
{
	call_stack.pop_back();
}

std::string get_call_stack()
{
	std::string res;
	for(const auto& i : call_stack) {
		if(!i) {
			continue;
		}

		res += "  ";
		res += i;
		res += "\n";
	}

	return res;
}

type_error::type_error(const std::string& str) : game::error(str)
{
	std::cerr << "ERROR: " << message << "\n" << get_call_stack();
}

variant_iterator::variant_iterator()
	: type_(TYPE_NULL)
	, list_iterator_()
	, map_iterator_()
{
}

variant_iterator::variant_iterator(const variant_iterator& iter)
	: type_(iter.type_)
	, list_iterator_()
	, map_iterator_()
{
	switch(type_) {
		case TYPE_LIST :
			list_iterator_ = iter.list_iterator_;
			break;

		case TYPE_MAP:
			map_iterator_ = iter.map_iterator_;
			break;

		case TYPE_NULL:
			/* DO NOTHING */
			break;
	}
}

variant_iterator::variant_iterator(const std::vector<variant>::iterator& iter)
	: type_(TYPE_LIST)
	, list_iterator_(iter)
	, map_iterator_()
{
}

variant_iterator::variant_iterator(const std::map<variant, variant>::iterator& iter)
	: type_(TYPE_MAP)
	, list_iterator_()
	, map_iterator_(iter)
{
}

variant variant_iterator::operator*() const
{
	if(type_ == TYPE_LIST) {
		return *list_iterator_;
	} else if(type_ == TYPE_MAP) {
		game_logic::key_value_pair* p = new game_logic::key_value_pair(map_iterator_->first, map_iterator_->second);
		variant res(p);
		return res;
	}

	return variant();
}

variant_iterator& variant_iterator::operator++()
{
	if(type_ == TYPE_LIST) {
		++list_iterator_;
	} else if(type_ == TYPE_MAP) {
		++map_iterator_;
	}

	return *this;
}

variant_iterator variant_iterator::operator++(int)
{
	variant_iterator iter(*this);
	if(type_ == TYPE_LIST) {
		++list_iterator_;
	} else if(type_ == TYPE_MAP) {
		++map_iterator_;
	}

	return iter;
}

variant_iterator& variant_iterator::operator--()
{
	if(type_ == TYPE_LIST) {
		--list_iterator_;
	} else if(type_ == TYPE_MAP) {
		--map_iterator_;
	}

	return *this;
}

variant_iterator variant_iterator::operator--(int)
{
	variant_iterator iter(*this);
	if(type_ == TYPE_LIST) {
		--list_iterator_;
	} else if(type_ == TYPE_MAP) {
		--map_iterator_;
	}

	return iter;
}

variant_iterator& variant_iterator::operator=(const variant_iterator& that)
{
	if(this == &that) {
		return *this;
	}

	type_ = that.type_;
	switch(type_) {
		case TYPE_LIST :
			list_iterator_ = that.list_iterator_;
			break;

		case TYPE_MAP:
			map_iterator_ = that.map_iterator_;
			break;

		case TYPE_NULL:
			/* DO NOTHING */
			break;
	}

	return *this;
}

bool variant_iterator::operator==(const variant_iterator& that) const
{
	if(type_ == TYPE_LIST) {
		return that.type_ != TYPE_LIST ? false : list_iterator_ == that.list_iterator_;
	} else if(type_ == TYPE_MAP) {
		return that.type_ != TYPE_MAP  ? false : map_iterator_ == that.map_iterator_;
	} else if(type_== TYPE_NULL && that.type_ == TYPE_NULL) {
		return true;
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
	: value_(std::make_shared<game_logic::variant_int>(n))
{
	assert(value_.get());
}

variant::variant(int n, variant::DECIMAL_VARIANT_TYPE)
	: value_(std::make_shared<game_logic::variant_decimal>(n))
{
	assert(value_.get());
}

variant::variant(double n, variant::DECIMAL_VARIANT_TYPE)
	: value_(std::make_shared<game_logic::variant_decimal>(n))
{
	assert(value_.get());
}

variant::variant(const game_logic::formula_callable* callable)
	: value_(std::make_shared<game_logic::variant_callable>(callable))
{
	assert(value_.get());
}

variant::variant(const std::vector<variant>& vec)
    : value_((std::make_shared<game_logic::variant_list>(vec)))
{
	assert(value_.get());
}

variant::variant(const std::string& str)
	: value_(std::make_shared<game_logic::variant_string>(str))
{
	assert(value_.get());
}

variant::variant(const std::map<variant,variant>& map)
	: value_((std::make_shared<game_logic::variant_map>(map)))
{
	assert(value_.get());
}

variant::variant(const variant& v)
    : value_(v.value_)
{
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
		return value_cast<game_logic::variant_list>()->get_container()[n];
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
		auto& map = value_cast<game_logic::variant_map>()->get_container();

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
	} else {
		throw type_error(formatter() << "type error: "
			<< " expected a list or a map but found " << type_string()
			<< " (" << to_debug_string() << ")");
	}
}

variant variant::get_keys() const
{
	must_be(VARIANT_TYPE::TYPE_MAP);

	std::vector<variant> tmp;
	for(const auto& i : value_cast<game_logic::variant_map>()->get_container()) {
		tmp.push_back(i.first);
	}

	return variant(tmp);
}

variant variant::get_values() const
{
	must_be(VARIANT_TYPE::TYPE_MAP);

	std::vector<variant> tmp;
	for(const auto& i : value_cast<game_logic::variant_map>()->get_container()) {
		tmp.push_back(i.second);
	}

	return variant(tmp);
}

variant_iterator variant::begin() const
{
	if(is_list()) {
		return variant_iterator(value_cast<game_logic::variant_list>()->get_container().begin());
	}

	if(is_map()) {
		return variant_iterator(value_cast<game_logic::variant_map>()->get_container().begin());
	}

	return variant_iterator();
}

variant_iterator variant::end() const
{
	if(is_list()) {
		return variant_iterator(value_cast<game_logic::variant_list>()->get_container().end());
	}

	if(is_map()) {
		return variant_iterator(value_cast<game_logic::variant_map>()->get_container().end());
	}

	return variant_iterator();
}

bool variant::is_empty() const
{
	return value_->is_empty();
}

size_t variant::num_elements() const
{
	return value_->num_elements();
}

variant variant::get_member(const std::string& name) const
{
	if(is_callable()) {
		return value_cast<game_logic::variant_callable>()->get_callable()->query_value(name);
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
	return value_cast<game_logic::variant_int>()->get_integer();
}

int variant::as_decimal() const
{
	if(is_decimal()) {
		return value_cast<game_logic::variant_decimal>()->get_decimal();
	} else if(is_int()) {
		return value_cast<game_logic::variant_int>()->get_integer() * 1000;
	} else if(is_null()) {
		return 0;
	} else {
		throw type_error(formatter() << "type error: "
			<< " expected integer or decimal but found " << type_string()
			<< " (" << to_debug_string() << ")");
	}
}

bool variant::as_bool() const
{
	return value_->as_bool();
}

const std::string& variant::as_string() const
{
	must_be(VARIANT_TYPE::TYPE_STRING);
	return value_cast<game_logic::variant_string>()->get_string();
}

const std::vector<variant>& variant::as_list() const
{
	must_be(VARIANT_TYPE::TYPE_LIST);
	return value_cast<game_logic::variant_list>()->get_container();
}

const std::map<variant, variant>& variant::as_map() const
{
	must_be(VARIANT_TYPE::TYPE_MAP);
	return value_cast<game_logic::variant_map>()->get_container();
}

variant variant::operator+(const variant& v) const
{
	if(is_list() && v.is_list()) {
		auto& list = value_cast<game_logic::variant_list>()->get_container();
		auto& other_list = v.value_cast<game_logic::variant_list>()->get_container();

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
		std::map<variant, variant> res = value_cast<game_logic::variant_map>()->get_container();

		for(const auto& member : v.value_cast<game_logic::variant_map>()->get_container()) {
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

		return variant(static_cast<int>(long_int) , variant::DECIMAL_VARIANT );
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

		return variant(static_cast<int>(long_int), variant::DECIMAL_VARIANT);
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
		return variant(-as_decimal(), variant::DECIMAL_VARIANT);
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

	return *value_ == *v.value_;
}

bool variant::operator!=(const variant& v) const
{
	return !operator==(v);
}

bool variant::operator<=(const variant& v) const
{
	if(type() != v.type()) {
		if(is_decimal() && v.is_int()) {
			return as_decimal() <= v.as_decimal();
		}

		if(v.is_decimal() && is_int()) {
			return as_decimal() <= v.as_decimal();
		}

		return type() < v.type();
	}

	return *value_ <= *v.value_;
}

bool variant::operator>=(const variant& v) const
{
	return v <= *this;
}

bool variant::operator<(const variant& v) const
{
	return !(*this >= v);
}

bool variant::operator>(const variant& v) const
{
	return !(*this <= v);
}

variant variant::list_elements_add(const variant& v) const
{
	must_both_be(VARIANT_TYPE::TYPE_LIST, v);
	return value_cast<game_logic::variant_list>()->list_op(v.value_, [this](variant& v1, variant& v2) { return v1 + v2; });
}

variant variant::list_elements_sub(const variant& v) const
{
	must_both_be(VARIANT_TYPE::TYPE_LIST, v);
	return value_cast<game_logic::variant_list>()->list_op(v.value_, [this](variant& v1, variant& v2) { return v1 - v2; });
}

variant variant::list_elements_mul(const variant& v) const
{
	must_both_be(VARIANT_TYPE::TYPE_LIST, v);
	return value_cast<game_logic::variant_list>()->list_op(v.value_, [this](variant& v1, variant& v2) { return v1 * v2; });
}

variant variant::list_elements_div(const variant& v) const
{
	must_both_be(VARIANT_TYPE::TYPE_LIST, v);
	return value_cast<game_logic::variant_list>()->list_op(v.value_, [this](variant& v1, variant& v2) { return v1 / v2; });
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
	} else {
		throw type_error(formatter() << "type error: expected two "
			<< " lists or two maps  but found " << type_string()
			<< " (" << to_debug_string() << ")"
			<< " and " << v.type_string()
			<< " (" << v.to_debug_string() << ")");
	}
}

variant variant::build_range(const variant& v) const
{
	must_both_be(VARIANT_TYPE::TYPE_INT, v);

	return value_cast<game_logic::variant_int>()->build_range_variant(v.as_int());
}

bool variant::contains(const variant& v) const
{
	if(type() != VARIANT_TYPE::TYPE_LIST && type() != VARIANT_TYPE::TYPE_MAP) {
		throw type_error(formatter() << "type error: expected "
			<< variant_type_to_string(VARIANT_TYPE::TYPE_LIST) << " or "
			<< variant_type_to_string(VARIANT_TYPE::TYPE_MAP) << " but found "
			<< type_string()
			<< " (" << to_debug_string() << ")");
	}

	if(type() == VARIANT_TYPE::TYPE_LIST) {
		return value_cast<game_logic::variant_list>()->contains(v);
	} else {
		return value_cast<game_logic::variant_map>()->contains(v);
	}
}

void variant::must_be(VARIANT_TYPE t) const
{
	if(type() != t) {
		throw type_error(formatter() << "type error: expected "
			<< variant_type_to_string(t) << " but found "
			<< type_string() << " (" << to_debug_string() << ")");
	}
}

void variant::must_both_be(VARIANT_TYPE t, const variant& second) const
{
	if(type() != t || second.type() != t) {
		throw type_error(formatter() << "type error: expected "
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
		*this = game_logic::formula(str).evaluate();
	} catch(...) {
		*this = variant(str);
	}
}

std::string variant::string_cast() const
{
	return value_->string_cast();
}

std::string variant::to_debug_string(bool verbose, bool clear_stack) const
{
	if(clear_stack) {
		game_logic::variant_callable::seen_stack.clear();
	}

	return value_->get_debug_string(verbose);
}
