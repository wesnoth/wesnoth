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

#include "formula/variant.hpp"
#include "formula/variant_value.hpp"

#include "utils/ranges.hpp"
#include <utility>

#include "formula/callable.hpp"
#include "formula/function.hpp"
#include "serialization/string_utils.hpp"

namespace wfl
{
namespace implementation
{
template<typename Range>
auto make_iterator_range(const variant_value_base* val, const Range& range) -> boost::iterator_range<variant_iterator>
{
	return {
		variant_iterator{val, std::cbegin(range)},
		variant_iterator{val, std::cend(range)}
	};
}

} // namespace implementation

boost::iterator_range<variant_iterator> variant_value_base::make_iterator() const
{
	return {variant_iterator(), variant_iterator()};
}

variant variant_value_base::deref_iterator(const utils::any& /*iter*/) const
{
	return variant();
}

variant variant_int::build_range_variant(int limit) const
{
	const int len = std::abs(limit - value_) + 1;

	std::vector<variant> res;
	res.reserve(len);

	for(int i = value_; res.size() != res.capacity(); value_ < limit ? ++i : --i) {
		res.emplace_back(i);
	}

	return variant(std::move(res));
}

std::string variant_decimal::to_string_impl(const bool sign_value) const
{
	std::ostringstream ss;

	int fractional =  value_ % 1000;
	int integer    = (value_ - fractional) / 1000;

	if(sign_value) {
		// Make sure we get the sign on small negative values.
		if(integer == 0 && value_ < 0) {
			ss << '-';
		}
	}

	ss << integer << ".";

	fractional = std::abs(fractional);

	if(fractional < 100) {
		if(fractional < 10) {
			ss << "00";
		} else {
			ss << 0;
		}
	}

	ss << fractional;

	return ss.str();
}

variant_callable::variant_callable(const_formula_callable_ptr callable)
	: callable_(std::move(callable))
{
	if(callable_) {
		callable_->subscribe_dtor(this);
	}
}

variant_callable::~variant_callable()
{
	if(callable_) {
		callable_->unsubscribe_dtor(this);
	}
}

std::string variant_callable::get_serialized_string() const
{
	if(callable_) {
		return callable_->serialize();
	}

	return {};
}

std::string variant_callable::get_debug_string(formula_seen_stack& seen, bool verbose) const
{
	std::ostringstream ss;
	ss << "{";

	if(!callable_) {
		ss << "null";
	} else if(!utils::contains(seen, callable_)) {
		if(!verbose) {
			seen.push_back(callable_);
		}

		formula_input_vector v = callable_->inputs();
		bool first = true;

		for(const auto& input : v) {
			if(!first) {
				ss << ", ";
			}

			first = false;
			ss << input.name << " ";

			if(input.access == formula_access::read_write) {
				ss << "(read-write) ";
			} else if(input.access == formula_access::write_only) {
				ss << "(writeonly) ";
			}

			ss << "-> " << callable_->query_value(input.name).to_debug_string(verbose, &seen);
		}
	} else {
		ss << "...";
	}

	ss << "}";

	return ss.str();
}

bool variant_callable::equals(const variant_value_base& other) const
{
	const variant_callable& other_ref = utils::cast_as(*this, other);
	return callable_ ? callable_->equals(*other_ref.callable_) : callable_ == other_ref.callable_;
}

bool variant_callable::less_than(const variant_value_base& other) const
{
	const variant_callable& other_ref = utils::cast_as(*this, other);
	return callable_ ? callable_->less(*other_ref.callable_) : other_ref.callable_ != nullptr;
}

boost::iterator_range<variant_iterator> variant_callable::make_iterator() const
{
	if(!callable_) {
		return variant_value_base::make_iterator();
	}

	if(inputs.empty()) {
		callable_->get_inputs(inputs);
	}

	return implementation::make_iterator_range(this, inputs);
}

variant variant_callable::deref_iterator(const utils::any& iter) const
{
	if(!callable_) {
		return variant();
	}

	return callable_->query_value(utils::any_cast<const formula_input_vector::const_iterator&>(iter)->name);
}

void variant_callable::iterator_inc(utils::any& iter) const
{
	++utils::any_cast<formula_input_vector::const_iterator&>(iter);
}

void variant_callable::iterator_dec(utils::any& iter) const
{
	--utils::any_cast<formula_input_vector::const_iterator&>(iter);
}

std::string variant_string::get_serialized_string() const
{
	std::ostringstream ss;
	ss << "'";

	for(const auto& c : string_) {
		switch(c) {
		case '\'':
			ss << "[']";
			break;
		case '[':
			ss << "[(]";
			break;
		case ']':
			ss << "[)]";
			break;
		default:
			ss << c;
			break;
		}
	}

	ss << "'";

	return ss.str();
}

namespace implementation
{
namespace detail
{
template<typename Func>
std::string serialize_value(const Func& op, const variant& value)
{
	return std::invoke(op, value);
}

template<typename Func>
std::string serialize_value(const Func& op, const std::pair<variant, variant>& value)
{
	std::ostringstream ss;

	ss << std::invoke(op, value.first);
	ss << "->";
	ss << std::invoke(op, value.second);

	return ss.str();
}

template<typename Range, typename Func>
auto make_serialized_range(const Range& range, const Func& op)
{
	return range | utils::views::transform([&op](auto&& value) { return serialize_value(op, value); });
}

/** WFL empty list literal */
std::string serialize_empty(const std::vector<variant>&)
{
	return "[]";
}

/** WFL empty map literal */
std::string serialize_empty(const std::map<variant, variant>&)
{
	return "[->]";
}

} // namespace detail

template<typename Range, typename Func>
std::string to_string(const Range& range, const Func& op)
{
	return utils::join(detail::make_serialized_range(range, op), ", ");
}

template<typename Range, typename Func>
std::string as_literal(const Range& range, const Func& op)
{
	if(range.empty()) {
		return detail::serialize_empty(range);
	}

	std::ostringstream ss;
	ss << "[" << to_string(range, op) << "]";
	return ss.str();
}

} // namespace implementation

template<typename T>
std::string variant_container<T>::string_cast() const
{
	return implementation::to_string(container_, &variant::string_cast);
}

template<typename T>
std::string variant_container<T>::get_serialized_string() const
{
	return implementation::as_literal(container_, &variant::serialize_to_string);
}

template<typename T>
std::string variant_container<T>::get_debug_string(formula_seen_stack& seen, bool verbose) const
{
	return implementation::as_literal(container_,
		[&seen, verbose](const variant& v) { return v.to_debug_string(verbose, &seen); });
}

template<typename T>
boost::iterator_range<variant_iterator> variant_container<T>::make_iterator() const
{
	return implementation::make_iterator_range(this, container_);
}

template<typename T>
void variant_container<T>::iterator_inc(utils::any& iter) const
{
	++as_container_iterator(iter);
}

template<typename T>
void variant_container<T>::iterator_dec(utils::any& iter) const
{
	--as_container_iterator(iter);
}

template<typename T>
bool variant_container<T>::iterator_equals(const utils::any& first, const utils::any& second) const
{
	return as_container_iterator(first) == as_container_iterator(second);
}

// Force compilation of the following template instantiations
template class variant_container<std::vector<variant>>;
template class variant_container<std::map<variant, variant>>;

variant variant_list::deref_iterator(const utils::any& iter) const
{
	return *as_container_iterator(iter);
}

variant variant_map::deref_iterator(const utils::any& iter) const
{
	const auto& [key, value] = *as_container_iterator(iter);
	return variant(std::make_shared<key_value_pair>(key, value));
}

} // namespace wfl
