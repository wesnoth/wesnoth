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

#include "foreach.hpp"
#include "version.hpp"
#include "serialization/string_utils.hpp"
#include "util.hpp"
#include <functional>
#include <cassert>
#include <sstream>
#include <stdexcept>

version_info::version_info(const version_info& o)
	: nums_                 (o.nums_),
	  special_              (o.special_),
	  special_separator_    (o.special_separator_),
	  sane_                 (o.sane_)
{
}

version_info::version_info(void)
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
	: nums_(3,0), sane_(false)
{
	const std::vector<std::string>& string_parts = utils::split(str,',');
	// first two components are required to be valid numbers, though
	// only first component's existence is checked at all
	const size_t parts = string_parts.size();
	if(parts == 0)
		return;

	try {
		nums_[0] = lexical_cast<unsigned int>(string_parts[0]);
		if(parts > 1) {
			nums_[1] = lexical_cast<unsigned int>(string_parts[1]);
		}
		if(parts == 3) {
			nums_[2] = lexical_cast<unsigned int>(string_parts[2]);
		} else if(parts > 2) {
			std::string numstr;
			// Check for special suffix and use it
			this->init_special_version(string_parts[2], numstr);
			nums_[2] = lexical_cast<unsigned int>(numstr);
		}
		if(parts > 3) {
			// Everything else goes to noncanonical space
			nums_.reserve(nums_.size()+parts-3);
			for(size_t i = 3; i < parts; ++i) {
				nums_.push_back(0);
				if(i == parts-1) {
					std::string numstr;
					// Check for special suffix and use it
					this->init_special_version(string_parts[i], numstr);
					nums_[i] = lexical_cast<unsigned int>(numstr);
				} else {
					nums_[i] = lexical_cast<unsigned int>(string_parts[i]);
				}
			}
		}
	} catch (bad_lexical_cast const&) {
		return;
	} catch (std::out_of_range const&) {
		return;
	}
}

void version_info::init_special_version(const std::string& full_component, std::string& number_string)
{
	const std::string::size_type sep_pos = full_component.find_first_not_of("0123456789");
	if(sep_pos != std::string::npos) {
		const char& c = full_component[sep_pos];
		if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
			special_separator_ = '\0';
			special_ = c;
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
	if(special_separator_ != '\0') o << special_separator_;
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
	template<class Predicate>
	bool version_info_comparison_internal(const version_info& l, const version_info& r, Predicate comp)
	{
		if((!l.good()) || !r.good()) throw version_info::not_sane_exception();

		const std::vector<unsigned int> lc = l.components();
		const std::vector<unsigned int> rc = r.components();
		const size_t lsize = lc.size();
		const size_t rsize = rc.size();
		const size_t csize = maximum(lsize, rsize);
		
		unsigned int lcomp, rcomp;
		
		bool res = true;
		for(size_t i = 0; res && i < csize; ++i) {
			lcomp = i >= lsize ? 0 : lc[i];
			rcomp = i >= rsize ? 0 : rc[i];
			res = res && comp(lcomp, rcomp);
		}
		return res;
	}
	
} // end unnamed namespace

bool operator==(const version_info& l, const version_info& r)
{
	std::equal_to<unsigned int> o;
	return version_info_comparison_internal(l, r, o) && l.special_version() == r.special_version();
}

bool operator!=(const version_info& l, const version_info& r)
{
	std::not_equal_to<unsigned int> o;
	return version_info_comparison_internal(l, r, o) && l.special_version() != r.special_version();
}

bool operator<(const version_info& l, const version_info& r)
{
	std::less<unsigned int> o;
	return version_info_comparison_internal(l, r, o) &&
	       ((l.special_version().empty() && !r.special_version().empty()) ||
	        l.special_version() < r.special_version());
}

bool operator>(const version_info& l, const version_info& r)
{
	std::greater<unsigned int> o;
	return version_info_comparison_internal(l, r, o) &&
	       ((r.special_version().empty() && !l.special_version().empty()) ||
	        l.special_version() > r.special_version());
}

bool operator<=(const version_info& l, const version_info& r)
{
	return l < r || l == r;
}

bool operator>=(const version_info& l, const version_info& r)
{
	return l > r || l == r;
}
