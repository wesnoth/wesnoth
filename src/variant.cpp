/*
   Copyright (C) 2008 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include <cmath>
#include <iostream>
#include <string.h>

#include <boost/lexical_cast.hpp>

#include "formatter.hpp"
#include "formula_callable.hpp"
#include "formula_function.hpp"
#include "util.hpp"

namespace {
std::string variant_type_to_string(variant::TYPE type) {
	switch(type) {
	case variant::TYPE_NULL:
		return "null";
	case variant::TYPE_INT:
		return "int";
	case variant::TYPE_DECIMAL:
		return "decimal";
	case variant::TYPE_CALLABLE:
		return "object";
	case variant::TYPE_LIST:
		return "list";
	case variant::TYPE_STRING:
		return "string";
	case variant::TYPE_MAP:
		return "map";
	default:
		assert(false);
		return "invalid";
	}
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
	for(std::vector<const char*>::const_iterator i = call_stack.begin();
	    i != call_stack.end(); ++i) {
		if(!*i) {
			continue;
		}
		res += "  ";
		res += *i;
		res += "\n";
	}
	return res;
}

type_error::type_error(const std::string& str) : game::error(str) {
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

variant_iterator::variant_iterator(
		const std::vector<variant>::iterator& iter)
	: type_(TYPE_LIST)
	, list_iterator_(iter)
	, map_iterator_()
{
}

variant_iterator::variant_iterator(
		const std::map<variant, variant>::iterator& iter)
	: type_(TYPE_MAP)
	, list_iterator_()
	, map_iterator_(iter)
{
}

variant variant_iterator::operator*() const
{
	if (type_ == TYPE_LIST)
	{
		return variant( *list_iterator_);
	} else if (type_ == TYPE_MAP)
	{
		game_logic::key_value_pair* p = new game_logic::key_value_pair( map_iterator_->first, map_iterator_->second );
		variant  res( p );
		return res;
	} else
		return variant();
}

variant_iterator& variant_iterator::operator++()
{
	if (type_ == TYPE_LIST)
	{
		++list_iterator_;
	} else if (type_ == TYPE_MAP)
	{
		++map_iterator_;
	}

	return *this;
}

variant_iterator variant_iterator::operator++(int)
{
	variant_iterator iter(*this);
	if (type_ == TYPE_LIST)
	{
		++list_iterator_;
	} else if (type_ == TYPE_MAP)
	{
		++map_iterator_;
	}

	return iter;
}

variant_iterator& variant_iterator::operator=(const variant_iterator& that)
{
	if (this == &that)
		return *this;
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
	if (type_ == TYPE_LIST)
	{
		if (that.type_ != TYPE_LIST)
			return false;
		return list_iterator_ == that.list_iterator_;
	} else if (type_ == TYPE_MAP)
	{
		if (that.type_ != TYPE_MAP)
			return false;
		return map_iterator_ == that.map_iterator_;
	} else if (type_ == TYPE_NULL &&  that.type_ == TYPE_NULL )
		return true;
	else
		return false;
}

bool variant_iterator::operator!=(const variant_iterator& that) const
{
	if (type_ == TYPE_LIST)
	{
		if (that.type_ != TYPE_LIST)
			return true;
		return list_iterator_ != that.list_iterator_;
	} else if (type_ == TYPE_MAP)
	{
		if (that.type_ != TYPE_MAP)
			return true;
		return map_iterator_ != that.map_iterator_;
	} else if (type_ == TYPE_NULL &&  that.type_ == TYPE_NULL )
		return false;
	else
		return true;
}

struct variant_list {
	variant_list()
		: elements()
		, refcount(0)
	{
	}

	std::vector<variant> elements;
	int refcount;
};

struct variant_string {
	variant_string()
		: str()
		, refcount(0)
	{
	}

	std::string str;
	int refcount;
};

struct variant_map {
	variant_map()
		: elements()
		, refcount(0)
	{
	}

	std::map<variant,variant> elements;
	int refcount;
};

void variant::increment_refcount()
{
	switch(type_) {
	case TYPE_LIST:
		++list_->refcount;
		break;
	case TYPE_STRING:
		++string_->refcount;
		break;
	case TYPE_MAP:
		++map_->refcount;
		break;
	case TYPE_CALLABLE:
		intrusive_ptr_add_ref(callable_);
		break;

	// These are not used here, add them to silence a compiler warning.
	case TYPE_NULL:
	case TYPE_DECIMAL:
	case TYPE_INT :
		break;
	}
}

void variant::release()
{
	switch(type_) {
	case TYPE_LIST:
		if(--list_->refcount == 0) {
			delete list_;
		}
		break;
	case TYPE_STRING:
		if(--string_->refcount == 0) {
			delete string_;
		}
		break;
	case TYPE_MAP:
		if(--map_->refcount == 0) {
			delete map_;
		}
		break;
	case TYPE_CALLABLE:
		intrusive_ptr_release(callable_);
		break;

	// These are not used here, add them to silence a compiler warning.
	case TYPE_NULL:
	case TYPE_DECIMAL:
	case TYPE_INT :
		break;
	}
}

variant::variant() : type_(TYPE_NULL), int_value_(0)
{}

variant::variant(int n) : type_(TYPE_INT), int_value_(n)
{}

variant::variant(int n, variant::DECIMAL_VARIANT_TYPE /*type*/) : type_(TYPE_DECIMAL), decimal_value_(n)
{}

