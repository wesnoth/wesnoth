/*
	Copyright (C) 2008 - 2025
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "exceptions.hpp"
#include "formula/callable_fwd.hpp"
#include "formula/formula_variant.hpp"

#include "utils/any.hpp"
#include <map>
#include <vector>

namespace wfl
{
class formula_callable;
class variant;
class variant_value_base;
class variant_iterator;

struct type_error : public game::error
{
	explicit type_error(const std::string& str);
};

/** @throws wfl::type_error for an incorrect type @a t of variant @a v. */
[[noreturn]] void assert_must_be(formula_variant::type t, const variant& v);

class variant
{
public:
	enum DECIMAL_VARIANT_TYPE { DECIMAL_VARIANT };

	variant();
	explicit variant(int n);
	variant(int n, DECIMAL_VARIANT_TYPE /*type*/);
	variant(double n, DECIMAL_VARIANT_TYPE /*type*/);
	explicit variant(const std::vector<variant>& array);
	explicit variant(std::vector<variant>&& array);
	explicit variant(const std::string& str);
	explicit variant(std::string&& str);
	explicit variant(const std::map<variant, variant>& map);
	explicit variant(std::map<variant, variant>&& map);
	explicit variant(const_formula_callable_ptr callable);
	variant(const variant& v) = default;
	variant(variant&& v) = default;

	variant& operator=(const variant& v) = default;
	variant& operator=(variant&& v) = default;

	variant operator[](std::size_t n) const;
	variant operator[](const variant& v) const;

	std::size_t num_elements() const;
	bool is_empty() const;

	variant get_member(const std::string& name) const;

	/** Functions to test the type of the internal value. */
	bool is_null()     const { return type() == formula_variant::type::null; }
	bool is_int()      const { return type() == formula_variant::type::integer; }
	bool is_decimal()  const { return type() == formula_variant::type::decimal; }
	bool is_callable() const { return type() == formula_variant::type::object; }
	bool is_list()     const { return type() == formula_variant::type::list; }
	bool is_string()   const { return type() == formula_variant::type::string; }
	bool is_map()      const { return type() == formula_variant::type::map; }

	/**
	 * Returns the variant's value as an integer.
	 * If @ref is_null() is true, returns @a fallback.
	 */
	int as_int(int fallback = 0) const;

	/**
	 * Returns the variant's internal representation of decimal number: ie, 1.234 is represented as 1234.
	 * If @ref is_null() is true, returns @a fallback.
	 */
	int as_decimal(int fallback = 0) const;

	/** Returns a boolean state of the variant value. The implementation is type-dependent. */
	bool as_bool() const;

	const std::vector<variant>& as_list() const;
	const std::map<variant, variant>& as_map() const;

	const std::string& as_string() const;

	const_formula_callable_ptr as_callable() const;

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
		return formula_variant::get_string(type());
	}

private:
	template<typename T>
	std::shared_ptr<T> value_cast() const
	{
		auto res = std::dynamic_pointer_cast<T>(value_);
		if(!res) {
			assert_must_be(T::value_type, *this);
		}

		return res;
	}

	void must_both_be(formula_variant::type t, const variant& second) const;

	formula_variant::type type() const;

	/** @invariant Never null. */
	std::shared_ptr<variant_value_base> value_;
};

/**
 * Executes all action_callables in @a execute using the provided context.
 */
variant execute_actions(const variant& execute, const variant& context);

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
	variant_iterator(const variant_value_base* value, const utils::any& iter);

	variant operator*() const;
	variant_iterator& operator++();
	variant_iterator operator++(int);
	variant_iterator& operator--();
	variant_iterator operator--(int);
	bool operator==(const variant_iterator& that) const;
	bool operator!=(const variant_iterator& that) const;
private:
	formula_variant::type type_;
	const variant_value_base* container_;
	utils::any iter_;
};

}
