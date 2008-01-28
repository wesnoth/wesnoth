#include <cmath>
#include <set>
#include <stdlib.h>
#include <vector>

#include <iostream>

#include "boost/lexical_cast.hpp"

#include "foreach.hpp"
#include "formatter.hpp"
#include "formula.hpp"
#include "formula_callable.hpp"
#include "variant.hpp"

namespace {
std::string variant_type_to_string(variant::TYPE type) {
	switch(type) {
	case variant::TYPE_NULL: return "null";
	case variant::TYPE_INT: return "int";
	case variant::TYPE_CALLABLE: return "object";
	case variant::TYPE_LIST: return "list";
	case variant::TYPE_STRING: return "string";
	default:
		assert(false);
	}
}
}

struct variant_list {
	variant_list() : refcount(0)
	{}
	std::vector<variant> elements;
	int refcount;
};

struct variant_string {
	variant_string() : refcount(0)
	{}
	std::string str;
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
	case TYPE_CALLABLE:
		intrusive_ptr_add_ref(callable_);
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
	case TYPE_CALLABLE:
		intrusive_ptr_release(callable_);
		break;
	}
}

variant::variant() : type_(TYPE_NULL), int_value_(0)
{}

variant::variant(int n) : type_(TYPE_INT), int_value_(n)
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
}

variant::variant(const std::string& str)
	: type_(TYPE_STRING)
{
	string_ = new variant_string;
	string_->str = str;
}

variant::variant(const variant& v)
{
	memcpy(this, &v, sizeof(v));
	increment_refcount();
}

const variant& variant::operator=(const variant& v)
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

size_t variant::num_elements() const
{
	if(type_ == TYPE_CALLABLE) {
		return 1;
	}

	must_be(TYPE_LIST);
	assert(list_);
	return list_->elements.size();
}

bool variant::as_bool() const
{
	switch(type_) {
	case TYPE_NULL:
		return false;
	case TYPE_INT:
		return int_value_ != 0;
	case TYPE_CALLABLE:
		return callable_ != NULL;
	case TYPE_LIST:
		return !list_->elements.empty();
	case TYPE_STRING:
		return !string_->str.empty();
	default:
		assert(false);
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
			foreach(const variant& var, list_->elements) {
				res.push_back(var);
			}

			foreach(const variant& var, v.list_->elements) {
				res.push_back(var);
			}

			return variant(&res);
		}
	}

	return variant(as_int() + v.as_int());
}

variant variant::operator-(const variant& v) const
{
	return variant(as_int() - v.as_int());
}

variant variant::operator*(const variant& v) const
{
	return variant(as_int() * v.as_int());
}

variant variant::operator/(const variant& v) const
{
	const int numerator = as_int();
	const int denominator = v.as_int();
	if(denominator == 0) {
		throw type_error(formatter() << "divide by zero error");
	}

	return variant(numerator/denominator);
}

variant variant::operator%(const variant& v) const
{
	const int numerator = as_int();
	const int denominator = v.as_int();
	if(denominator == 0) {
		throw type_error(formatter() << "divide by zero error");
	}

	return variant(numerator%denominator);
}

variant variant::operator^(const variant& v) const
{
	return variant(static_cast<int>(pow(as_int(), v.as_int())));
}

variant variant::operator-() const
{
	return variant(-as_int());
}

bool variant::operator==(const variant& v) const
{
	if(type_ != v.type_) {
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

	case TYPE_LIST: {
		if(num_elements() != v.num_elements()) {
			return false;
		}

		for(int n = 0; n != num_elements(); ++n) {
			if((*this)[n] != v[n]) {
				return false;
			}
		}

		return true;
	}

	case TYPE_CALLABLE: {
		return callable_->equals(v.callable_);
	}
	}

	assert(false);
}

bool variant::operator!=(const variant& v) const
{
	return !operator==(v);
}

bool variant::operator<=(const variant& v) const
{
	if(type_ != v.type_) {
		return false;
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

	case TYPE_LIST: {
		if(num_elements() != v.num_elements()) {
			return false;
		}

		for(int n = 0; n != num_elements() && n != v.num_elements(); ++n) {
			if((*this)[n] < v[n]) {
				return true;
			} else if((*this)[n] > v[n]) {
				return false;
			}
		}

		return num_elements() <= v.num_elements();
	}

	case TYPE_CALLABLE: {
		return !v.callable_->less(callable_);
	}
	}

	assert(false);
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

void variant::must_be(variant::TYPE t) const
{
	if(type_ != t) {
		throw type_error(formatter() << "type error: " << " expected " << variant_type_to_string(t) << " but found " << variant_type_to_string(type_) << " (" << to_debug_string() << ")");
	}
}

void variant::serialize_to_string(std::string& str) const
{
	switch(type_) {
	case TYPE_NULL:
		str += "null()";
	case TYPE_INT:
		str += boost::lexical_cast<std::string>(int_value_);
		break;
	case TYPE_CALLABLE:
		std::cerr << "ERROR: attempt to serialize callable variant\n";
		break;
	case TYPE_LIST: {
		str += "[";
		bool first_time = true;
		foreach(const variant& var, list_->elements) {
			if(!first_time) {
				str += ",";
			}
			first_time = false;
			var.serialize_to_string(str);
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
	*this = game_logic::formula(str).execute();
}

std::string variant::string_cast() const
{
	switch(type_) {
	case TYPE_NULL:
		return "0";
	case TYPE_INT:
		return boost::lexical_cast<std::string>(int_value_);
	case TYPE_CALLABLE:
		return "(object)";
	case TYPE_LIST: {
		std::string res = "";
		foreach(const variant& var, list_->elements) {
			if(!res.empty()) {
				res += ", ";
			}

			res += var.string_cast();
		}

		return res;
	}

	case TYPE_STRING:
		return string_->str;
	default:
		assert(false);
	}
}

std::string variant::to_debug_string(std::vector<const game_logic::formula_callable*>* seen) const
{
	std::vector<const game_logic::formula_callable*> seen_stack;
	if(!seen) {
		seen = &seen_stack;
	}

	std::ostringstream s;
	switch(type_) {
	case TYPE_NULL:
		s << "(null)";
	case TYPE_INT:
		s << int_value_;
		break;
	case TYPE_LIST: {
		s << "[";
		for(int n = 0; n != num_elements(); ++n) {
			if(n != 0) {
				s << ", ";
			}

			s << operator[](n).to_debug_string(seen);
		}
		s << "]";
		break;
	}
	case TYPE_CALLABLE: {
		s << "{";
		if(std::find(seen->begin(), seen->end(), callable_) == seen->end()) {
			seen->push_back(callable_);
			std::vector<game_logic::formula_input> v = callable_->inputs();
			bool first = true;
			foreach(const game_logic::formula_input& input, v) {
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

				s << "-> " << callable_->query_value(input.name).to_debug_string(seen);
			}
		} else {
			s << "...";
		}
		s << "}";
		break;
	}
	case TYPE_STRING: {
		s << "'" << string_->str << "'";
		break;
	}
	}

	return s.str();
}
