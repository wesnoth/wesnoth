/* $Id$ */
/*
   Copyright (C) 2004 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include <vector>
#include <map>

#include "tstring.hpp"
#include "wassert.hpp"
#include "gettext.hpp"
#include "filesystem.hpp"
#include "log.hpp"

#define ERR_CF lg::err(lg::config)

namespace {
	const char TRANSLATABLE_PART = 0x01;
	const char UNTRANSLATABLE_PART = 0x02;
	const char TEXTDOMAIN_SEPARATOR = 0x03;
	const char ID_TRANSLATABLE_PART = 0x04;

	std::vector<std::string> id_to_textdomain;
	std::map<std::string, unsigned int> textdomain_to_id;
}

t_string::walker::walker(const t_string& string) :
	string_(string.value_),
	begin_(0)
{
	if(!string.translatable_) {
		begin_ = 0;
		end_ = string_.size();
		translatable_ = false;
	} else {
		update();
	}
}

t_string::walker::walker(const std::string& string) :
	string_(string),
	begin_(0)
{
	update();
}

void t_string::walker::next()
{
	begin_ = end_;
	update();
}

bool t_string::walker::eos() const
{
	return begin_ == string_.size();
}

bool t_string::walker::last() const
{
	return end_ == string_.size();
}

bool t_string::walker::translatable() const
{
	return translatable_;
}

const std::string& t_string::walker::textdomain() const
{
	return textdomain_;
}

std::string::const_iterator t_string::walker::begin() const
{
	return string_.begin() + begin_;
}

std::string::const_iterator t_string::walker::end() const
{
	return string_.begin() + end_;
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
			ERR_CF << "Error: invalid string\n";
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
			ERR_CF << "Error: invalid string\n";
			begin_ = string_.size();
			return;
		}
		end_ = string_.find_first_of(mark, begin_ + 3);
		if(end_ == std::string::npos)
			end_ = string_.size();

		id = string_[begin_ + 1] + string_[begin_ + 2] * 256;
		if(id >= id_to_textdomain.size()) {
			ERR_CF << "Error: invalid string\n";
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
			ERR_CF << "Error: invalid string\n";
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
	value_()
{
}

t_string::t_string(const t_string& string) :
	translatable_(string.translatable_),
	value_(string.value_)
{
}

t_string::t_string(const std::string& string) :
	translatable_(false),
	value_(string)
{
}

t_string::t_string(const std::string& string, const std::string& textdomain) :
	translatable_(true),
	value_(1, ID_TRANSLATABLE_PART)
{
	std::map<std::string, unsigned int>::const_iterator idi = textdomain_to_id.find(textdomain);
	unsigned int id;

	if(idi == textdomain_to_id.end()) {
		textdomain_to_id[textdomain] = id_to_textdomain.size();
		id = id_to_textdomain.size();
		id_to_textdomain.push_back(textdomain);

		// Register and bind this textdomain
		bindtextdomain(textdomain.c_str(), get_intl_dir().c_str());
		bind_textdomain_codeset(textdomain.c_str(), "UTF-8");
	} else {
		id = idi->second;
	}

	value_ += char(id & 0xff);
	value_ += char(id >> 8);
	value_ += string;
}

t_string::t_string(const char* string) :
	translatable_(false),
	value_(string)
{
}

t_string::~t_string()
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

std::string t_string::to_serialized() const
{
	t_string res;

	for(walker w(*this); !w.eos(); w.next()) {
		t_string chunk;

		std::string substr(w.begin(), w.end());
		if(w.translatable()) {
			chunk.translatable_ = true;
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

bool t_string::operator==(const t_string& string) const
{
	return string.translatable_ == translatable_ && string.value_ == value_;
}

bool t_string::operator==(const std::string& string) const
{
	return !translatable_ && value_ == string;
}

bool t_string::operator==(const char* string) const
{
	return !translatable_ && value_ == string;
}

t_string t_string::operator+(const t_string& string) const
{
	t_string res;

	if(translatable_ || string.translatable_) {
		res.translatable_ = true;
		if(translatable_) {
			res.value_ += value_;
		} else {
			if (!value_.empty()) {
				res.value_ += UNTRANSLATABLE_PART;
				res.value_ += value_;
			}
		}
		if(string.translatable_) {
			res.value_ += string.value_;
		} else {
			if (!string.empty()) {
				res.value_ += UNTRANSLATABLE_PART;
				res.value_ += string.value_;
			}
		}
	} else {
		res.translatable_ = false;
		res.value_ = value_ + string.value_;
	}

	return res;
}

t_string t_string::operator+(const std::string& string) const
{
	t_string res;

	if(translatable_) {
		res.translatable_ = true;
		res.value_ = value_;
		if (!string.empty()) {
			res.value_ += UNTRANSLATABLE_PART;
			res.value_ += string;
		}
	} else {
		res.translatable_ = false;
		res.value_ = value_ + string;
	}

	return res;
}

t_string t_string::operator+(const char* string) const
{
	t_string res;

	if(translatable_) {
		res.translatable_ = true;
		res.value_ = value_;
		if (string[0] != 0) {
			res.value_ += UNTRANSLATABLE_PART;
			res.value_ += string;
		}
	} else {
		res.translatable_ = false;
		res.value_ = value_ + string;
	}

	return res;
}

t_string& t_string::operator+=(const t_string& string)
{
	if(translatable_ || string.translatable_) {
		if(!translatable_) {
			if (!value_.empty()) {
				value_ = UNTRANSLATABLE_PART + value_;
			}
		}
		if(string.translatable_) {
			value_ += string.value_;
		} else {
			if (!string.value_.empty()) {
				value_ += UNTRANSLATABLE_PART;
				value_ += string.value_;
			}
		}
		translatable_ = true;
	} else {
		translatable_ = false;
		value_ += string.value_;
	}

	return *this;
}

t_string& t_string::operator+=(const std::string& string)
{
	if(translatable_) {
		if (!string.empty()) {
			value_ += UNTRANSLATABLE_PART;
			value_ += string;
		}
	} else {
		value_ += string;
	}

	return *this;
}

t_string& t_string::operator+=(const char* string) 
{
	if(translatable_) {
		if (string[0] != 0) {
			value_ += UNTRANSLATABLE_PART;
			value_ += string;
		}
	} else {
		value_ += string;
	}

	return *this;
}

bool t_string::operator!=(const t_string& string) const { return !(*this == string); }
bool t_string::operator!=(const std::string& string) const { return !(*this == string); }
bool t_string::operator!=(const char* string) const { return !(*this == string); }

bool t_string::empty() const 
{
	return value_.empty();
}

std::string::size_type t_string::size() const
{
	return str().size();
}

t_string::operator const std::string&() const
{
	return str();
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

const char* t_string::c_str() const
{
	return str().c_str();
}

const std::string& t_string::value() const
{
	return value_;
}

std::ostream& operator<<(std::ostream& stream, const t_string& string)
{
	stream << string.str();
	return stream;
}

bool operator==(const std::string& a, const t_string& b) 
{ 
	return b == a; 
}

bool operator==(const char* a, const t_string& b) 
{ 
	return b == a; 
}

bool operator!=(const std::string& a, const t_string& b) 
{ 
	return b != a; 
}

bool operator!=(const char* a, const t_string& b) 
{ 
	return b != a; 
}

t_string operator+(const std::string& a, const t_string& b)
{
	return t_string(a) + b;
}

t_string operator+(const char*a, const t_string& b)
{
	return t_string(a) + b;
}

