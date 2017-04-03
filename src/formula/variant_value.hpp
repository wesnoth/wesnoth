/*
   Copyright (C) 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef VARIANT_PRIVATE_HPP_INCLUDED
#define VARIANT_PRIVATE_HPP_INCLUDED

#include "exceptions.hpp"
#include "formula/callable_fwd.hpp"
#include "utils/general.hpp"
#include "utils/make_enum.hpp"

#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <utility>
#include <vector>

class variant;

/** The various types the variant class is designed to handle */
MAKE_ENUM(VARIANT_TYPE,
	(TYPE_NULL,     "null")
	(TYPE_INT,      "int")
	(TYPE_DECIMAL,  "decimal")
	(TYPE_CALLABLE, "object")
	(TYPE_LIST,     "list")
	(TYPE_STRING,   "string")
	(TYPE_MAP,      "map")
);

using variant_vector = std::vector<variant>;
using variant_map_raw = std::map<variant, variant>;

struct type_error : public game::error
{
	explicit type_error(const std::string& str);
};

namespace game_logic
{
class formula_callable;
class variant_value_base;

using value_base_ptr = std::shared_ptr<variant_value_base>;

/** Casts a @ref variant_value_base shared pointer to a new derived type. */
template<typename T>
static std::shared_ptr<T> value_cast(value_base_ptr ptr)
{
	std::shared_ptr<T> res = std::dynamic_pointer_cast<T>(ptr);
	if(!res) {
		throw type_error("Could not cast type");
	}

	return res;
}

/** Casts a @ref variant_value_base reference to a new derived type. */
template<typename T>
static T& value_ref_cast(variant_value_base& ptr)
{
	try {
		return dynamic_cast<T&>(ptr);
	} catch(std::bad_cast&) {
		throw type_error("Could not cast type");
	}
}

/**
 * Base class for all variant types.
 *
 * This provides a common interface for all type classes to implement, as well as
 * giving variant a base pointer type for its value member. It also serves as the
 * implementation of the 'null' variant value.
 *
 * Do note this class should *not* implement any data members.
 */
class variant_value_base
{
public:
	/** Returns the number of elements in a type. Not relevant for every derivative. */
	virtual size_t num_elements() const
	{
		return 0;
	}

	/** Whether the stored value is considered empty or not. */
	virtual bool is_empty() const
	{
		return true;
	}

	/** Returns the stored variant value in plain string form. */
	virtual std::string string_cast() const
	{
		return "0";
	}

	/** Returns the stored variant value in formula syntax. */
	virtual std::string get_serialized_string() const
	{
		return "null()";
	}

	/** Returns debug info for the variant value. */
	virtual std::string get_debug_string(formula_seen_stack& /*seen*/, bool /*verbose*/) const
	{
		return get_serialized_string();
	}

	/** Returns a bool expression of the variant value. */
	virtual bool as_bool() const
	{
		return false;
	}

	/** Called to determine if this variant is equal to another _of the same type_.
	This function is _only_ called if get_type() returns the same result for both arguments. */
	virtual bool equals(variant_value_base& /*other*/) const
	{
		return true; // null is equal to null
	}

	/** Called to determine if this variant is less than another _of the same type_.
	This function is _only_ called if get_type() returns the same result for both arguments. */
	virtual bool less_than(variant_value_base& /*other*/) const
	{
		return false; // null is not less than null
	}

	/** Returns the id of the variant type */
	virtual const VARIANT_TYPE& get_type() const
	{
		static VARIANT_TYPE type = VARIANT_TYPE::TYPE_NULL;
		return type;
	}
};


class variant_int : public virtual variant_value_base
{
public:
	explicit variant_int(int value) : value_(value) {}

	virtual bool as_bool() const override
	{
		return value_ != 0;
	}

	int get_integer() const
	{
		return value_;
	}

	variant build_range_variant(int limit) const;

	virtual std::string string_cast() const override
	{
		return std::to_string(value_);
	}

	virtual std::string get_serialized_string() const override
	{
		return string_cast();
	}

	virtual std::string get_debug_string(formula_seen_stack& /*seen*/, bool /*verbose*/) const override
	{
		return string_cast();
	}

	virtual bool equals(variant_value_base& other) const override
	{
		return value_ == value_ref_cast<variant_int>(other).value_;
	}

	virtual bool less_than(variant_value_base& other) const override
	{
		return value_ < value_ref_cast<variant_int>(other).value_;
	}

	virtual const VARIANT_TYPE& get_type() const override
	{
		static VARIANT_TYPE type = VARIANT_TYPE::TYPE_INT;
		return type;
	}

private:
	int value_;
};


class variant_decimal : public virtual variant_value_base
{
public:
	explicit variant_decimal(int value) : value_(value) {}

	explicit variant_decimal(double value) : value_(0)
	{
		value *= 1000;
		value_ = static_cast<int>(value);
		value -= value_;

		if(value > 0.5) {
			value_++;
		} else if(value < -0.5) {
			value_--;
		}
	}

	virtual bool as_bool() const override
	{
		return value_ != 0;
	}

	int get_decimal() const
	{
		return value_;
	}

	virtual std::string string_cast() const override
	{
		return to_string_impl(false);
	}

	virtual std::string get_serialized_string() const override
	{
		return to_string_impl(false);
	}

	virtual std::string get_debug_string(formula_seen_stack& /*seen*/, bool /*verbose*/) const override
	{
		return to_string_impl(true);
	}