variant::variant(const game_logic::formula_callable* callable)
	: type_(TYPE_CALLABLE), callable_(callable)
{
	assert(callable_);
	increment_refcount();
}

variant::variant(std::vector<variant>* array)
    : type_(TYPE_LIST)
{
	assert(array);
	list_ = new variant_list;
	list_->elements.swap(*array);
	increment_refcount();
}

variant::variant(const std::string& str)
	: type_(TYPE_STRING)
{
	string_ = new variant_string;
	string_->str = str;
	increment_refcount();
}

variant::variant(std::map<variant,variant>* map)
    : type_(TYPE_MAP)
{
	assert(map);
	map_ = new variant_map;
	map_->elements.swap(*map);
	increment_refcount();
}

variant::variant(const variant& v)
    : type_(v.type_)
{
	memcpy(this, &v, sizeof(v));
	increment_refcount();
}

variant::~variant()
{
	release();
}

variant& variant::operator=(const variant& v)
{
	if(&v != this) {
		release();
		memcpy(this, &v, sizeof(v));
		increment_refcount();
	}
	return *this;
}

const variant& variant::operator[](size_t n) const
{
	if(type_ == TYPE_CALLABLE) {
		assert(n == 0);
		return *this;
	}

	must_be(TYPE_LIST);
	assert(list_);
	if(n >= list_->elements.size()) {
		throw type_error("invalid index");
	}

	return list_->elements[n];
}

const variant& variant::operator[](const variant& v) const
{
	if(type_ == TYPE_CALLABLE) {
		assert(v.as_int() == 0);
		return *this;
	}

	if(type_ == TYPE_MAP) {
		assert(map_);
		std::map<variant,variant>::const_iterator i = map_->elements.find(v);
		if (i == map_->elements.end())
		{
			static variant null_variant;
			return null_variant;
		}
		return i->second;
	} else if(type_ == TYPE_LIST) {
		return operator[](v.as_int());
	} else {
		throw type_error((formatter() << "type error: "
			<< " expected a list or a map but found "
			<< variant_type_to_string(type_)
			<< " (" << to_debug_string() << ")").str());
	}
}

variant variant::get_keys() const
{
	must_be(TYPE_MAP);
	assert(map_);
	std::vector<variant> tmp;
	for(std::map<variant,variant>::const_iterator i=map_->elements.begin(); i != map_->elements.end(); ++i) {
			tmp.push_back(i->first);
	}
	return variant(&tmp);
}

variant variant::get_values() const
{
	must_be(TYPE_MAP);
	assert(map_);
	std::vector<variant> tmp;
	for(std::map<variant,variant>::const_iterator i=map_->elements.begin(); i != map_->elements.end(); ++i) {
			tmp.push_back(i->second);
	}
	return variant(&tmp);
}

