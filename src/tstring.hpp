/* $Id$ */
/*
   Copyright (C) 2004 - 2008 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef TSTRING_H_INCLUDED
#define TSTRING_H_INCLUDED

#include <string>

class t_string
{
public:
	class walker
	{
	public:
		walker(const t_string& string);
		walker(const std::string&);

		void next()                               { begin_ = end_; update(); }
		bool eos() const                          { return begin_ == string_.size(); }
		bool last() const                         { return end_ == string_.size(); }
		bool translatable() const                 { return translatable_; }
		const std::string& textdomain() const     { return textdomain_; }
		std::string::const_iterator begin() const { return string_.begin() + begin_; }
		std::string::const_iterator end() const   { return string_.begin() + end_; }
	private:
		void update(void);

		const std::string& string_;
		std::string::size_type begin_;
		std::string::size_type end_;
		std::string textdomain_;
		bool translatable_;
	};

	friend class walker;

	t_string();
	t_string(const t_string&);
	t_string(const std::string& string);
	t_string(const std::string& string, const std::string& textdomain);
	t_string(const char* string);

	static t_string from_serialized(const std::string& string);
	std::string to_serialized() const;

	t_string& operator=(const t_string&);
	t_string& operator=(const std::string&);
	t_string& operator=(const char*);

	t_string operator+(const t_string&) const;
	t_string operator+(const std::string&) const;
	t_string operator+(const char*) const;

	t_string& operator+=(const t_string&);
	t_string& operator+=(const std::string&);
	t_string& operator+=(const char*);

	bool operator==(const t_string& string) const    { return string.translatable_ == translatable_ && string.value_ == value_; }
	bool operator==(const std::string& string) const { return !translatable_ && value_ == string; }
	bool operator==(const char* string) const        { return !translatable_ && value_ == string; }
	bool operator!=(const t_string& string) const    { return !(*this == string); }
	bool operator!=(const std::string& string) const { return !(*this == string); }
	bool operator!=(const char* string) const        { return !(*this == string); }

	bool operator<(const t_string& string) const     { return value_ < string.value_; }

	bool empty() const                               { return value_.empty(); }
	std::string::size_type size() const              { return str().size(); }

	operator const std::string&() const              { return str(); }
	const std::string& str() const;
	const char* c_str() const                        { return str().c_str(); }

	// Warning: value() may contain platform dependant prefix bytes !
	// Consider base_str() for a more reliable untranslated string
	const std::string& value() const                 { return value_; }
	const std::string base_str() const;

	void reset_translation() const                   { translated_value_ = ""; }

	static void add_textdomain(const std::string& name, const std::string& path);
private:
	std::string value_;
	mutable std::string translated_value_;
	bool translatable_, last_untranslatable_;
};

std::ostream& operator<<(std::ostream&, const t_string&);
inline bool operator==(const std::string& a, const t_string& b)    { return a == b.str(); }
inline bool operator==(const char* a, const t_string& b)           { return b == a; }
inline bool operator!=(const std::string& a, const t_string& b)    { return a != b.str(); }
inline bool operator!=(const char* a, const t_string& b)           { return b != a; }
inline t_string operator+(const std::string& a, const t_string& b) { return t_string(a + b.str()); }
inline t_string operator+(const char* a, const t_string& b)        { return t_string(a) + b; }

#endif