	virtual bool equals(variant_value_base& other) const override
	{
		return value_ == value_ref_cast<variant_decimal>(other).value_;
	}

	virtual bool less_than(variant_value_base& other) const override
	{
		return value_ < value_ref_cast<variant_decimal>(other).value_;
	}

	virtual const VARIANT_TYPE& get_type() const override
	{
		static VARIANT_TYPE type = VARIANT_TYPE::TYPE_DECIMAL;
		return type;
	}

private:
	std::string to_string_impl(const bool sign_value) const;

	int value_;
};


class variant_callable : public virtual variant_value_base
{
public:
	explicit variant_callable(const formula_callable* callable);

	virtual bool as_bool() const override
	{
		return callable_ != nullptr;
	}

	virtual size_t num_elements() const override
	{
		return 1;
	}

	const formula_callable* get_callable() const
	{
		return callable_;
	}

	virtual std::string string_cast() const override
	{
		return "(object)";
	}

	virtual std::string get_serialized_string() const override;

	virtual std::string get_debug_string(formula_seen_stack& seen, bool verbose) const override;

	virtual bool equals(variant_value_base& other) const override;
	virtual bool less_than(variant_value_base& other) const override;

	virtual const VARIANT_TYPE& get_type() const override
	{
		static VARIANT_TYPE type = VARIANT_TYPE::TYPE_CALLABLE;
		return type;
	}

private:
	const formula_callable* callable_;
};


class variant_string : public virtual variant_value_base
{
public:
	explicit variant_string(const std::string& str) : string_(str) {}

	virtual bool is_empty() const override
	{
		return string_.empty();
	}

	virtual bool as_bool() const override
	{
		return !is_empty();
	}

	const std::string& get_string() const
	{
		return string_;
	}

	virtual std::string string_cast() const override
	{
		return string_;
	}

	virtual std::string get_serialized_string() const override;

	virtual std::string get_debug_string(formula_seen_stack& /*seen*/, bool /*verbose*/) const override
	{
		return string_;
	}

	virtual bool equals(variant_value_base& other) const override
	{
		return string_ == value_ref_cast<variant_string>(other).string_;
	}

	virtual bool less_than(variant_value_base& other) const override
	{
		return string_ < value_ref_cast<variant_string>(other).string_;
	}

	virtual const VARIANT_TYPE& get_type() const override
	{
		static VARIANT_TYPE type = VARIANT_TYPE::TYPE_STRING;
		return type;
	}

private:
	std::string string_;
};

/**
 * Generalized implementation handling container variants.
 *
 * This class shouldn't usually be used directly. Instead, it's better to
 * create a new derived class specialized to a specific container type.
 */
template<typename T>
class variant_container : public virtual variant_value_base
{
public:
	explicit variant_container(const T& container)
		: container_(container)
	{
		// NOTE: add more conditions if this changes.
		static_assert((std::is_same<variant_vector, T>::value || std::is_same<variant_map_raw, T>::value),
			"variant_container only accepts vector or map specifications.");
	}

	virtual bool is_empty() const override
	{
		return container_.empty();
	}

	virtual size_t num_elements() const override
	{
		return container_.size();
	}

	virtual bool as_bool() const override
	{
		return !is_empty();
	}

	T& get_container()
	{
		return container_;
	}

	const T& get_container() const
	{
		return container_;
	}

	virtual std::string string_cast() const override;

	virtual std::string get_serialized_string() const override;

	virtual std::string get_debug_string(formula_seen_stack& seen, bool verbose) const override;

	bool contains(const variant& member) const
	{
		return util::contains<T, variant>(container_, member);
	}

protected:
	using mod_func_t = std::function<std::string(const variant&)>;

	virtual std::string to_string_detail(const typename T::value_type& value, mod_func_t mod_func) const = 0;

private:
	/**
	 * Implementation to handle string conversion for @ref string_cast, @ref get_serialized_string,
	 * and @ref get_debug_string.
	 *
	 * Derived classes should provide type-specific value handling by implementing @ref to_string_detail.
	 */
	std::string to_string_impl(bool annotate, bool annotate_empty, mod_func_t mod_func) const;

	T container_;
};


class variant_list : public variant_container<variant_vector>
{
public:
	explicit variant_list(const variant_vector& vec)
		: variant_container<variant_vector>(vec)
	{}

	/**
	 * Applies the provided function to the corresponding variants in this and another list.
	 */
	variant list_op(value_base_ptr second, std::function<variant(variant&, variant&)> op_func);

	virtual bool equals(variant_value_base& other) const override;
	virtual bool less_than(variant_value_base& other) const override;

	virtual const VARIANT_TYPE& get_type() const override
	{
		static VARIANT_TYPE type = VARIANT_TYPE::TYPE_LIST;
		return type;
	}

private:
	virtual std::string to_string_detail(const variant_vector::value_type& container_val, mod_func_t mod_func) const override
	{
		return mod_func(container_val);
	}
};


class variant_map : public variant_container<variant_map_raw>
{
public:
	explicit variant_map(const variant_map_raw& map)
		: variant_container<variant_map_raw>(map)
	{}

	virtual bool equals(variant_value_base& other) const override;
	virtual bool less_than(variant_value_base& other) const override;

	virtual const VARIANT_TYPE& get_type() const override
	{
		static VARIANT_TYPE type = VARIANT_TYPE::TYPE_MAP;
		return type;
	}

private:
	virtual std::string to_string_detail(const variant_map_raw::value_type& container_val, mod_func_t mod_func) const override;
};

} // namespace game_logic

#endif
