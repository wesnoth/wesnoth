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

#ifndef VARIANT_HPP_INCLUDED
#define VARIANT_HPP_INCLUDED

#include <map>
#include <vector>

#include "exceptions.hpp"

namespace game_logic {
class formula_callable;
}

void push_call_stack(const char* str);
void pop_call_stack();
std::string get_call_stack();

struct call_stack_manager {
	explicit call_stack_manager(const char* str) {
		push_call_stack(str);
	}

	~call_stack_manager() {
		pop_call_stack();
	}
};

class variant_iterator;

struct type_error : public game::error {
	explicit type_error(const std::string& str);
};


class variant {
public:

	enum TYPE { TYPE_NULL, TYPE_INT, TYPE_DECIMAL, TYPE_CALLABLE, TYPE_LIST, TYPE_STRING, TYPE_MAP };

	enum DECIMAL_VARIANT_TYPE { DECIMAL_VARIANT };

	variant();
	explicit variant(int n);
	variant(int n, DECIMAL_VARIANT_TYPE /*type*/);
	variant(double n, DECIMAL_VARIANT_TYPE /*type*/);
	explicit variant(const game_logic::formula_callable* callable);
	explicit variant(std::vector<variant>* array);
	explicit variant(const std::string& str);
	explicit variant(std::map<variant,variant>* map);
	~variant();

	variant(const variant& v);
	variant& operator=(const variant& v);

	variant operator[](size_t n) const;
	variant operator[](const variant& v) const;
	size_t num_elements() const;
	bool is_empty() const;

	variant get_member(const std::string& str) const;

	bool is_string() const { return type_ == TYPE_STRING; }
	bool is_null() const { return type_ == TYPE_NULL; }
	bool is_int() const { return type_ == TYPE_INT; }
	bool is_decimal() const { return type_ == TYPE_DECIMAL; }
	bool is_map() const { return type_ == TYPE_MAP; }
	int as_int() const;

	//this function returns variant's internal representation of decimal number:
	//for example number 1.234 is represented as 1234
	int as_decimal() const;

	bool as_bool() const;

	bool is_list() const { return type_ == TYPE_LIST; }
	
	const std::vector<variant>& as_list() const;
	const std::map<variant,variant>& as_map() const;

	const std::string& as_string() const;
	std::string type_string() const;

	bool is_callable() const { return type_ == TYPE_CALLABLE; }
	const game_logic::formula_callable* as_callable() const {
		must_be(TYPE_CALLABLE); return callable_; }
	game_logic::formula_callable* mutable_callable() const {
		must_be(TYPE_CALLABLE); return mutable_callable_; }

	template<typename T>
	T* try_convert() const {
		if(!is_callable()) {
			return nullptr;
		}

		return dynamic_cast<T*>(mutable_callable());
	}

	template<typename T>
	T* convert_to() const {
		T* res = dynamic_cast<T*>(mutable_callable());
		if(!res) {
			throw type_error("could not convert type");
		}

		return res;
	}

	variant operator+(const variant&) const;
	variant operator-(const variant&) const;
	variant operator*(const variant&) const;
	variant operator/(const variant&) const;
	variant operator^(const variant&) const;
	variant operator%(const variant&) const;
	variant operator-() const;

	bool operator==(const variant&) const;
	bool operator!=(const variant&) const;
	bool operator<(const variant&) const;
	bool operator>(const variant&) const;
	bool operator<=(const variant&) const;
	bool operator>=(const variant&) const;

	variant list_elements_add(const variant& v) const;
	variant list_elements_sub(const variant& v) const;
	variant list_elements_mul(const variant& v) const;
	variant list_elements_div(const variant& v) const;
	variant concatenate(const variant& v) const;
	variant build_range(const variant& v) const;
	bool contains(const variant& other) const;

	variant get_keys() const;
	variant get_values() const;

	variant_iterator begin() const;
	variant_iterator end() const;

	void serialize_to_string(std::string& str) const;
	void serialize_from_string(const std::string& str);

	std::string string_cast() const;

	std::string to_debug_string(std::vector<const game_logic::formula_callable*>* seen=nullptr, bool verbose = false) const;

private:
	void must_be(TYPE t) const;
	TYPE type_;
	union {
		int int_value_;
		int decimal_value_;
		const game_logic::formula_callable* callable_;
		game_logic::formula_callable* mutable_callable_;
		std::vector<variant>* list_;
		std::string* string_;
		std::map<variant,variant>* map_;
	};

	void release();
};

/**
 * Iterator class for the variant.
 *
 * Depending on the @p type_ the @p list_iterator_ and the @p map_iterator_ are
 * a valid iterator or singular. Since most actions on singular iterators
 * result in Undefined Behavior care should be taken when copying the
 * @p list_iterator_ and @p map_iterator_.
 */
class variant_iterator {
public:
	typedef variant value_type;
	typedef std::bidirectional_iterator_tag iterator_category;
	typedef variant& reference;
	typedef variant* pointer;
	typedef int difference_type;
	
	/**
	 * Constructor for a TYPE_NULL variant.
	 */
	variant_iterator();

	/**
	 * Constructor for a TYPE_LIST variant.
	 *
	 * @pre @p iter is not singular.
	 *
	 * @param iter                Iterator to initialize @p list_iterator_ with.
	 */
	explicit variant_iterator(const std::vector<variant>::iterator& iter);

	/**
	 * Constructor for a TYPE_MAP variant.
	 *
	 * @pre @p iter is not singular.
	 *
	 * @param iter                Iterator to initialize @p map_iterator_ with.
	 */
	explicit variant_iterator(const std::map<variant, variant>::iterator& iter);

	variant_iterator(const variant_iterator&);

	variant operator*() const;
	variant_iterator& operator++();
	variant_iterator operator++(int);
	variant_iterator& operator--();
	variant_iterator operator--(int);
	variant_iterator& operator=(const variant_iterator& that);
	bool operator==(const variant_iterator& that) const;
	bool operator!=(const variant_iterator& that) const;

	enum TYPE { TYPE_NULL, TYPE_LIST, TYPE_MAP };
private:
	TYPE type_;
	std::vector<variant>::iterator list_iterator_;
	std::map<variant,variant>::iterator map_iterator_;
};

template<typename T>
T* convert_variant(const variant& v) {
	T* res = dynamic_cast<T*>(v.mutable_callable());
	if(!res) {
		throw type_error("could not convert type");
	}

	return res;
}


template<typename T>
T* try_convert_variant(const variant& v) {
	if(!v.is_callable()) {
		return nullptr;
	}

	return dynamic_cast<T*>(v.mutable_callable());
}



#endif
