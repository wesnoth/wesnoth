/* $Id$ */
/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Copyright (C) 2005 - 2007 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file serialization/string_utils.cpp
//! Various string-routines.

#include "global.hpp"

#include <cctype>
#include <sstream>
#include "gettext.hpp"
#include "util.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "wctype.h"

#define ERR_GENERAL LOG_STREAM(err, general)

variable_set::~variable_set()
{
}

static bool two_dots(char a, char b) { return a == '.' && b == '.'; }

static std::string do_interpolation(const std::string &str, const variable_set& set)
{
	std::string res = str;
	// This needs to be able to store negative numbers to check for the while's condition
	// (which is only false when the previous '$' was at index 0)
	int rfind_dollars_sign_from = res.size();
	while(rfind_dollars_sign_from >= 0) {
		// Going in a backwards order allows nested variable-retrieval, e.g. in arrays.
		// For example, "I am $creatures[$i].user_description!"
		const std::string::size_type var_begin_loc = res.rfind('$', rfind_dollars_sign_from);

		// If there are no '$' left then we're done.
		if(var_begin_loc == std::string::npos) {
			break;
		}

		// For the next iteration of the loop, search for more '$'
		// (not from the same place because sometimes the '$' is not replaced)
		rfind_dollars_sign_from = int(var_begin_loc) - 1;


		const std::string::iterator var_begin = res.begin() + var_begin_loc;

		// The '$' is not part of the variable name.
		const std::string::iterator var_name_begin = var_begin + 1;

		// Find the maximum extent of the variable name (it may be shortened later).
		std::string::iterator var_end = var_name_begin;
		for(int bracket_nesting_level = 0; var_end != res.end(); ++var_end) {
			const char c = *var_end;
			if(c == '[') {
				++bracket_nesting_level;
			}
			else if(c == ']') {
				if(--bracket_nesting_level < 0) {
					break;
				}
			}
			else if(!isdigit(c) && !isalpha(c) && c != '.' && c != '_') {
				break;
			}
		}

		// Two dots in a row cannot be part of a valid variable name.
		// That matters for random=, e.g. $x..$y
		var_end = std::adjacent_find(var_name_begin, var_end, two_dots);

		// If the last character is '.', then it can't be a sub-variable.
		// It's probably meant to be a period instead. Don't include it.
		// Would need to do it repetitively if there are multiple '.'s at the end,
		// but don't actually need to do so because the previous check for adjacent '.'s would catch that.
		// For example, "My score is $score." or "My score is $score..."
		if(*(var_end-1) == '.'
		// However, "$array[$i]" by itself does not name a variable,
		// so if "$array[$i]." is encountered, then best to include the '.',
		// so that it more closely follows the syntax of a variable (if only to get rid of all of it).
		// (If it's the script writer's error, they'll have to fix it in either case.)
		// For example in "$array[$i].$field_name", if field_name does not exist as a variable,
		// then the result of the expansion should be "", not "." (which it would be if this exception did not exist).
		&& *(var_end-2) != ']') {
			--var_end;
		}

		const std::string var_name(var_name_begin, var_end);

		if(*var_end == '|') {
			// It's been used to end this variable name; now it has no more effect.
			// This can allow use of things like "$$composite_var_name|.x"
			// (Yes, that's a WML 'pointer' of sorts. They are sometimes useful.)
			// If there should still be a '|' there afterwards to affect other variable names (unlikely),
			// just put another '|' there, one matching each '$', e.g. "$$var_containing_var_name||blah"
			var_end++;
		}


		if (var_name == "") {
			// Allow for a way to have $s in a string.
			// $| will be replaced by $.
			res.replace(var_begin, var_end, "$");
		}
		else {
			// The variable is replaced with its value.
			res.replace(var_begin, var_end,
			    set.get_variable_const(var_name));
		}
	}

	return res;
}

