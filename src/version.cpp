/*
   Copyright (C) 2008 - 2018 by Iris Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "version.hpp"
#include "lexical_cast.hpp"
#include "serialization/string_utils.hpp"

#include <cassert>
#include <functional>
#include <locale>

#include <boost/algorithm/string.hpp>

version_info::version_info()
	: nums_(3,0), special_(""), special_separator_('\0')
{
}

version_info::version_info(unsigned int major, unsigned int minor, unsigned int revision_level,
                           char special_separator, const std::string& special)
	: nums_(3,0), special_(special), special_separator_(special_separator)
{
	nums_[0] = major;
	nums_[1] = minor;
	nums_[2] = revision_level;
}

version_info::version_info(const std::string& str)
	: nums_(3,0)
	, special_("")
	, special_separator_('\0')
{
	std::string v = str;
	boost::trim(v);

	if(v.empty())
		return;

	//
	// The breakpoint is where the "special" version component begins.
	// For 1.1.2a it would at the index of the char 'a'. For 1.1.4+dev it is at '+'.
	//
	// For 1.5.2 it is at npos.
	//
	const std::string::size_type breakpoint_pos = v.find_first_not_of(".0123456789");
	std::string left_side;
	if(breakpoint_pos != std::string::npos) {
		const std::string right_side = v.substr(breakpoint_pos);
		assert(right_side.empty() == false);

		if(std::isalpha(right_side[0], std::locale::classic())) {
			special_separator_ = '\0';
			special_ = right_side;
		}
		else {
			special_separator_ = right_side[0];
			if(right_side.size() > 1) {
				special_ = right_side.substr(1);
			}
		}

		left_side = v.substr(0, breakpoint_pos);
	}
	else {
		left_side = v;
	}

	const std::vector<std::string> components = utils::split(left_side, '.');
	const size_t s = components.size();
	if(s == 0) {
		return;
	}
	else if(s > 3) {
		nums_.resize(s, 0);
	}

	for(size_t i = 0; (i < s); ++i) {
		nums_[i] = lexical_cast_default<unsigned int>(components[i]);
	}
}

std::string version_info::str() const
{
	const size_t s = nums_.size();

	std::ostringstream o;
	for(size_t k = 0; k < s; ++k) {
		o << nums_[k];

		if(s != 1+k) {
			o << '.';
		}
	}

	if(! special_.empty()) {
		if(special_separator_ != '\0') {
			o << special_separator_;
		}

		o << special_;
	}

	return o.str();
}

void version_info::set_major_version(unsigned int v) {
	nums_[0] = v;
}

void version_info::set_minor_version(unsigned int v) {
	nums_[1] = v;
}

void version_info::set_revision_level(unsigned int v) {
	nums_[2] = v;
}

unsigned int version_info::major_version() const {
	return nums_[0];
}

unsigned int version_info::minor_version() const {
	return nums_[1];
}

unsigned int version_info::revision_level() const {
	return nums_[2];
}

bool version_info::is_canonical() const {
	return nums_.size() <= 3;
}

namespace {
	template<template<typename> class Fcn>
	bool version_comparison_internal(const version_info& l, const version_info& r)
	{
		std::vector<unsigned int> lc = l.components();
		std::vector<unsigned int> rc = r.components();

		const size_t lsize = lc.size();
		const size_t rsize = rc.size();
		const size_t csize = std::max(lsize, rsize);

		// make compatible, missing items default to zero
		if(lsize < csize) lc.resize(csize, 0);
		if(rsize < csize) rc.resize(csize, 0);

		using comp_list = std::vector<unsigned int>;
		using comp_pair = std::tuple<const comp_list&, const std::string&>;
		Fcn<comp_pair> comp;

		const comp_pair& lp = std::tie(lc, l.special_version());
		const comp_pair& rp = std::tie(rc, r.special_version());
		return comp(lp, rp);
	}
} // end unnamed namespace

bool operator==(const version_info& l, const version_info& r)
{
	return version_comparison_internal<std::equal_to>(l, r);
}

bool operator!=(const version_info& l, const version_info& r)
{
	return version_comparison_internal<std::not_equal_to>(l, r);
}

bool operator<(const version_info& l, const version_info& r)
{
	return version_comparison_internal<std::less>(l, r);
}

bool operator>(const version_info& l, const version_info& r)
{
	return version_comparison_internal<std::greater>(l, r);
}

bool operator<=(const version_info& l, const version_info& r)
{
	return version_comparison_internal<std::less_equal>(l, r);
}

bool operator>=(const version_info& l, const version_info& r)
{
	return version_comparison_internal<std::greater_equal>(l, r);
}

VERSION_COMP_OP parse_version_op(const std::string& op_str)
{
	if(op_str == "==") {
		return OP_EQUAL;
	} else if(op_str == "!=") {
		return OP_NOT_EQUAL;
	} else if(op_str == "<") {
		return OP_LESS;
	} else if(op_str == "<=") {
		return OP_LESS_OR_EQUAL;
	} else if(op_str == ">") {
		return OP_GREATER;
	} else if(op_str == ">=") {
		return OP_GREATER_OR_EQUAL;
	}

	return OP_INVALID;
}

bool do_version_check(const version_info& a, VERSION_COMP_OP op, const version_info& b)
{
	switch(op) {
	case OP_EQUAL:
		return a == b;
	case OP_NOT_EQUAL:
		return a != b;
	case OP_LESS:
		return a < b;
	case OP_LESS_OR_EQUAL:
		return a <= b;
	case OP_GREATER:
		return a > b;
	case OP_GREATER_OR_EQUAL:
		return a >= b;
	default:
		;
	}

	return false;
}
