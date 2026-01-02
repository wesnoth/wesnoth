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

#include "exceptions.hpp"
#include "formula/callable_fwd.hpp"
#include "formula_variant.hpp"
#include "utils/any.hpp"
#include "utils/general.hpp"

#include <functional>
#include <map>
#include <vector>
#include <boost/range/iterator_range.hpp>

namespace wfl
{
class variant_value_base;
class variant_iterator;
class variant;

using variant_vector = std::vector<variant>;
using variant_map_raw = std::map<variant, variant>;
using value_base_ptr = std::shared_ptr<variant_value_base>;

struct type_error : public game::error
{
	explicit type_error(const std::string& str);
};

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
	} catch(const std::bad_cast&) {
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
	virtual bool equals(variant_value_base& /*other*/) const
	{
		return true; // null is equal to null
	}

	/**
	 * Called to determine if this variant is less than another _of the same type_.
	 * This function is _only_ called if get_type() returns the same result for both arguments.
	 */
	virtual bool less_than(variant_value_base& /*other*/) const
	{
		return false; // null is not less than null
	}

	/** Returns the id of the variant type */
	virtual formula_variant::type get_type() const
	{
		return formula_variant::type::null;
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

	virtual bool equals(variant_value_base& other) const override
	{
		return value_ == value_ref_cast<variant_numeric>(other).value_;
	}

	virtual bool less_than(variant_value_base& other) const override
	{
		return value_ < value_ref_cast<variant_numeric>(other).value_;
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

	virtual formula_variant::type get_type() const override
	{
		return formula_variant::type::integer;
	}
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

	virtual formula_variant::type get_type() const override
	{
		return formula_variant::type::decimal;
	}

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

	virtual bool equals(variant_value_base& other) const override;
	virtual bool less_than(variant_value_base& other) const override;

	virtual formula_variant::type get_type() const override
	{
		return formula_variant::type::object;
	}

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

	virtual bool equals(variant_value_base& other) const override
	{
		return string_ == value_ref_cast<variant_string>(other).string_;
	}

	virtual bool less_than(variant_value_base& other) const override
	{
		return string_ < value_ref_cast<variant_string>(other).string_;
	}

	virtual formula_variant::type get_type() const override
	{
		return formula_variant::type::string;
	}

private:
	std::string string_;
};

/**
 * Generalized interface for container variants.
 */
template<typename Derived>
class variant_container : public variant_value_base
{
protected:
	/** Only derived classes can instantiate this class. */
	variant_container() = default;

public:
	virtual bool is_empty() const override
	{
		return container().empty();
	}

	virtual std::size_t num_elements() const override
	{
		return container().size();
	}

	virtual bool as_bool() const override
	{
		return !is_empty();
	}

	virtual std::string string_cast() const override;

	virtual std::string get_serialized_string() const override;

	virtual std::string get_debug_string(formula_seen_stack& seen, bool verbose) const override;

	bool contains(const variant& member) const
	{
		return utils::contains(container(), member);
	}

	// We implement these here since the interface is the same for all
	// specializations and leave the deref function to the derived classes.
	virtual boost::iterator_range<variant_iterator> make_iterator() const override;

	virtual void iterator_inc(utils::any&) const override;
	virtual void iterator_dec(utils::any&) const override;
	virtual bool iterator_equals(const utils::any& first, const utils::any& second) const override;

	/** Inherited from variant_value_base. */
	virtual bool equals(variant_value_base& other) const override
	{
		return container_for(*this) == container_for(other);
	}

	/** Inherited from variant_value_base. */
	virtual bool less_than(variant_value_base& other) const override
	{
		return container_for(*this) < container_for(other);
	}

protected:
	using to_string_op = std::function<std::string(const variant&)>;

private:
	/**
	 * String conversion helper for @ref string_cast, @ref get_serialized_string,
	 * and @ref get_debug_string.
	 *
	 * Derived classes should implement container-specific handling by defining a
	 * static to_string_detail function which takes the container's value_type as
	 * its first parameter and a to_string_op functor as its second.
	 */
	std::string to_string_impl(bool annotate, bool annotate_empty, const to_string_op& mod_func) const;

	/** Read-only access to the underlying container. */
	const auto& container() const
	{
		return container_for(*this);
	}

	/** Helper to call get_container for the derived class. */
	static const auto& container_for(const variant_value_base& value)
	{
		return static_cast<const Derived&>(value).get_container();
	}
};


class variant_list : public variant_container<variant_list>
{
public:
	friend class variant_container<variant_list>;

	explicit variant_list(const variant_vector& vec)
		: container_(vec)
	{
	}

	explicit variant_list(variant_vector&& vec)
		: container_(std::move(vec))
	{
	}

	const variant_vector& get_container() const
	{
		return container_;
	}


	virtual formula_variant::type get_type() const override
	{
		return formula_variant::type::list;
	}

	virtual variant deref_iterator(const utils::any&) const override;

private:
	/** Helper for @ref variant_container::to_string_impl. */
	static std::string to_string_detail(const variant& value, const to_string_op& op)
	{
		return op(value);
	}

	variant_vector container_;
};


class variant_map : public variant_container<variant_map>
{
public:
	friend class variant_container<variant_map>;

	explicit variant_map(const variant_map_raw& map)
		: container_(map)
	{
	}

	explicit variant_map(variant_map_raw&& map)
		: container_(std::move(map))
	{
	}

	const variant_map_raw& get_container() const
	{
		return container_;
	}


	virtual formula_variant::type get_type() const override
	{
		return formula_variant::type::map;
	}

	virtual variant deref_iterator(const utils::any&) const override;

private:
	/** Helper for @ref variant_container::to_string_impl. */
	static std::string to_string_detail(const variant_map_raw::value_type& value, const to_string_op& op);

	variant_map_raw container_;
};

} // namespace wfl