variant_iterator variant::begin() const
{
	if(type_ == TYPE_LIST)
		return variant_iterator( list_->elements.begin() );

	if(type_ == TYPE_MAP)
		return variant_iterator( map_->elements.begin() );

	return variant_iterator();
}
variant_iterator variant::end() const
{
	if(type_ == TYPE_LIST)
		return variant_iterator( list_->elements.end() );

	if(type_ == TYPE_MAP)
		return variant_iterator( map_->elements.end() );

	return variant_iterator();
}

bool variant::is_empty() const
{
	if(type_ == TYPE_NULL) {
		return true;
	} else if (type_ == TYPE_LIST) {
		assert(list_);
		return list_->elements.empty();
	} else if (type_ == TYPE_MAP) {
		assert(map_);
		return map_->elements.empty();
	}

	return false;
}

size_t variant::num_elements() const
{
	if(type_ == TYPE_CALLABLE) {
		return 1;
	}

	if (type_ == TYPE_LIST) {
		assert(list_);
		return list_->elements.size();
	} else if (type_ == TYPE_MAP) {
		assert(map_);
		return map_->elements.size();
	} else {
		throw type_error((formatter() << "type error: "
			<< " expected a list or a map but found "
			<< variant_type_to_string(type_)
			<< " (" << to_debug_string() << ")").str());
	}
}

variant variant::get_member(const std::string& str) const
{
	if(is_callable()) {
		return callable_->query_value(str);
	}

	if(str == "self") {
		return *this;
	} else {
		return variant();
	}
}

int variant::as_decimal() const
{
	if( type_ == TYPE_DECIMAL) {
		return decimal_value_;
	} else if( type_ == TYPE_INT ) {
		return int_value_*1000;
	} else if( type_ == TYPE_NULL) {
		return 0;
	} else {
		throw type_error((formatter() << "type error: "
			<< " expected integer or decimal but found "
			<< variant_type_to_string(type_)
			<< " (" << to_debug_string() << ")").str());
	}
}

bool variant::as_bool() const
{
	switch(type_) {
	case TYPE_NULL:
		return false;
	case TYPE_INT:
		return int_value_ != 0;
	case TYPE_DECIMAL:
		return decimal_value_ != 0;
	case TYPE_CALLABLE:
		return callable_ != NULL;
	case TYPE_LIST:
		return !list_->elements.empty();
	case TYPE_MAP:
		return !map_->elements.empty();
	case TYPE_STRING:
		return !string_->str.empty();
	default:
		assert(false);
		return false;
	}
}

const std::string& variant::as_string() const
{
	must_be(TYPE_STRING);
	assert(string_);
	return string_->str;
}

variant variant::operator+(const variant& v) const
{
	if(type_ == TYPE_LIST) {
		if(v.type_ == TYPE_LIST) {
			std::vector<variant> res;
			res.reserve(list_->elements.size() + v.list_->elements.size());
			for(size_t i = 0; i<list_->elements.size(); ++i) {
				const variant& var = list_->elements[i];
				res.push_back(var);
			}

			for(size_t j = 0; j<v.list_->elements.size(); ++j) {
				const variant& var = v.list_->elements[j];
				res.push_back(var);
			}

			return variant(&res);
		}
	}
	if(type_ == TYPE_MAP) {
		if(v.type_ == TYPE_MAP) {
			std::map<variant,variant> res(map_->elements);

			for(std::map<variant,variant>::const_iterator i = v.map_->elements.begin(); i != v.map_->elements.end(); ++i) {
				res[i->first] = i->second;
			}

			return variant(&res);
		}
	}
	if(type_ == TYPE_DECIMAL || v.type_ == TYPE_DECIMAL) {
		return variant( as_decimal() + v.as_decimal() , DECIMAL_VARIANT);
	}

	return variant(as_int() + v.as_int());
}

variant variant::operator-(const variant& v) const
{
	if(type_ == TYPE_DECIMAL || v.type_ == TYPE_DECIMAL) {
		return variant( as_decimal() - v.as_decimal() , DECIMAL_VARIANT);
	}

	return variant(as_int() - v.as_int());
}