namespace utils {

bool isnewline(char c)
{
	return c == '\r' || c == '\n';
}

// Make sure that we can use Mac, DOS, or Unix style text files on any system
// and they will work, by making sure the definition of whitespace is consistent
bool portable_isspace(char c)
{
	// returns true only on ASCII spaces
	if (static_cast<unsigned char>(c) >= 128)
		return false;
	return isnewline(c) || isspace(c);
}

// Make sure we regard '\r' and '\n' as a space, since Mac, Unix, and DOS
// all consider these differently.
bool notspace(char c)
{
	return !portable_isspace(c);
}

std::string &strip(std::string &str)
{
	// If all the string contains is whitespace,
	// then the whitespace may have meaning, so don't strip it
	std::string::iterator it = std::find_if(str.begin(), str.end(), notspace);
	if (it == str.end())
		return str;

	str.erase(str.begin(), it);
	str.erase(std::find_if(str.rbegin(), str.rend(), notspace).base(), str.end());

	return str;
}

std::vector< std::string > split(std::string const &val, char c, int flags)
{
	std::vector< std::string > res;

	std::string::const_iterator i1 = val.begin();
	std::string::const_iterator i2 = val.begin();

	while (i2 != val.end()) {
		if (*i2 == c) {
			std::string new_val(i1, i2);
			if (flags & STRIP_SPACES)
				strip(new_val);
			if (!(flags & REMOVE_EMPTY) || !new_val.empty())
				res.push_back(new_val);
			++i2;
			if (flags & STRIP_SPACES) {
				while (i2 != val.end() && *i2 == ' ')
					++i2;
			}

			i1 = i2;
		} else {
			++i2;
		}
	}

	std::string new_val(i1, i2);
	if (flags & STRIP_SPACES)
		strip(new_val);
	if (!(flags & REMOVE_EMPTY) || !new_val.empty())
		res.push_back(new_val);

	return res;
}

// Splits a function based either on a separator
// where text within paranthesis is protected from splitting,
// (note that one can use the same character for both the left and right paranthesis)
// or if the separator == 0 it splits a string into an odd number of parts:
// - The part before the first '(',
// - the part between the first '('
// - and the matching right ')', etc ...
// and the remainder of the string.
// Note that this will find the first matching char in the left string
// and match against the corresponding char in the right string.
// In this mode, a correctly processed string should return with
// an odd number of elements to the vector and
// an empty elements are never removed as they are placeholders.
// hence REMOVE EMPTY only works for the separator split.
//
// parenthetical_split("a(b)c{d}e(f{g})h",0,"({",")}") should return
// a vector of <"a","b","c","d","e","f{g}","h">

std::vector< std::string > paranthetical_split(std::string const &val, const char separator, std::string const &left, std::string const &right,int flags)
{
	std::vector< std::string > res;
	std::vector<char> part;

	std::string::const_iterator i1 = val.begin();
	std::string::const_iterator i2 = val.begin();

	std::string lp=left;
	std::string rp=right;

	if(left.size()!=right.size()){
		ERR_GENERAL << "Left and Right Parenthesis lists not same length\n";
		return res;
	}

	while (i2 != val.end()) {
		if(separator && *i2 == separator){
			std::string new_val(i1, i2);
			if (flags & STRIP_SPACES)
				strip(new_val);
			if (!(flags & REMOVE_EMPTY) || !new_val.empty())
				res.push_back(new_val);
			++i2;
			if (flags & STRIP_SPACES) {
				while (i2 != val.end() && *i2 == ' ')
					++i2;
			}
			i1=i2;
			continue;
		}
		if(part.size() && *i2 == part.back()){
			part.pop_back();
			if(!separator && part.size() == 0){
				std::string new_val(i1, i2);
				if (flags & STRIP_SPACES)
					strip(new_val);
				res.push_back(new_val);
				++i2;
				i1=i2;
			}else{
				++i2;
			}
			continue;
		}
		bool found=false;
		for(size_t i=0; i < lp.size(); i++){
			if (*i2 == lp[i]){
				if (!separator && part.size()==0){
					std::string new_val(i1, i2);
					if (flags & STRIP_SPACES)
						strip(new_val);
					res.push_back(new_val);
					++i2;
					i1=i2;
				}else{
					++i2;
				}
				part.push_back(rp[i]);
				found=true;
				break;
			}
		}
		if(!found){
				++i2;
		}
	}

	std::string new_val(i1, i2);
	if (flags & STRIP_SPACES)
		strip(new_val);
	if (!(flags & REMOVE_EMPTY) || !new_val.empty())
		res.push_back(new_val);

	if(part.size()){
			ERR_GENERAL << "Mismatched paranthesis:\n"<<val<<"\n";;
	}

	return res;
}

class string_map_variable_set : public variable_set
{
public:
	string_map_variable_set(const string_map& map) : map_(map) {};

