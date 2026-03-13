/*
	Copyright (C) 2017 - 2025
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

#include "formula/callable_fwd.hpp"
#include "formula_variant.hpp"
#include "utils/any.hpp"
#include "utils/general.hpp"

#include <functional>
#include <map>
#include <vector>
#include <boost/range/iterator_range.hpp>

namespace utils
{
template<typename To, typename From>
inline To& cast_as(To&, From& value)
{
	return static_cast<To&>(value);
}

} // namespace utils

namespace wfl
{
class variant_value_base;
class variant_iterator;
class variant;

#define IMPLEMENT_VALUE_TYPE(value)                                                                                    \
	static constexpr auto value_type = value;                                                                          \
	formula_variant::type get_type() const override                                                                    \
	{                                                                                                                  \
		return value_type;                                                                                             \
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
	virtual std::size_t num_elements() const
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

	/**
	 * Called to determine if this variant is equal to another _of the same type_.
	 * This function is _only_ called if get_type() returns the same result for both arguments.
	 */
	virtual bool equals(const variant_value_base& /*other*/) const
	{
		return true; // null is equal to null
	}

	/**
	 * Called to determine if this variant is less than another _of the same type_.
	 * This function is _only_ called if get_type() returns the same result for both arguments.
	 */
	virtual bool less_than(const variant_value_base& /*other*/) const
	{
		return false; // null is not less than null
	}

	/** Each 'final' derived class should define a static type flag. */
	static constexpr auto value_type = formula_variant::type::null;

	/** Returns the id of the variant type */
	virtual formula_variant::type get_type() const
	{
		return value_type;
	}

	/**
	 * Creates an iterator pair that can be used for iteration.
	 * For an iterable type, it should use the two-argument constructor of variant-iterator,
	 * passing the underlying iterator as the utils::any parameter.
	 *
	 * This creates both the begin and end iterator, but the variant implementation
	 * discards one of the two.
	 */
	virtual boost::iterator_range<variant_iterator> make_iterator() const;

	/**
	 * Implements the dereference functionality of variant_iterator
	 * for a value of this type.
	 *
	 * @param iter The opaque reference that was passed to the variant_iterator by @ref make_iterator.
	 */
	virtual variant deref_iterator(const utils::any& iter) const;

	/**
	 * Implements the increment functionality of variant_iterator
	 * for a value of this type.
	 *
	 * The parameter is an opaque reference that was passed to the variant_iterator by @ref make_iterator.
	 */
	virtual void iterator_inc(utils::any&) const {}

	/**
	 * Implements the decrement functionality of variant_iterator
	 * for a value of this type.
	 *
	 * The parameter is an opaque reference that was passed to the variant_iterator by @ref make_iterator.
	 */
	virtual void iterator_dec(utils::any&) const {}

	/**
	 * Implements the equality functionality of variant_iterator
	 * for a value of this type.
	 *
	 * Note that this is only called if the two iterators are already known to be of the same type.
	 *
	 * The first parameter is an opaque reference that was passed to the variant_iterator by @ref make_iterator.
	 * The second parameter is an opaque reference that was passed to the variant_iterator by @ref make_iterator.
	 */
	virtual bool iterator_equals(const utils::any& /*first*/, const utils::any& /*second*/) const
	{
		return true;
	}

	virtual ~variant_value_base() {}
};


/**
 * Base class for numeric variant values. Currently only supports a value stored as an
 * integer, but for now, that's all that's necessary.
 */
class variant_numeric : public variant_value_base
{
public:
	explicit variant_numeric(int value) : value_(value) {}

	virtual bool as_bool() const override
	{
		return value_ != 0;
	}

	int get_numeric_value() const
	{
		return value_;
	}

	virtual bool equals(const variant_value_base& other) const override
	{
		return value_ == utils::cast_as(*this, other).value_;
	}

	virtual bool less_than(const variant_value_base& other) const override
	{
		return value_ < utils::cast_as(*this, other).value_;
	}

protected:
	int value_;
};


class variant_int : public variant_numeric
{
public:
	explicit variant_int(int value) : variant_numeric(value) {}

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

	/** Required by variant_value_base. */
	IMPLEMENT_VALUE_TYPE(formula_variant::type::integer)
};


class variant_decimal : public variant_numeric
{
public:
	explicit variant_decimal(int value) : variant_numeric(value) {}

	explicit variant_decimal(double value) : variant_numeric(0)
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

	/** Required by variant_value_base. */
	IMPLEMENT_VALUE_TYPE(formula_variant::type::decimal)

private:
	std::string to_string_impl(const bool sign_value) const;
};


class variant_callable : public variant_value_base, private callable_die_subscriber
{
public:
	explicit variant_callable(const_formula_callable_ptr callable);
	~variant_callable();