variant variant::operator*(const variant& v) const
{
	if(type_ == TYPE_DECIMAL || v.type_ == TYPE_DECIMAL) {

		long long long_int = as_decimal();

		long_int *= v.as_decimal();

		long_int /= 100;

		if( long_int%10 >= 5) {
			long_int /= 10;
			++long_int;
		} else
			long_int/=10;

		return variant( static_cast<int>(long_int) , variant::DECIMAL_VARIANT );
	}

	return variant(as_int() * v.as_int());
}

variant variant::operator/(const variant& v) const
{
	if(type_ == TYPE_DECIMAL || v.type_ == TYPE_DECIMAL) {
		int denominator = v.as_decimal();

		if(denominator == 0) {
			throw type_error((formatter() << "divide by zero error").str());
		}

		long long long_int = as_decimal();

		long_int *= 10000;

		long_int /= denominator;

		if( long_int%10 >= 5) {
			long_int /= 10;
			++long_int;
		} else
			long_int/=10;

		return variant(  static_cast<int>(long_int) , variant::DECIMAL_VARIANT);
	}


	const int numerator = as_int();
	const int denominator = v.as_int();
	if(denominator == 0) {
		throw type_error((formatter() << "divide by zero error").str());;
	}

	return variant(numerator/denominator);
}

variant variant::operator%(const variant& v) const
{
	const int numerator = as_int();
	const int denominator = v.as_int();
	if(denominator == 0) {
		throw type_error((formatter() << "divide by zero error").str());
	}

	return variant(numerator%denominator);
}


variant variant::operator^(const variant& v) const
{
	if( type_ == TYPE_DECIMAL || v.type_ == TYPE_DECIMAL ) {

		double res = pow( as_decimal()/1000.0 , v.as_decimal()/1000.0 );

		res *= 1000;
		int i =  static_cast<int>( res );

		res -= i;

		if( res > 0.5 )
			i++;

		return variant( i , variant::DECIMAL_VARIANT);
	}

	return variant(static_cast<int>(
			round_portable(pow(static_cast<double>(as_int()), v.as_int()))));
}

variant variant::operator-() const
{
	if( type_ == TYPE_DECIMAL)
		return variant( -decimal_value_, variant::DECIMAL_VARIANT );

	return variant(-as_int());
}

bool variant::operator==(const variant& v) const
{
	if(type_ != v.type_) {
		if( type_ == TYPE_DECIMAL || v.type_ == TYPE_DECIMAL ) {
			return as_decimal() == v.as_decimal();
		}

		return false;
	}

	switch(type_) {
	case TYPE_NULL: {
		return v.is_null();
	}

	case TYPE_STRING: {
		return string_->str == v.string_->str;
	}

	case TYPE_INT: {
		return int_value_ == v.int_value_;
	}

	case TYPE_DECIMAL: {
		return decimal_value_ == v.decimal_value_;
	}

	case TYPE_LIST: {
		if(num_elements() != v.num_elements()) {
			return false;
		}

		for(size_t n = 0; n != num_elements(); ++n) {
			if((*this)[n] != v[n]) {
				return false;
			}
		}

		return true;
	}

	case TYPE_MAP: {
		return map_->elements == v.map_->elements;
	}

	case TYPE_CALLABLE: {
		return callable_->equals(v.callable_);
	}
	}

	assert(false);
	return false;
}

bool variant::operator!=(const variant& v) const
{
	return !operator==(v);
}

bool variant::operator<=(const variant& v) const
{
	if(type_ != v.type_) {
		if( type_ == TYPE_DECIMAL || v.type_ == TYPE_DECIMAL ) {
			return as_decimal() <= v.as_decimal();
		}

		return type_ < v.type_;
	}

	switch(type_) {
	case TYPE_NULL: {
		return true;
	}

	case TYPE_STRING: {
		return string_->str <= v.string_->str;
	}

	case TYPE_INT: {
		return int_value_ <= v.int_value_;
	}

	case TYPE_DECIMAL: {
		return decimal_value_ <= v.decimal_value_;
	}

	case TYPE_LIST: {
		for(size_t n = 0; n != num_elements() && n != v.num_elements(); ++n) {
			if((*this)[n] < v[n]) {
				return true;
			} else if((*this)[n] > v[n]) {
				return false;
			}
		}

		return num_elements() <= v.num_elements();
	}

	case TYPE_MAP: {
		return map_->elements <= v.map_->elements;
	}

	case TYPE_CALLABLE: {
		return !v.callable_->less(callable_);
	}
	}

	assert(false);
	return false;
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
	must_be(TYPE_LIST);
	v.must_be(TYPE_LIST);

	if( num_elements() != v.num_elements() )
		throw type_error("Operator '.+' requires two lists of the same length");

	std::vector< variant > res;
	res.reserve(num_elements());

	for(size_t i = 0; i < num_elements(); ++i) {
		res.push_back( (*this)[i] + v[i] );
	}

	return variant( &res );
}

