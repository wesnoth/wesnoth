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

#include "formula/variant.hpp"
#include "formula/variant_value.hpp"

#include "formula/callable.hpp"
#include "formula/function.hpp"

namespace wfl
{

boost::iterator_range<variant_iterator> variant_value_base::make_iterator() const
{
	return {variant_iterator(), variant_iterator()};
}

variant variant_value_base::deref_iterator(const boost::any& /*iter*/) const
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

	return variant(res);
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
	: callable_(callable)
{
	if(callable_) {
		callable_->subscribe_dtor(this);
	}
}

variant_callable::~variant_callable() {
	if(callable_) {
		callable_->unsubscribe_dtor(this);
	}
}

std::string variant_callable::get_serialized_string() const
{
	// TODO: make serialize return a string.
	std::string str;
	if(callable_) {
		callable_->serialize(str);
	}

	return str;
}

std::string variant_callable::get_debug_string(formula_seen_stack& seen, bool verbose) const
{
	std::ostringstream ss;
	ss << "{";

	if(!callable_) {
		ss << "null";
	} else if(std::find(seen.begin(), seen.end(), callable_) == seen.end()) {
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

			if(input.access == FORMULA_READ_WRITE) {
				ss << "(read-write) ";
			} else if(input.access == FORMULA_WRITE_ONLY) {
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

bool variant_callable::equals(variant_value_base& other) const
{
	variant_callable& other_ref = value_ref_cast<variant_callable>(other);
	return callable_ ? callable_->equals(*other_ref.callable_) : callable_ == other_ref.callable_;
}

bool variant_callable::less_than(variant_value_base& other) const
{
	variant_callable& other_ref = value_ref_cast<variant_callable>(other);
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

	return {variant_iterator(this, inputs.cbegin()), variant_iterator(this, inputs.cend())};
}

variant variant_callable::deref_iterator(const boost::any& iter) const
{
	if(!callable_) {
		return variant();
	}

	return callable_->query_value(boost::any_cast<const formula_input_vector::const_iterator&>(iter)->name);
}

void variant_callable::iterator_inc(boost::any& iter) const
{
	++boost::any_cast<formula_input_vector::const_iterator&>(iter);
}

void variant_callable::iterator_dec(boost::any& iter) const
{
	--boost::any_cast<formula_input_vector::const_iterator&>(iter);
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

template<typename T>
std::string variant_container<T>::to_string_impl(bool annotate, bool annotate_empty, mod_func_t mod_func) const
{
	std::ostringstream ss;

	if(annotate) {
		ss << "[";
	}

	bool first_time = true;

	for(const auto& member : container_) {
		if(!first_time) {
			ss << ", ";
		}

		first_time = false;

		ss << to_string_detail(member, mod_func);
	}

	// TODO: evaluate if this really needs to be separately conditional.
	if(annotate_empty && container_.empty()) {
		ss << "->";
	}

	if(annotate) {
		ss << "]";
	}

	return ss.str();
}

template<typename T>
std::string variant_container<T>::string_cast() const
{
	return to_string_impl(false, false, [](const variant& v) { return v.string_cast(); });
}

template<typename T>
std::string variant_container<T>::get_serialized_string() const
{
	return to_string_impl(true, true,   [](const variant& v) { return v.serialize_to_string(); });
}

template<typename T>
std::string variant_container<T>::get_debug_string(formula_seen_stack& seen, bool verbose) const
{
	return to_string_impl(true, false, [&](const variant& v) { return v.to_debug_string(verbose, &seen); });
}

template<typename T>
boost::iterator_range<variant_iterator> variant_container<T>::make_iterator() const
{
	return {variant_iterator(this, get_container().cbegin()), variant_iterator(this, get_container().cend())};
}

template<typename T>
void variant_container<T>::iterator_inc(boost::any& iter) const
{
	++boost::any_cast<typename T::const_iterator&>(iter);
}

template<typename T>
void variant_container<T>::iterator_dec(boost::any& iter) const
{
	--boost::any_cast<typename T::const_iterator&>(iter);
}

template<typename T>
bool variant_container<T>::iterator_equals(const boost::any& first, const boost::any& second) const
{
	return boost::any_cast<typename T::const_iterator>(first) == boost::any_cast<typename T::const_iterator>(second);
}

// Force compilation of the following template instantiations
template class variant_container<variant_vector>;
template class variant_container<variant_map_raw>;

variant variant_list::list_op(value_base_ptr second, std::function<variant(variant&, variant&)> op_func)
{
	const auto& other_list = value_cast<variant_list>(second);

	if(num_elements() != other_list->num_elements()) {
		throw type_error("List op requires two lists of the same length");
	}

	std::vector<variant> res;
	res.reserve(num_elements());

	for(size_t i = 0; i < num_elements(); ++i) {
		res.push_back(op_func(get_container()[i], other_list->get_container()[i]));
	}

	return variant(res);
}

bool variant_list::equals(variant_value_base& other) const
{
	const auto& other_container = value_ref_cast<variant_list>(other).get_container();

	if(num_elements() != other.num_elements()) {
		return false;
	}

	for(size_t n = 0; n < num_elements(); ++n) {
		if(get_container()[n] != other_container[n]) {
			return false;
		}
	}

	return true;
}

bool variant_list::less_than(variant_value_base& other) const
{
	const auto& other_container = value_ref_cast<variant_list>(other).get_container();

	for(size_t n = 0; n != num_elements() && n != other.num_elements(); ++n) {
		if(get_container()[n] < other_container[n]) {
			return true;
		} else if(get_container()[n] > other_container[n]) {
			return false;
		}
	}

	return num_elements() < other.num_elements();
}

variant variant_list::deref_iterator(const boost::any& iter) const
{
	return *boost::any_cast<const variant_vector::const_iterator&>(iter);
}

std::string variant_map::to_string_detail(const variant_map_raw::value_type& container_val, mod_func_t mod_func) const
{
	std::ostringstream ss;

	ss << mod_func(container_val.first);
	ss << "->";
	ss << mod_func(container_val.second);

	return ss.str();
}

bool variant_map::equals(variant_value_base& other) const
{
	return get_container() == value_ref_cast<variant_map>(other).get_container();
}

bool variant_map::less_than(variant_value_base& other) const
{
	return get_container() < value_ref_cast<variant_map>(other).get_container();
}

variant variant_map::deref_iterator(const boost::any& iter) const
{
	const variant_map_raw::value_type& p = *boost::any_cast<const variant_map_raw::const_iterator&>(iter);
	auto the_pair = std::make_shared<key_value_pair>(p.first, p.second);
	return variant(the_pair);
}

} // namespace wfl