	virtual const t_string& get_variable_const (const std::string& key) const
	{
		static const t_string empty_string = "";

		string_map::const_iterator itor = map_.find(key);
		if(itor == map_.end()) {
			return empty_string;
		} else {
			return itor->second;
		}
	};
private:
	const string_map& map_;

};

std::string interpolate_variables_into_string(const std::string &str, const string_map * const symbols)
{
	string_map_variable_set set(*symbols);
	return do_interpolation(str, set);
}

std::string interpolate_variables_into_string(const std::string &str, const variable_set& variables)
{
	return do_interpolation(str, variables);
}

// Modify a number by string representing integer difference, or optionally %
int apply_modifier( const int number, const std::string &amount, const int minimum ) {
	// wassert( amount.empty() == false );
	int value = atoi(amount.c_str());
	if(amount[amount.size()-1] == '%') {
		value = div100rounded(number * value);
	}
	value += number;
	if (( minimum > 0 ) && ( value < minimum ))
	    value = minimum;
	return value;
}

//! Prepends a configurable set of characters with a backslash
std::string &escape(std::string &str, const std::string& special_chars)
{
	std::string::size_type pos = 0;
	do {
		pos = str.find_first_of(special_chars, pos);
		if (pos == std::string::npos)
			break;
		str.insert(pos, 1, '\\');
		pos += 2;
	} while (pos < str.size());
	return str;
}

//! Prepend all special characters with a backslash.
// Special characters are:
// #@{}+-,\*=
std::string& escape(std::string& str)
{
	static const std::string special_chars("#@{}+-,\\*=");
	return escape(str, special_chars);
}

//! Remove all escape characters (backslash)
std::string &unescape(std::string &str)
{
	std::string::size_type pos = 0;
	do {
		pos = str.find('\\', pos);
		if (pos == std::string::npos)
			break;
		str.erase(pos, 1);
		++pos;
	} while (pos < str.size());
	return str;
}

bool string_bool(const std::string& str,bool def)
{
	if(str != "") {
		if(str == "yes" || str == "on" || str == "true" || lexical_cast_default<int>(str)) {
			return true;
		}
		if(str == "no" || str == "off" || str == "false" || !lexical_cast_default<int>(str)) {
			return false;
		}
	}
	return def;
}

//! Check if the username is valid
//! (all alpha-numeric plus underscore)
bool isvalid_username(const std::string& username)
{
	const size_t alnum = std::count_if(username.begin(),username.end(),isalnum);
	const size_t underscore = std::count(username.begin(),username.end(),'_');
	if((alnum + underscore != username.size()) || underscore == username.size() || username.empty() ) {
		return false;
	}
	return true;
}

std::string join(std::vector< std::string > const &v, char c)
{
	std::stringstream str;
	for(std::vector< std::string >::const_iterator i = v.begin(); i != v.end(); ++i) {
		str << *i;
		if (i + 1 != v.end())
			str << c;
	}

	return str.str();
}

// This function is identical to split(), except it does not split
// when it otherwise would if the previous character was identical to the parameter 'quote'.
// i.e. it does not split quoted commas.
// This method was added to make it possible to quote user input,
// particularly so commas in user input will not cause visual problems in menus.
//
//! @todo Why not change split()? That would change the methods post condition.
std::vector< std::string > quoted_split(std::string const &val, char c, int flags, char quote)
{
	std::vector<std::string> res;

	std::string::const_iterator i1 = val.begin();
	std::string::const_iterator i2 = val.begin();

	while (i2 != val.end()) {
		if (*i2 == quote) {
			// Ignore quoted character
			++i2;
			if (i2 != val.end()) ++i2;
		} else if (*i2 == c) {
			std::string new_val(i1, i2);
			if (flags & STRIP_SPACES)
				strip(new_val);
			if (!(flags & REMOVE_EMPTY) || !new_val.empty())
				res.push_back(new_val);
			++i2;
			if (flags & STRIP_SPACES) {
				while(i2 != val.end() && *i2 == ' ')
					++i2;
			}

			i1 = i2;
		} else {
			++i2;
		}
	}

	std::string new_val(i1, i2);
	if (flags & STRIP_SPACES)
		strip(new_val);
	if (!(flags & REMOVE_EMPTY) || !new_val.empty())
		res.push_back(new_val);

	return res;
}

std::pair< int, int > parse_range(std::string const &str)
{
	const std::string::const_iterator dash = std::find(str.begin(), str.end(), '-');
	const std::string a(str.begin(), dash);
	const std::string b = dash != str.end() ? std::string(dash + 1, str.end()) : a;
	std::pair<int,int> res(atoi(a.c_str()), atoi(b.c_str()));
	if (res.second < res.first)
		res.second = res.first;

	return res;
}

std::vector< std::pair< int, int > > parse_ranges(std::string const &str)
{
	std::vector< std::pair< int, int > > to_return;
	std::vector<std::string> strs = utils::split(str);
	std::vector<std::string>::const_iterator i, i_end=strs.end();
	for(i = strs.begin(); i != i_end; ++i) {
		to_return.push_back(parse_range(*i));
	}
	return to_return;
}

int byte_size_from_utf8_first(unsigned char ch)
{
	int count;

	if ((ch & 0x80) == 0)
		count = 1;
	else if ((ch & 0xE0) == 0xC0)
		count = 2;
	else if ((ch & 0xF0) == 0xE0)
		count = 3;
	else if ((ch & 0xF8) == 0xF0)
		count = 4;
	else if ((ch & 0xFC) == 0xF8)
		count = 5;
	else if ((ch & 0xFE) == 0xFC)
		count = 6;
	else
		throw invalid_utf8_exception(); // Stop on invalid characters

	return count;
}

utf8_iterator::utf8_iterator(const std::string& str)
{
	current_substr.first = str.begin();
	string_end = str.end();
	update();
}

utf8_iterator::utf8_iterator(std::string::const_iterator const &beg, std::string::const_iterator const &end)
{
	current_substr.first = beg;
	string_end = end;
	update();
}

utf8_iterator utf8_iterator::begin(std::string const &str)
{
	return utf8_iterator(str.begin(), str.end());
}

utf8_iterator utf8_iterator::end(const std::string& str)
{
	return utf8_iterator(str.end(), str.end());
}

bool utf8_iterator::operator==(const utf8_iterator& a) const
{
	return current_substr.first == a.current_substr.first;
}

utf8_iterator& utf8_iterator::operator++()
{
	current_substr.first = current_substr.second;
	update();
	return *this;
}

wchar_t utf8_iterator::operator*() const
{
	return current_char;
}

bool utf8_iterator::next_is_end()
{
	if(current_substr.second == string_end)
		return true;
	return false;
}

const std::pair<std::string::const_iterator, std::string::const_iterator>& utf8_iterator::substr() const
{
	return current_substr;
}

void utf8_iterator::update()
{
	// Do not try to update the current unicode char at end-of-string.
	if(current_substr.first == string_end)
		return;

	size_t size = byte_size_from_utf8_first(*current_substr.first);
	current_substr.second = current_substr.first + size;

	current_char = static_cast<unsigned char>(*current_substr.first);

	// Convert the first character
	if(size != 1) {
		current_char &= 0xFF >> (size + 1);
	}

	// Convert the continuation bytes
	for(std::string::const_iterator c = current_substr.first+1;
			c != current_substr.second; ++c) {
		// If the string ends occurs within an UTF8-sequence, this is bad.
		if (c == string_end)
			throw invalid_utf8_exception();

		if ((*c & 0xC0) != 0x80)
			throw invalid_utf8_exception();

		current_char = (current_char << 6) | (static_cast<unsigned char>(*c) & 0x3F);
	}
}


std::string wstring_to_string(const wide_string &src)
{
	wchar_t ch;
	wide_string::const_iterator i;
	Uint32 bitmask;
	std::string ret;

	try {

		for(i = src.begin(); i != src.end(); ++i) {
			unsigned int count;
			ch = *i;

			// Determine the bytes required
			count = 1;
			if(ch >= 0x80)
				count++;

			bitmask = 0x800;
			for(unsigned int j = 0; j < 5; ++j) {
				if(static_cast<Uint32>(ch) >= bitmask) {
					count++;
				}

				bitmask <<= 5;
			}

			if(count > 6) {
				throw invalid_utf8_exception();
			}

			if(count == 1) {
				push_back(ret,static_cast<char>(ch));
			} else {
				for(int j = static_cast<int>(count) - 1; j >= 0; --j) {
					unsigned char c = (ch >> (6 * j)) & 0x3f;
					c |= 0x80;
					if(j == static_cast<int>(count) - 1) {
						c |= 0xff << (8 - count);
					}
					push_back(ret, c);
				}
			}

		}

		return ret;
	}
	catch(invalid_utf8_exception e) {
		ERR_GENERAL << "Invalid wide character string\n";
		return ret;
	}
}

std::string wchar_to_string(const wchar_t c)
{
	wide_string s;
	s.push_back(c);
	return wstring_to_string(s);
}

wide_string string_to_wstring(const std::string &src)
{
	wide_string res;

	try {
		utf8_iterator i1(src);
		const utf8_iterator i2(utf8_iterator::end(src));

		// Equivalent to res.insert(res.end(),i1,i2) which doesn't work on VC++6.
		while(i1 != i2) {
			push_back(res,*i1);
			++i1;
		}
	}
	catch(invalid_utf8_exception e) {
		ERR_GENERAL << "Invalid UTF-8 string: \"" << src << "\"\n";
		return res;
	}

	return res;
}

utf8_string capitalize(const utf8_string& s)
{
	if(s.size() > 0) {
		utf8_iterator itor(s);
#if defined(__APPLE__) || defined(__AMIGAOS4__)
		//! @todo FIXME: Should we support towupper on recent OSX platforms?
		wchar_t uchar = *itor;
		if(uchar >= 0 && uchar < 0x100)
			uchar = toupper(uchar);
		std::string res = utils::wchar_to_string(uchar);
#else
		std::string res = utils::wchar_to_string(towupper(*itor));
#endif
		res.append(itor.substr().second, s.end());
		return res;
	}
	return s;
}

utf8_string uppercase(const utf8_string& s)
{
	if(s.size() > 0) {
		utf8_iterator itor(s);
		std::string res;

		for(;itor != utf8_iterator::end(s); ++itor) {
#if defined(__APPLE__) || defined(__AMIGAOS4__)
			//! @todo FIXME: Should we support towupper on recent OSX platforms?
			wchar_t uchar = *itor;
			if(uchar >= 0 && uchar < 0x100)
				uchar = toupper(uchar);
			res += utils::wchar_to_string(uchar);
#else
			res += utils::wchar_to_string(towupper(*itor));
#endif
		}

		return res;
	}
	return s;
}

utf8_string lowercase(const utf8_string& s)
{
	if(s.size() > 0) {
		utf8_iterator itor(s);
		std::string res;

		for(;itor != utf8_iterator::end(s); ++itor) {
#if defined(__APPLE__) || defined(__OpenBSD__) || defined(__AMIGAOS4__)
			//! @todo FIXME: Should we support towupper on recent OSX platforms?
			wchar_t uchar = *itor;
			if(uchar >= 0 && uchar < 0x100)
				uchar = tolower(uchar);
			res += utils::wchar_to_string(uchar);
#else
			res += utils::wchar_to_string(towlower(*itor));
#endif
		}

		res.append(itor.substr().second, s.end());
		return res;
	}
	return s;
}

//! Truncates a string.
//!
//! If the string send has more than size utf-8 characters it will be truncated
//! to this size. No assumptions can be made about the actual size of the 
//! string.
//! 
//! @param str[in]      String which can be converted to utf-8.
//! @param str[out]     String which contains maximal size utf-8 characters.
//! @param size         The size to truncate at.
void truncate_as_wstring(std::string& str, const size_t size)
{
	wide_string utf8_str = utils::string_to_wstring(str);
	if(utf8_str.size() > size) {
		utf8_str.resize(size);
		str = utils::wstring_to_string(utf8_str);
	}
}

// match using '*' as any number of characters (including none), and '?' as any one character
bool wildcard_string_match(const std::string& str, const std::string& match)
{
	const bool wild_matching = (!match.empty() && match[0] == '*');
	const std::string::size_type solid_begin = match.find_first_not_of('*');
	const bool have_solids = (solid_begin != std::string::npos);
	//check the simple case first
	if(str.empty() || !have_solids) {
		return wild_matching || str == match;
	}
	const std::string::size_type solid_end = match.find_first_of('*', solid_begin);
	const std::string::size_type solid_len = (solid_end == std::string::npos)
		? match.length() - solid_begin : solid_end - solid_begin;
	std::string::size_type current = 0;
	bool matches;
	do {
		matches = true;
		//now try to place the str into the solid space
		const std::string::size_type test_len = str.length() - current;
		for(std::string::size_type i=0; i < solid_len && matches; ++i) {
			char solid_c = match[solid_begin + i];
			if(i > test_len || !(solid_c == '?' || solid_c == str[current+i])) {
				matches = false;
			}
		}
		if(matches) {
			//the solid space matched, now consume it and attempt to find more
			const std::string consumed_match = (solid_begin+solid_len < match.length())
				? match.substr(solid_end) : "";
			const std::string consumed_str = (solid_len < test_len)
				? str.substr(current+solid_len) : "";
			matches = wildcard_string_match(consumed_str, consumed_match);
		}
	} while(wild_matching && !matches && ++current < str.length());
	return matches;
}


} // end namespace utils

std::string vgettext(const char *msgid, const utils::string_map& symbols)
{
	const std::string orig(_(msgid));
	const std::string msg = utils::interpolate_variables_into_string(orig, &symbols);
	return msg;
}

std::string vngettext(const char* sing, const char* plur, int n, const utils::string_map& symbols)
{
	const std::string orig(_n(sing, plur, n));
	const std::string msg = utils::interpolate_variables_into_string(orig, &symbols);
	return msg;
}
