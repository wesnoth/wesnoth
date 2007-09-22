/* $Id$ */
/*
   Copyright (C) 2004 by Philippe Plantier <ayin@anathas.org>
   Copyright (C) 2005 - 2007 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file tstring.cpp
//! Routines for translatable strings.

#include "global.hpp"

#include <sstream>
#include <vector>
#include <map>

#include "tstring.hpp"
#include "gettext.hpp"
#include "log.hpp"

#define LOG_CF lg::info(lg::config)
#define ERR_CF lg::err(lg::config)

namespace {
	const char TRANSLATABLE_PART = 0x01;
	const char UNTRANSLATABLE_PART = 0x02;
	const char TEXTDOMAIN_SEPARATOR = 0x03;
	const char ID_TRANSLATABLE_PART = 0x04;
	const char UNTRANSLATABLE_STRING = 0x05;

	std::vector<std::string> id_to_textdomain;
	std::map<std::string, unsigned int> textdomain_to_id;
}

t_string::walker::walker(const t_string& string) :
	string_(string.value_),
	begin_(0),
	end_(string_.size()),
	textdomain_(),
	translatable_(false)
{
	if(string.translatable_) {
		update();
	}
}

t_string::walker::walker(const std::string& string) :
	string_(string),
	begin_(0),
	end_(string_.size()),
	textdomain_(),
	translatable_(false)
{
	update();
}

void t_string::walker::update()
{
	unsigned int id;

	static std::string mark = std::string(TRANSLATABLE_PART, 1) + UNTRANSLATABLE_PART +
		ID_TRANSLATABLE_PART;

	if(begin_ == string_.size())
		return;

	switch(string_[begin_]) {
	case TRANSLATABLE_PART: {
		std::string::size_type textdomain_end =
			string_.find(TEXTDOMAIN_SEPARATOR, begin_ + 1);

		if(textdomain_end == std::string::npos || textdomain_end >= string_.size() - 1) {
			ERR_CF << "Error: invalid string: " << string_ << "\n";
			begin_ = string_.size();
			return;
		}

		end_ = string_.find_first_of(mark, textdomain_end + 1);
		if(end_ == std::string::npos)
			end_ = string_.size();

		textdomain_ = std::string(string_, begin_+1, textdomain_end - begin_ - 1);
		translatable_ = true;
		begin_ = textdomain_end + 1;

		break;
	}
	case ID_TRANSLATABLE_PART:
		if(begin_ + 3 >= string_.size()) {
			ERR_CF << "Error: invalid string: " << string_ << "\n";
			begin_ = string_.size();
			return;
		}
		end_ = string_.find_first_of(mark, begin_ + 3);
		if(end_ == std::string::npos)
			end_ = string_.size();

		id = string_[begin_ + 1] + string_[begin_ + 2] * 256;
		if(id >= id_to_textdomain.size()) {
			ERR_CF << "Error: invalid string: " << string_ << "\n";
			begin_ = string_.size();
			return;
		}
		textdomain_ = id_to_textdomain[id];
		begin_ += 3;
		translatable_ = true;

		break;

	case UNTRANSLATABLE_PART:
		end_ = string_.find_first_of(mark, begin_ + 1);
		if(end_ == std::string::npos)
			end_ = string_.size();

		if(end_ <= begin_ + 1) {
			ERR_CF << "Error: invalid string: " << string_ << "\n";
			begin_ = string_.size();
			return;
		}

		translatable_ = false;
		textdomain_ = "";
		begin_ += 1;
		break;

	default:
		end_ = string_.size();
		translatable_ = false;
		textdomain_ = "";
		break;
	}
}

t_string::t_string() :
	translatable_(false),
	last_untranslatable_(false),
	value_(),
	translated_value_()
{
}

t_string::t_string(const t_string& string) :
	translatable_(string.translatable_),
	last_untranslatable_(string.last_untranslatable_),
	value_(string.value_),
	translated_value_(string.translated_value_)
{
}

t_string::t_string(const std::string& string) :
	translatable_(false),
	last_untranslatable_(false),
	value_(string),
	translated_value_()
{
}

t_string::t_string(const std::string& string, const std::string& textdomain) :
	translatable_(true),
	last_untranslatable_(false),
	value_(1, ID_TRANSLATABLE_PART),
	translated_value_()
{
	std::map<std::string, unsigned int>::const_iterator idi = textdomain_to_id.find(textdomain);
	unsigned int id;

	if(idi == textdomain_to_id.end()) {
		id = id_to_textdomain.size();
		textdomain_to_id[textdomain] = id;
		id_to_textdomain.push_back(textdomain);
	} else {
		id = idi->second;
	}

	value_ += char(id & 0xff);
	value_ += char(id >> 8);
	value_ += string;
}

t_string::t_string(const char* string) :
	translatable_(false),
	last_untranslatable_(false),
	value_(string),
	translated_value_()
{
}

t_string t_string::from_serialized(const std::string& string)
{
	t_string orig(string);

	if(!string.empty() && (string[0] == TRANSLATABLE_PART || string[0] == UNTRANSLATABLE_PART)) {
		orig.translatable_ = true;
	} else {
		orig.translatable_ = false;
	}

	t_string res;

	for(walker w(orig); !w.eos(); w.next()) {
		std::string substr(w.begin(), w.end());

		if(w.translatable()) {
			res += t_string(substr, w.textdomain());
		} else {
			res += substr;
		}
	}

	return res;
}

const std::string t_string::base_str() const
{
	std::string res;
	for(walker w(*this); !w.eos(); w.next()) {
		res += std::string(w.begin(), w.end());
	}
	return res;
}

std::string t_string::to_serialized() const
{
	t_string res;

	for(walker w(*this); !w.eos(); w.next()) {
		t_string chunk;

		std::string substr(w.begin(), w.end());
		if(w.translatable()) {
			chunk.translatable_ = true;
			chunk.last_untranslatable_ = false;
			chunk.value_ = TRANSLATABLE_PART + w.textdomain() +
				TEXTDOMAIN_SEPARATOR + substr;
		} else {
			chunk.translatable_ = false;
			chunk.value_ = substr;
		}

		res += chunk;
	}

	return res.value();
}

t_string& t_string::operator=(const t_string& string)
{
	value_ = string.value_;
	translatable_ = string.translatable_;
	last_untranslatable_ = string.last_untranslatable_;
	translated_value_ = string.translated_value_;

	return *this;
}

t_string& t_string::operator=(const std::string& string)
{
	translatable_ = false;
	value_ =  string;
	translated_value_ = "";

	return *this;
}

t_string& t_string::operator=(const char* string)
{
	translatable_ = false;
	value_ = string;
	translated_value_ = "";

	return *this;
}

t_string t_string::operator+(const t_string& string) const
{
	t_string res(*this);
	res += string;
	return res;
}

t_string t_string::operator+(const std::string& string) const
{
	t_string res(*this);
	res += string;
	return res;
}

t_string t_string::operator+(const char* string) const
{
	t_string res(*this);
	res += string;
	return res;
}

t_string& t_string::operator+=(const t_string& string)
{
	if (string.value_.empty())
		return *this;
	if (value_.empty()) {
		*this = string;
		return *this;
	}

	if(translatable_ || string.translatable_) {
		if(!translatable_) {
			value_ = UNTRANSLATABLE_PART + value_;
			translatable_ = true;
			last_untranslatable_ = true;
		} else
			translated_value_ = "";
		if(string.translatable_) {
			if (last_untranslatable_ && string.value_[0] == UNTRANSLATABLE_PART)
				value_.append(string.value_.begin() + 1, string.value_.end());
			else
				value_ += string.value_;
			last_untranslatable_ = string.last_untranslatable_;
		} else {
			if (!last_untranslatable_) {
				value_ += UNTRANSLATABLE_PART;
				last_untranslatable_ = true;
			}
			value_ += string.value_;
		}
	} else {
		value_ += string.value_;
	}

	return *this;
}

t_string& t_string::operator+=(const std::string& string)
{
	if (string.empty())
		return *this;
	if (value_.empty()) {
		*this = string;
		return *this;
	}

	if(translatable_) {
		if (!last_untranslatable_) {
			value_ += UNTRANSLATABLE_PART;
			last_untranslatable_ = true;
		}
		value_ += string;
		translated_value_ = "";
	} else {
		value_ += string;
	}

	return *this;
}

t_string& t_string::operator+=(const char* string)
{
	if (string[0] == 0)
		return *this;
	if (value_.empty()) {
		*this = string;
		return *this;
	}

	if(translatable_) {
		if (!last_untranslatable_) {
			value_ += UNTRANSLATABLE_PART;
			last_untranslatable_ = true;
		}
		value_ += string;
		translated_value_ = "";
	} else {
		value_ += string;
	}

	return *this;
}


const std::string& t_string::str() const
{
	if(!translatable_)
		return value_;

	if(translatable_ && !translated_value_.empty())
		return translated_value_;

	for(walker w(*this); !w.eos(); w.next()) {
		std::string part(w.begin(), w.end());

		if(w.translatable()) {
			translated_value_ += dsgettext(w.textdomain().c_str(), part.c_str());
		} else {
			translated_value_ += part;
		}
	}

	return translated_value_;
}


void t_string::add_textdomain(const std::string& name, const std::string& path)
{
	LOG_CF << "Binding textdomain " << name << " to path " << path << "\n";

	// Register and (re-)bind this textdomain
	bindtextdomain(name.c_str(), path.c_str());
	bind_textdomain_codeset(name.c_str(), "UTF-8");
}

std::ostream& operator<<(std::ostream& stream, const t_string& string)
{
	stream << string.str();
	return stream;
}

