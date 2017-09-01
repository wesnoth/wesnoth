/*
   Copyright (C) 2004 by Philippe Plantier <ayin@anathas.org>
   Copyright (C) 2005 - 2017 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file
 *  Routines for translatable strings.
 */

#include <map>
#include <vector>
#include <mutex>

#include "tstring.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include <boost/functional/hash_fwd.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>

static lg::log_domain log_config("config");
#define LOG_CF LOG_STREAM(info, log_config)
#define ERR_CF LOG_STREAM(err, log_config)

static unsigned language_counter = 0;

namespace {
	const char TRANSLATABLE_PART = 0x01;
	const char UNTRANSLATABLE_PART = 0x02;
	const char TEXTDOMAIN_SEPARATOR = 0x03;
	const char ID_TRANSLATABLE_PART = 0x04;
	const char PLURAL_PART = 0x05;

	std::vector<std::string> id_to_textdomain;
	std::map<std::string, unsigned int> textdomain_to_id;
}

size_t t_string_base::hash_value() const {
	size_t seed = 0;
	boost::hash_combine(seed, value_);
	boost::hash_combine(seed, translatable_);
	boost::hash_combine(seed, last_untranslatable_);
	return seed;
}

t_string_base::walker::walker(const t_string_base& string) :
	string_(string.value_),
	begin_(0),
	end_(string_.size()),
	textdomain_(),
	translatable_(false),
	countable_(false),
	count_(0)
{
	if(string.translatable_) {
		update();
	}
}

static std::string mark = std::string(TRANSLATABLE_PART, 1) + UNTRANSLATABLE_PART +
	ID_TRANSLATABLE_PART + PLURAL_PART;

