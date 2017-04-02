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

#include "formula/variant_value.hpp"

#include <map>
#include <vector>

// TODO: expand to cover variant as well
namespace game_logic
{
	class formula_callable;
}

void push_call_stack(const char* str);
void pop_call_stack();
std::string get_call_stack();

struct call_stack_manager
{
	explicit call_stack_manager(const char* str)
	{
		push_call_stack(str);
	}

	~call_stack_manager()
	{
		pop_call_stack();
	}
};

class variant_iterator;

class variant
{
public:
	enum DECIMAL_VARIANT_TYPE { DECIMAL_VARIANT };

	variant();
	explicit variant(int n);
	variant(int n, DECIMAL_VARIANT_TYPE /*type*/);
	variant(double n, DECIMAL_VARIANT_TYPE /*type*/);
	explicit variant(const game_logic::formula_callable* callable);
	explicit variant(const std::vector<variant>& array);
	explicit variant(const std::string& str);
	explicit variant(const std::map<variant, variant>& map);

	variant& operator=(const variant& v);

	variant operator[](size_t n) const;
	variant operator[](const variant& v) const;

	size_t num_elements() const;
	bool is_empty() const;

	variant get_member(const std::string& name) const;

	/** Functions to test the type of the internal value. */
	bool is_null()     const { return type() == VARIANT_TYPE::TYPE_NULL; }
	bool is_int()      const { return type() == VARIANT_TYPE::TYPE_INT; }
	bool is_decimal()  const { return type() == VARIANT_TYPE::TYPE_DECIMAL; }
	bool is_callable() const { return type() == VARIANT_TYPE::TYPE_CALLABLE; }
	bool is_list()     const { return type() == VARIANT_TYPE::TYPE_LIST; }
	bool is_string()   const { return type() == VARIANT_TYPE::TYPE_STRING; }
	bool is_map()      const { return type() == VARIANT_TYPE::TYPE_MAP; }

	int as_int() const;

	/** Returns variant's internal representation of decimal number: ie, 1.234 is represented as 1234 */
	int as_decimal() const;

	/** Returns a boolean state of the variant value. The implementation is type-dependent. */
	bool as_bool() const;

	const std::vector<variant>& as_list() const;
	const std::map<variant, variant>& as_map() const;

	const std::string& as_string() const;

	const game_logic::formula_callable* as_callable() const
	{
		must_be(VARIANT_TYPE::TYPE_CALLABLE);
		return value_cast<game_logic::variant_callable>()->get_callable();
	}

	template<typename T>
	T* try_convert() const
	{
		if(!is_callable()) {
			return nullptr;
		}

		return dynamic_cast<T*>(const_cast<game_logic::formula_callable*>(as_callable()));
	}

	template<typename T>
	T* convert_to() const
	{
		T* res = dynamic_cast<T*>(const_cast<game_logic::formula_callable*>(as_callable()));
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
	bool operator<(const variant&)  const;
	bool operator>(const variant&)  const;
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

	std::string serialize_to_string() const;
	void serialize_from_string(const std::string& str);

	std::string string_cast() const;

	std::string to_debug_string(game_logic::const_formula_callable_vec* seen = nullptr, bool verbose = false) const;

	/** Gets string name of the current value type */
	std::string type_string() const
	{
		return type().to_string();
	}

private:
	template<typename T>
	std::shared_ptr<T> value_cast() const
	{
		return game_logic::value_cast<T>(value_);
	}

	void must_be(VARIANT_TYPE t) const;

	void must_both_be(VARIANT_TYPE t, const variant& second) const;

	VARIANT_TYPE type() const
	{
		return value_->get_type();
	}

	/**
	 * Variant value.
	 * Each of the constructors initialized this with the appropriate helper class.
	 */
	game_logic::value_base_ptr value_;
};

/**
 * Iterator class for the variant.
 *
 * Depending on the @p type_ the @p list_iterator_ and the @p map_iterator_ are
 * a valid iterator or singular. Since most actions on singular iterators
 * result in Undefined Behavior care should be taken when copying the
 * @p list_iterator_ and @p map_iterator_.
 */
class variant_iterator
{
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

#endif
