/* $Id$ */
/*
   Copyright (C) 2008 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "version.hpp"
#include "serialization/string_utils.hpp"

#include <cassert>
#include <stdexcept>


version_info::version_info(const version_info& o)
	: nums_                 (o.nums_),
	  special_              (o.special_),
	  special_separator_    (o.special_separator_),
	  sane_                 (o.sane_)
{
}

version_info::version_info()
	: nums_(3,0), special_(""), special_separator_('\0'), sane_(true)
{
}

version_info::version_info(unsigned int major, unsigned int minor, unsigned int revision_level, bool sane,
                           char special_separator, const std::string& special)
	: nums_(3,0), special_(special), special_separator_(special_separator), sane_(sane)
{
	nums_[0] = major;
	nums_[1] = minor;
	nums_[2] = revision_level;
}

version_info::version_info(const std::string& str)
	: nums_(3,0)
	, special_("")
	, special_separator_('\0')
	, sane_(true)
{
	if(str.empty())
		return;

	// first two components are required to be valid numbers
	const std::vector<std::string> string_parts = utils::split(str,'.');
	const size_t parts = string_parts.size();
	if(parts == 0)
		return;

	if(parts > 3)
		nums_.resize(parts, 0);

	try {
		size_t i = 0;
		for(; i < parts-1; i++)
			nums_[i] = lexical_cast<unsigned int>(string_parts[i]);

		// Check for special suffix on last number and use it
		std::string numstr;
		this->init_special_version(string_parts[i], numstr);
		nums_[i] = lexical_cast<unsigned int>(numstr);
	}
	catch (bad_lexical_cast const&) {
		sane_ = false;
	}
	catch (std::out_of_range const&) {
		;
	}
}

void version_info::init_special_version(const std::string& full_component, std::string& number_string)
{
	const std::string::size_type sep_pos = full_component.find_first_not_of("0123456789");
	if(sep_pos != std::string::npos) {
		const char& c = full_component[sep_pos];
		if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
			special_separator_ = '\0';
			special_ = full_component.substr(sep_pos);
		} else {
			special_separator_ = c;
			if(sep_pos != full_component.size() - 1) {
				special_ = full_component.substr(1+sep_pos);
			} else {
				special_ = "";
			}
		}
		number_string = full_component.substr(0,sep_pos);
	} else {
		number_string = full_component;
	}
}


void version_info::assign(const version_info& o)
{
	if(&o == this) return;
	
	this->sane_ = o.sane_;
	this->special_separator_ = o.special_separator_;
	this->special_ = o.special_;
	this->nums_.clear();
	this->nums_ = o.nums_;
}

std::string version_info::str() const
{
	const size_t items = nums_.size();
	assert(items >= 3);

	std::ostringstream o;
	o << nums_[0] << '.' << nums_[1] << '.' << nums_[2];
	if(items > 3) {
		o << '.';
		for(size_t i = 3; i < items; ++i)
			o << nums_[i] << '.';
	}
	if(special_separator_ != '\0' && special_.empty() != true) o << special_separator_;
	if(special_.empty() != true)   o << special_;

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
	return nums_.size() <= 3 && sane_;
}

namespace {
	enum COMP_TYPE {
		EQUAL,
		NOT_EQUAL,
		LT, GT
	};

	namespace {
		static const size_t max_recursions = 256;
		static size_t level = 0;
	}
	
	/*
	           x         >          y
	x0.x1.x2.x3.[...].xN > y0.y1.y2.y3.[...].yN iff
	
	x0 > y0 || (x0 == y0 && (x1 > y1 || (x1 == y1 && (x2 > y2 || (x2 >= y2 || 
	
	*/
	template<typename _Toperator, typename _Tfallback_operator>
	bool recursive_order_operation(const std::vector<unsigned int>& l, const std::vector<unsigned int>& r, size_t k)
	{
		if(k >= l.size() || k >= r.size() || ++level > max_recursions) {
			return false;
		}

		unsigned int const& lvalue = l[k];
		unsigned int const& rvalue = r[k];

		_Toperator o;
		_Tfallback_operator fallback_o;

		bool ret = o(lvalue, rvalue);
		if((!ret) && fallback_o(lvalue, rvalue)) {
			ret = recursive_order_operation<_Toperator, _Tfallback_operator>(l,r,++k);
		}
		return ret;
	}

	bool version_numbers_comparison_internal(const version_info& l, const version_info& r, COMP_TYPE o)
	{
		if((!l.good()) || !r.good()) throw version_info::not_sane_exception();

		std::vector<unsigned int> lc = l.components();
		std::vector<unsigned int> rc = r.components();

		const size_t lsize = lc.size();
		const size_t rsize = rc.size();
		const size_t csize = std::max(lsize, rsize);

		// make compatible, missing items default to zero
		if(lsize < csize) lc.resize(csize, 0);
		if(rsize < csize) rc.resize(csize, 0);
		
		bool result = true;
		
		const std::vector<unsigned int>& lcc = lc;
		const std::vector<unsigned int>& rcc = rc;
		
		switch(o)
		{
			case EQUAL: case NOT_EQUAL: {			
				for(size_t i = 0; i < csize; ++i) {
					unsigned int const& lvalue = lc[i];
					unsigned int const& rvalue = rc[i];
					if(o == NOT_EQUAL) {
						if((result = (lvalue != rvalue))) {
							return true;
						}
						continue;
					} else {
						result = result && lvalue == rvalue;
						if(!result) {
							break;
						}
					}
				}
				break;
			}
			case LT:
				result = recursive_order_operation<std::less<unsigned int>, std::equal_to<unsigned int> >(lcc, rcc, 0);
				break;
			case GT:
				result = recursive_order_operation<std::greater<unsigned int>, std::equal_to<unsigned int> >(lcc, rcc, 0);
				break;
			default:
				assert(0 == 1);
				break;
		}
		return result;
	}
	
} // end unnamed namespace

bool operator==(const version_info& l, const version_info& r)
{
	return version_numbers_comparison_internal(l, r, EQUAL) && l.special_version() == r.special_version();
}

bool operator!=(const version_info& l, const version_info& r)
{
	return version_numbers_comparison_internal(l, r, NOT_EQUAL) || l.special_version() != r.special_version();
}

bool operator<(const version_info& l, const version_info& r)
{
	return version_numbers_comparison_internal(l, r, LT) || (
		version_numbers_comparison_internal(l, r, EQUAL) && (
			(l.special_version().empty() && !r.special_version().empty()) ||
			(l.special_version() < r.special_version())
		)
	);
}

bool operator>(const version_info& l, const version_info& r)
{
	return version_numbers_comparison_internal(l, r, GT) || (
		version_numbers_comparison_internal(l, r, EQUAL) && (
			(r.special_version().empty() && !l.special_version().empty()) ||
			(l.special_version() > r.special_version())
		)
	);
}

bool operator<=(const version_info& l, const version_info& r)
{
	return l < r || l == r;
}

bool operator>=(const version_info& l, const version_info& r)
{
	return l > r || l == r;
}