void t_string_base::walker::update()
{
	unsigned int id;

	if(begin_ == string_.size())
		return;

	switch(string_[begin_]) {
	case TRANSLATABLE_PART: {
		// Format: [TRANSLATABLE_PART]textdomain[TEXTDOMAIN_SEPARATOR]msgid[...]
		std::string::size_type textdomain_end =
			string_.find(TEXTDOMAIN_SEPARATOR, begin_ + 1);

		if(textdomain_end == std::string::npos || textdomain_end >= string_.size() - 1) {
			ERR_CF << "Error: invalid string: " << string_ << std::endl;
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
		// Format: [ID_TRANSLATABLE_PART][2-byte textdomain ID]msgid[...]
		if(begin_ + 3 >= string_.size()) {
			ERR_CF << "Error: invalid string: " << string_ << std::endl;
			begin_ = string_.size();
			return;
		}
		end_ = string_.find_first_of(mark, begin_ + 3);
		if(end_ == std::string::npos)
			end_ = string_.size();

		id = string_[begin_ + 1] + string_[begin_ + 2] * 256;
		if(id >= id_to_textdomain.size()) {
			ERR_CF << "Error: invalid string: " << string_ << std::endl;
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
			ERR_CF << "Error: invalid string: " << string_ << std::endl;
			begin_ = string_.size();
			return;
		}

		translatable_ = false;
		textdomain_ = "";
		begin_ += 1;
		break;

	case PLURAL_PART:
		begin_ = string_.find_first_of(mark, end_ + 5);
		if(begin_ == std::string::npos)
			begin_ = string_.size();
		if(string_[begin_] == PLURAL_PART) {
			ERR_CF << "Error: invalid string: " << string_ << std::endl;
			begin_ = string_.size();
			return;
		}
		update();
		break;

	default:
		end_ = string_.size();
		translatable_ = false;
		textdomain_ = "";
		break;
	}

	if(translatable_ && string_[end_] == PLURAL_PART) {
		// Format: [PLURAL_PART][4-byte count]msgid_plural[...]
		if(end_ + 5 >= string_.size()) {
			ERR_CF << "Error: invalid string: " << string_ << std::endl;
			begin_ = string_.size();
			return;
		}
		std::string::size_type real_end = string_.find_first_of(mark, end_ + 6);
		if(real_end < string_.size() && string_[real_end] == PLURAL_PART) {
			ERR_CF << "Error: invalid string: " << string_ << std::endl;
			begin_ = string_.size();
			return;
		}
		countable_ = true;
		union {
			int32_t count;
			char data[4];
		} cvt;
		std::copy_n(string_.data() + end_ + 1, 4, cvt.data);
		count_ = cvt.count;
	} else {
		countable_ = false;
		count_ = 0;
	}
}

std::string::const_iterator t_string_base::walker::plural_begin() const
{
	if(!countable_) {
		return begin();
	}

	return end() + 5;
}

std::string::const_iterator t_string_base::walker::plural_end() const
{
	if(!countable_) {
		return end();
	}

	std::string::size_type pl_end = string_.find_first_of(mark, end_ + 5);
	if(pl_end == std::string::npos) {
		pl_end = string_.size();
	}
	return string_.begin() + pl_end;
}

t_string_base::t_string_base() :
	value_(),
	translated_value_(),
	translation_timestamp_(0),
	translatable_(false),
	last_untranslatable_(false)
{
}

t_string_base::~t_string_base()
{
}

t_string_base::t_string_base(const t_string_base& string) :
	value_(string.value_),
	translated_value_(string.translated_value_),
	translation_timestamp_(string.translation_timestamp_),
	translatable_(string.translatable_),
	last_untranslatable_(string.last_untranslatable_)
{
}

t_string_base::t_string_base(const std::string& string) :
	value_(string),
	translated_value_(),
	translation_timestamp_(0),
	translatable_(false),
	last_untranslatable_(false)
{
}

t_string_base::t_string_base(const std::string& string, const std::string& textdomain) :
	value_(1, ID_TRANSLATABLE_PART),
	translated_value_(),
	translation_timestamp_(0),
	translatable_(true),
	last_untranslatable_(false)
{
	if (string.empty()) {
		value_.clear();
		translatable_ = false;
		return;
	}

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

t_string_base::t_string_base(const std::string& sing, const std::string& pl, int count, const std::string& textdomain) :
	value_(1, ID_TRANSLATABLE_PART),
	translated_value_(),
	translation_timestamp_(0),
	translatable_(true),
	last_untranslatable_(false)
{
	if (sing.empty() && pl.empty()) {
		value_.clear();
		translatable_ = false;
		return;
	}

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
	value_ += sing;
	value_ += PLURAL_PART;

	union {
		int32_t count;
		char data[4];
	} cvt;
	cvt.count = count;
	for(char c : cvt.data) {
		value_ += c;
	}
	value_ += pl;
}

t_string_base::t_string_base(const char* string) :
	value_(string),
	translated_value_(),
	translation_timestamp_(0),
	translatable_(false),
	last_untranslatable_(false)
{
}

t_string_base t_string_base::from_serialized(const std::string& string)
{
	t_string_base orig(string);

	if(!string.empty() && (string[0] == TRANSLATABLE_PART || string[0] == UNTRANSLATABLE_PART)) {
		orig.translatable_ = true;
	} else {
		orig.translatable_ = false;
	}

	t_string_base res;

	for(walker w(orig); !w.eos(); w.next()) {
		std::string substr(w.begin(), w.end());

		if(w.translatable()) {
			res += t_string_base(substr, w.textdomain());
		} else {
			res += substr;
		}
	}

	return res;
}

std::string t_string_base::base_str() const
{
	std::string res;
	for(walker w(*this); !w.eos(); w.next()) {
		res += std::string(w.begin(), w.end());
	}
	return res;
}

std::string t_string_base::to_serialized() const
{
	t_string_base res;

	for(walker w(*this); !w.eos(); w.next()) {
		t_string_base chunk;

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

t_string_base& t_string_base::operator=(const t_string_base& string)
{
	value_ = string.value_;
	translated_value_ = string.translated_value_;
	translation_timestamp_ = string.translation_timestamp_;
	translatable_ = string.translatable_;
	last_untranslatable_ = string.last_untranslatable_;

	return *this;
}

t_string_base& t_string_base::operator=(const std::string& string)
{
	value_ = string;
	translated_value_ = "";
	translation_timestamp_ = 0;
	translatable_ = false;
	last_untranslatable_ = false;

	return *this;
}

t_string_base& t_string_base::operator=(const char* string)
{
	value_ = string;
	translated_value_ = "";
	translation_timestamp_ = 0;
	translatable_ = false;
	last_untranslatable_ = false;

	return *this;
}

t_string_base t_string_base::operator+(const t_string_base& string) const
{
	t_string_base res(*this);
	res += string;
	return res;
}

t_string_base t_string_base::operator+(const std::string& string) const
{
	t_string_base res(*this);
	res += string;
	return res;
}

t_string_base t_string_base::operator+(const char* string) const
{
	t_string_base res(*this);
	res += string;
	return res;
}

t_string_base& t_string_base::operator+=(const t_string_base& string)
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

t_string_base& t_string_base::operator+=(const std::string& string)
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

t_string_base& t_string_base::operator+=(const char* string)
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

bool t_string_base::operator==(const t_string_base &that) const
{
	return that.translatable_ == translatable_ && that.value_ == value_;
}

bool t_string_base::operator==(const std::string &that) const
{
	return !translatable_ && value_ == that;
}

bool t_string_base::operator==(const char *that) const
{
	return !translatable_ && value_ == that;
}

bool t_string_base::operator<(const t_string_base &that) const
{
	return value_ < that.value_;
}

const std::string& t_string_base::str() const
{
	if(!translatable_)
		return value_;

	if (translatable_ && !translated_value_.empty() && translation_timestamp_ == language_counter)
		return translated_value_;

	translated_value_.clear();

	for(walker w(*this); !w.eos(); w.next()) {
		std::string part(w.begin(), w.end());

		if(w.translatable()) {
			if(w.countable()) {
				std::string plural(w.plural_begin(), w.plural_end());
				translated_value_ += translation::dsngettext(w.textdomain().c_str(), part.c_str(), plural.c_str(), w.count());
			} else {
				translated_value_ += translation::dsgettext(w.textdomain().c_str(), part.c_str());
			}
		} else {
			translated_value_ += part;
		}
	}

	translation_timestamp_ = language_counter;
	return translated_value_;
}

t_string::t_string() : val_(new base())
{
}

t_string::~t_string()
{
}

t_string::t_string(const t_string &o) : val_(o.val_)
{
}

t_string::t_string(const base &o) : val_(new base(o))
{
}

t_string::t_string(const char *o) : val_(new base(o))
{
}

t_string::t_string(const std::string &o) : val_(new base(o))
{
}

t_string::t_string(const std::string &o, const std::string &textdomain) : val_(new base(o, textdomain))
{
}

t_string::t_string(const std::string &s, const std::string& pl, int c, const std::string &textdomain)
		: val_(new base(s, pl, c, textdomain))
{
}

t_string &t_string::operator=(const t_string &o)
{
	val_ = o.val_;
	return *this;
}

t_string &t_string::operator=(const char *o)
{
	t_string o2(o);
	swap(o2);
	return *this;
}

void t_string::add_textdomain(const std::string &name, const std::string &path)
{
	LOG_CF << "Binding textdomain " << name << " to path " << path << "\n";

	// Register and (re-)bind this textdomain
	translation::bind_textdomain(name.c_str(), path.c_str(), "UTF-8");
}

void t_string::reset_translations()
{
	++language_counter;
}

std::ostream& operator<<(std::ostream& stream, const t_string_base& string)
{
	stream << string.str();
	return stream;
}
