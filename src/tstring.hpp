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

		void next();
		bool eos() const;
		bool last() const;
		bool translatable() const;
		const std::string& textdomain() const;
		std::string::const_iterator begin() const;
		std::string::const_iterator end() const;
	private:
		void update(void);

		const std::string& string_;
		std::string::size_type begin_;
		std::string::size_type end_;
		std::string textdomain_;
		bool translatable_;
	};

	t_string();
	t_string(const t_string&);
	t_string(const std::string& string);
	t_string(const std::string& string, const std::string& textdomain);
	t_string(const char* string);
	~t_string();

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
	
	bool operator==(const t_string&) const;
	bool operator==(const std::string&) const;
	bool operator==(const char*) const;
	bool operator!=(const t_string&) const;
	bool operator!=(const std::string&) const;
	bool operator!=(const char*) const;

	bool empty() const;
	std::string::size_type size() const;

	operator const std::string&() const;
	const std::string& str() const;
	const char* c_str() const;
	const std::string& value() const;

private:
	bool translatable_;
	std::string value_;
	mutable std::string translated_value_;
};

std::ostream& operator<<(std::ostream&, const t_string&);
bool operator==(const std::string& a, const t_string& b);
bool operator==(const char* a, const t_string& b);
bool operator!=(const std::string& a, const t_string& b);
bool operator!=(const char* a, const t_string& b);
t_string operator+(const std::string& a, const t_string& b);
t_string operator+(const char*, const t_string& b);

#endif