variant variant::list_elements_sub(const variant& v) const
{
	must_be(TYPE_LIST);
	v.must_be(TYPE_LIST);

	if( num_elements() != v.num_elements() )
		throw type_error("Operator '.-' requires two lists of the same length");

	std::vector< variant > res;
	res.reserve(num_elements());

	for(size_t i = 0; i < num_elements(); ++i) {
		res.push_back( (*this)[i] - v[i] );
	}

	return variant( &res );
}

variant variant::list_elements_mul(const variant& v) const
{
	must_be(TYPE_LIST);
	v.must_be(TYPE_LIST);

	if( num_elements() != v.num_elements() )
		throw type_error("Operator '.*' requires two lists of the same length");

	std::vector< variant > res;
	res.reserve(num_elements());

	for(size_t i = 0; i < num_elements(); ++i) {
		res.push_back( (*this)[i] * v[i] );
	}

	return variant( &res );
}

variant variant::list_elements_div(const variant& v) const
{
	must_be(TYPE_LIST);
	v.must_be(TYPE_LIST);

	if( num_elements() != v.num_elements() )
		throw type_error("Operator './' requires two lists of the same length");

	std::vector< variant > res;
	res.reserve(num_elements());

	for(size_t i = 0; i < num_elements(); ++i) {
		res.push_back( (*this)[i] / v[i] );
	}

	return variant( &res );
}

void variant::must_be(variant::TYPE t) const
{
	if(type_ != t) {
		throw type_error((formatter() << "type error: " << " expected "
			<< variant_type_to_string(t) << " but found "
			<< variant_type_to_string(type_)
			<< " (" << to_debug_string() << ")").str());
	}
}

void variant::serialize_to_string(std::string& str) const
{
	switch(type_) {
	case TYPE_NULL:
		str += "null()";
		break;
	case TYPE_INT:
		str += boost::lexical_cast<std::string>(int_value_);
		break;
	case TYPE_DECIMAL: {
		std::ostringstream s;

		int fractional = decimal_value_ % 1000;
		int integer = (decimal_value_ - fractional) / 1000;

		s << integer << ".";

		fractional = abs(fractional);

		if( fractional < 100) {
			if( fractional < 10)
				s << "00";
			else
				s << 0;
		}

		s << fractional;

		str += s.str();
		break;
	}
	case TYPE_CALLABLE:
		callable_->serialize(str);
		break;
	case TYPE_LIST: {
		str += "[";
		bool first_time = true;
		for(size_t i=0; i<list_->elements.size(); ++i) {
			const variant& var = list_->elements[i];
			if(!first_time) {
				str += ",";
			}
			first_time = false;
			var.serialize_to_string(str);
		}
		str += "]";
		break;
	}
	case TYPE_MAP: {
		str += "[";
		bool first_time = true;
		for(std::map<variant,variant>::const_iterator i=map_->elements.begin(); i != map_->elements.end(); ++i) {
			if(!first_time) {
				str += ",";
			}
			first_time = false;
			i->first.serialize_to_string(str);
			str += "->";
			i->second.serialize_to_string(str);
		}
		str += "]";
		break;
	}
	case TYPE_STRING:
		str += "'";
		str += string_->str;
		str += "'";
		break;
	default:
		assert(false);
	}
}

void variant::serialize_from_string(const std::string& str)
{
	try {
		*this = game_logic::formula(str).evaluate();
	} catch(...) {
		*this = variant(str);
	}
}

