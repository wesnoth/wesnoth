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

namespace wfl
{
class formula_callable;
class variant_iterator;

class variant
{
public:
	enum DECIMAL_VARIANT_TYPE { DECIMAL_VARIANT };

	variant();
	explicit variant(int n);
	variant(int n, DECIMAL_VARIANT_TYPE /*type*/);
	variant(double n, DECIMAL_VARIANT_TYPE /*type*/);
	explicit variant(const std::vector<variant>& array);
	explicit variant(const std::string& str);
	explicit variant(const std::map<variant, variant>& map);

	template<typename T>
	variant(std::shared_ptr<T> callable)
		: value_(std::make_shared<variant_callable>(callable))
	{
		assert(value_.get());
	}

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

	const_formula_callable_ptr as_callable() const
	{
		must_be(VARIANT_TYPE::TYPE_CALLABLE);
		return value_cast<variant_callable>()->get_callable();
	}

	template<typename T>
	std::shared_ptr<T> try_convert() const
	{
		if(!is_callable()) {
			return nullptr;
		}

		return std::dynamic_pointer_cast<T>(std::const_pointer_cast<formula_callable>(as_callable()));
	}

	template<typename T>
	std::shared_ptr<T> convert_to() const
	{
		std::shared_ptr<T> res = std::dynamic_pointer_cast<T>(std::const_pointer_cast<formula_callable>(as_callable()));
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

	std::string to_debug_string(bool verbose = false, formula_seen_stack* seen = nullptr) const;

	/** Gets string name of the current value type */
	std::string type_string() const
	{
		return type().to_string();
	}

	variant execute_variant(const variant& to_exec);

private:
	template<typename T>
	std::shared_ptr<T> value_cast() const
	{
		return wfl::value_cast<T>(value_);
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
	value_base_ptr value_;
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
	 * Constructor for a no-op iterator.
	 */
	variant_iterator();

	/**
	 * Constructor for a generic iterator.
	 *
	 * @pre @p iter is not singular.
	 *
	 * @param value   A pointer to a variant value representing the container.
	 * @param iter    An underlying iterator for the underlying container.
	 */
	variant_iterator(const variant_value_base* value, const boost::any& iter);

	variant operator*() const;
	variant_iterator& operator++();
	variant_iterator operator++(int);
	variant_iterator& operator--();
	variant_iterator operator--(int);
	bool operator==(const variant_iterator& that) const;
	bool operator!=(const variant_iterator& that) const;
private:
	VARIANT_TYPE type_;
	const variant_value_base* container_;
	boost::any iter_;
};

}

#endif