	virtual bool as_bool() const override
	{
		return callable_ != nullptr;
	}

	virtual std::size_t num_elements() const override
	{
		return 1;
	}

	const_formula_callable_ptr get_callable() const
	{
		return callable_;
	}

	virtual std::string string_cast() const override
	{
		return "(object)";
	}

	virtual std::string get_serialized_string() const override;

	virtual std::string get_debug_string(formula_seen_stack& seen, bool verbose) const override;

	virtual bool equals(const variant_value_base& other) const override;
	virtual bool less_than(const variant_value_base& other) const override;

	/** Required by variant_value_base. */
	IMPLEMENT_VALUE_TYPE(formula_variant::type::object)

	virtual boost::iterator_range<variant_iterator> make_iterator() const override;
	virtual variant deref_iterator(const utils::any& iter) const override;

	virtual void iterator_inc(utils::any& iter) const override;
	virtual void iterator_dec(utils::any& iter) const override;
	virtual bool iterator_equals(const utils::any& /*first*/, const utils::any& /*second*/) const override
	{
		return true; // TODO: implement
	}

private:
	void notify_dead() override {callable_.reset();}

	mutable formula_input_vector inputs; // for iteration
	const_formula_callable_ptr callable_;
};


class variant_string : public variant_value_base
{
public:
	explicit variant_string(const std::string& str) : string_(str) {}
	explicit variant_string(std::string&& str) : string_(std::move(str)) {}

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

	virtual bool equals(const variant_value_base& other) const override
	{
		return string_ == utils::cast_as(*this, other).string_;
	}

	virtual bool less_than(const variant_value_base& other) const override
	{
		return string_ < utils::cast_as(*this, other).string_;
	}

	/** Required by variant_value_base. */
	IMPLEMENT_VALUE_TYPE(formula_variant::type::string)

private:
	std::string string_;
};

/**
 * Generalized interface for container variants.
 */
template<typename Container>
class variant_container : public variant_value_base
{
public:
	explicit variant_container(const Container& c)
		: container_(c)
	{
	}

	explicit variant_container(Container&& c)
		: container_(std::move(c))
	{
	}

	const Container& get_container() const
	{
		return container_;
	}

	virtual bool is_empty() const override
	{
		return container_.empty();
	}

	virtual std::size_t num_elements() const override
	{
		return container_.size();
	}

	virtual bool as_bool() const override
	{
		return !is_empty();
	}

	virtual std::string string_cast() const override;
	virtual std::string get_serialized_string() const override;
	virtual std::string get_debug_string(formula_seen_stack& seen, bool verbose) const override;

	// We implement these here since the interface is the same for all
	// specializations and leave the deref function to the derived classes.
	virtual boost::iterator_range<variant_iterator> make_iterator() const override;

	virtual void iterator_inc(utils::any&) const override;
	virtual void iterator_dec(utils::any&) const override;
	virtual bool iterator_equals(const utils::any& first, const utils::any& second) const override;

	/** Inherited from variant_value_base. */
	virtual bool equals(const variant_value_base& other) const override
	{
		return container_ == utils::cast_as(*this, other).container_;
	}

	/** Inherited from variant_value_base. */
	virtual bool less_than(const variant_value_base& other) const override
	{
		return container_ < utils::cast_as(*this, other).container_;
	}

protected:
	using iterator       = typename Container::iterator;
	using const_iterator = typename Container::const_iterator;

	/** Casts opaque @a iter to a mutable const_iterator reference. */
	static const_iterator& as_container_iterator(utils::any& iter)
	{
		return utils::any_cast<const_iterator&>(iter);
	}

	/** Casts opaque @a iter to a constant const_iterator reference. */
	static const const_iterator& as_container_iterator(const utils::any& iter)
	{
		return utils::any_cast<const const_iterator&>(iter);
	}

private:
	Container container_;
};


class variant_list : public variant_container<std::vector<variant>>
{
public:
	explicit variant_list(const std::vector<variant>& vec)
		: variant_container(vec)
	{
	}

	explicit variant_list(std::vector<variant>&& vec)
		: variant_container(std::move(vec))
	{
	}

	/** Required by variant_value_base. */
	IMPLEMENT_VALUE_TYPE(formula_variant::type::list)

	virtual variant deref_iterator(const utils::any&) const override;
};


class variant_map : public variant_container<std::map<variant, variant>>
{
public:
	explicit variant_map(const std::map<variant, variant>& map)
		: variant_container(map)
	{
	}

	explicit variant_map(std::map<variant, variant>&& map)
		: variant_container(std::move(map))
	{
	}

	/** Required by variant_value_base. */
	IMPLEMENT_VALUE_TYPE(formula_variant::type::map)

	virtual variant deref_iterator(const utils::any&) const override;
};

#undef IMPLEMENT_VALUE_TYPE

} // namespace wfl