int variant::refcount() const
{
	switch(type_) {
	case TYPE_LIST:
		return list_->refcount;
		break;
	case TYPE_STRING:
		return string_->refcount;
		break;
	case TYPE_MAP:
		return map_->refcount;
		break;
	case TYPE_CALLABLE:
		return callable_->refcount();
		break;
	default:
		return -1;
	}
}

std::string variant::string_cast() const
{
	switch(type_) {
	case TYPE_NULL:
		return "0";
	case TYPE_INT:
		return boost::lexical_cast<std::string>(int_value_);
	case TYPE_DECIMAL: {
		std::ostringstream s;

		int fractional = decimal_value_ % 1000;
		int integer = (decimal_value_ - fractional) / 1000;

		s << integer << ".";

		fractional = abs(fractional);

		if( fractional < 100) {
			if( fractional < 10)
				s << "00";
			else
				s << 0;
		}

		s << fractional;

		return s.str();
	}
	case TYPE_CALLABLE:
		return "(object)";
	case TYPE_LIST: {
		std::string res = "";
		for(size_t i=0; i<list_->elements.size(); ++i) {
			const variant& var = list_->elements[i];
			if(!res.empty()) {
				res += ", ";
			}

			res += var.string_cast();
		}

		return res;
	}
	case TYPE_MAP: {
		std::string res = "";
		for(std::map<variant,variant>::const_iterator i=map_->elements.begin(); i != map_->elements.end(); ++i) {
			if(!res.empty()) {
				res += ",";
			}
			res += i->first.string_cast();
			res += "->";
			res += i->second.string_cast();
		}
		return res;
	}

	case TYPE_STRING:
		return string_->str;
	default:
		assert(false);
		return "invalid";
	}
}

std::string variant::to_debug_string(std::vector<const game_logic::formula_callable*>* seen, bool verbose) const
{
	std::vector<const game_logic::formula_callable*> seen_stack;
	if(!seen) {
		seen = &seen_stack;
	}

	std::ostringstream s;
	switch(type_) {
	case TYPE_NULL:
		s << "(null)";
		break;
	case TYPE_INT:
		s << int_value_;
		break;
	case TYPE_DECIMAL: {
		int fractional = decimal_value_ % 1000;
		int integer = (decimal_value_ - fractional) / 1000;

		// Make sure we get the sign on small negative values.
		if ( integer == 0  &&  decimal_value_ < 0 )
			s << '-';
		s << integer << ".";

		fractional = abs(fractional);

		if( fractional < 100) {
			if( fractional < 10)
				s << "00";
			else
				s << 0;
		}

		s << fractional;

		break;
	}
	case TYPE_LIST: {
		s << "[";
		for(size_t n = 0; n != num_elements(); ++n) {
			if(n != 0) {
				s << ", ";
			}

			s << operator[](n).to_debug_string(seen, verbose);
		}
		s << "]";
		break;
	}
	case TYPE_CALLABLE: {
		s << "{";
		if(std::find(seen->begin(), seen->end(), callable_) == seen->end()) {
			if(!verbose)
				seen->push_back(callable_);
			std::vector<game_logic::formula_input> v = callable_->inputs();
			bool first = true;
			for(size_t i=0; i<v.size(); ++i) {
				const game_logic::formula_input& input = v[i];
				if(!first) {
					s << ", ";
				}
				first = false;
				s << input.name << " ";
				if(input.access == game_logic::FORMULA_READ_WRITE) {
					s << "(read-write) ";
				} else if(input.access == game_logic::FORMULA_WRITE_ONLY) {
					s << "(writeonly) ";
				}

				s << "-> " << callable_->query_value(input.name).to_debug_string(seen, verbose);
			}
		} else {
			s << "...";
		}
		s << "}";
		break;
	}
	case TYPE_MAP: {
		s << "[";
		bool first_time = true;
		for(std::map<variant,variant>::const_iterator i=map_->elements.begin(); i != map_->elements.end(); ++i) {
			if(!first_time) {
				s << ",";
			}
			first_time = false;
			s << i->first.to_debug_string(seen, verbose);
			s << "->";
			s << i->second.to_debug_string(seen, verbose);
		}
		s << "]";
		break;
	}
	case TYPE_STRING: {
		s << "'" << string_->str << "'";
		break;
	}
	}

	return s.str();
}
