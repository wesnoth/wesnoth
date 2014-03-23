/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Copyright (C) 2005 - 2014 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Various string-routines.
 */

#include "global.hpp"

#include "gettext.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "util.hpp"
#include <limits>
#include <boost/array.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

static lg::log_domain log_engine("engine");
#define ERR_GENERAL LOG_STREAM(err, lg::general)
#define ERR_NG LOG_STREAM(err, log_engine)

namespace {
size_t byte_size_from_ucs4_codepoint(ucs4::char_t ch)
{
	if(ch < (1u << 7))
		return 1;
	else if(ch < (1u << 11))
		return 2;
	else if(ch < (1u << 16))
		return 3;
	else if(ch < (1u << 21))
		return 4;
	else if(ch < (1u << 26))
		return 5;
	else if(ch < (1u << 31))
		return 6;
	else
		throw utf8::invalid_utf8_exception(); // Invalid UCS-4
}
} // anonymous namespace

namespace utils {

const std::string ellipsis = "...";

const std::string unicode_minus = "−";
const std::string unicode_en_dash = "–";
const std::string unicode_em_dash = "—";
const std::string unicode_figure_dash = "‒";
const std::string unicode_multiplication_sign = "×";
const std::string unicode_bullet = "•";

bool isnewline(const char c)
{
	return c == '\r' || c == '\n';
}

// Make sure that we can use Mac, DOS, or Unix style text files on any system
// and they will work, by making sure the definition of whitespace is consistent
bool portable_isspace(const char c)
{
	// returns true only on ASCII spaces
	if (static_cast<unsigned char>(c) >= 128)
		return false;
	return isnewline(c) || isspace(c);
}

// Make sure we regard '\r' and '\n' as a space, since Mac, Unix, and DOS
// all consider these differently.
bool notspace(const char c)
{
	return !portable_isspace(c);
}

std::string replace(std::string str, const std::string &src, const std::string &dst)
{
	std::string::size_type pos = 0;
	while ( (pos = str.find(src, pos)) != std::string::npos ) {
		str.replace( pos, src.size(), dst );
		pos++;
	}
	return str;
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

std::string &strip_end(std::string &str)
{
	str.erase(std::find_if(str.rbegin(), str.rend(), notspace).base(), str.end());

	return str;
}

/**
 * Splits a (comma-)separated string into a vector of pieces.
 * @param[in]  val    A (comma-)separated string.
 * @param[in]  c      The separator character (usually a comma).
 * @param[in]  flags  Flags controlling how the split is done.
 *                    This is a bit field with two settings (both on by default):
 *                    REMOVE_EMPTY causes empty pieces to be skipped/removed.
 *                    STRIP_SPACES causes the leading and trailing spaces of each piece to be ignored/stripped.
 */
std::vector< std::string > split(std::string const &val, const char c, const int flags)
{
	std::vector< std::string > res;

	std::string::const_iterator i1 = val.begin();
	std::string::const_iterator i2;
	if (flags & STRIP_SPACES) {
		while (i1 != val.end() && portable_isspace(*i1))
			++i1;
	}
	i2=i1;

	while (i2 != val.end()) {
		if (*i2 == c) {
			std::string new_val(i1, i2);
			if (flags & STRIP_SPACES)
				strip_end(new_val);
			if (!(flags & REMOVE_EMPTY) || !new_val.empty())
				res.push_back(new_val);
			++i2;
			if (flags & STRIP_SPACES) {
				while (i2 != val.end() && portable_isspace(*i2))
					++i2;
			}

			i1 = i2;
		} else {
			++i2;
		}
	}

	std::string new_val(i1, i2);
	if (flags & STRIP_SPACES)
		strip_end(new_val);
	if (!(flags & REMOVE_EMPTY) || !new_val.empty())
		res.push_back(new_val);

	return res;
}

std::vector< std::string > square_parenthetical_split(std::string const &val,
		const char separator, std::string const &left,
		std::string const &right,const int flags)
{
	std::vector< std::string > res;
	std::vector<char> part;
	bool in_parenthesis = false;
	std::vector<std::string::const_iterator> square_left;
	std::vector<std::string::const_iterator> square_right;
	std::vector< std::string > square_expansion;

	std::string lp=left;
	std::string rp=right;

	std::string::const_iterator i1 = val.begin();
	std::string::const_iterator i2;
	std::string::const_iterator j1;
	if (flags & STRIP_SPACES) {
		while (i1 != val.end() && portable_isspace(*i1))
			++i1;
	}
	i2=i1;
	j1=i1;

	if (i1 == val.end()) return res;

	if (!separator) {
		ERR_GENERAL << "Separator must be specified for square bracket split funtion.\n";
		return res;
	}

	if(left.size()!=right.size()){
		ERR_GENERAL << "Left and Right Parenthesis lists not same length\n";
		return res;
	}

	while (true) {
		if(i2 == val.end() || (!in_parenthesis && *i2 == separator)) {
			//push back square contents
			size_t size_square_exp = 0;
			for (size_t i=0; i < square_left.size(); i++) {
				std::string tmp_val(square_left[i]+1,square_right[i]);
				std::vector< std::string > tmp = split(tmp_val);
				std::vector<std::string>::const_iterator itor = tmp.begin();
				for(; itor != tmp.end(); ++itor) {
					size_t found_tilde = (*itor).find_first_of('~');
					if (found_tilde == std::string::npos) {
						size_t found_asterisk = (*itor).find_first_of('*');
						if (found_asterisk == std::string::npos) {
							std::string tmp = (*itor);
							square_expansion.push_back(strip(tmp));
						}
						else { //'*' multiple expansion
							std::string s_begin = (*itor).substr(0,found_asterisk);
							s_begin = strip(s_begin);
							std::string s_end = (*itor).substr(found_asterisk+1);
							s_end = strip(s_end);
							for (int ast=atoi(s_end.c_str()); ast>0; --ast)
								square_expansion.push_back(s_begin);
						}
					}
					else { //expand number range
						std::string s_begin = (*itor).substr(0,found_tilde);
						s_begin = strip(s_begin);
						int begin = atoi(s_begin.c_str());
						size_t padding = 0, padding_end = 0;
						while (padding<s_begin.size() && s_begin[padding]=='0') {
							padding++;
						}
						std::string s_end = (*itor).substr(found_tilde+1);
						s_end = strip(s_end);
						int end = atoi(s_end.c_str());
						while (padding_end<s_end.size() && s_end[padding_end]=='0') {
							padding_end++;
						}
						if (padding*padding_end > 0 && s_begin.size() != s_end.size()) {
							ERR_GENERAL << "Square bracket padding sizes not matching: "
										<< s_begin << " and " << s_end <<".\n";
						}
						if (padding_end > padding) padding = padding_end;
						
						int increment = (end >= begin ? 1 : -1);
						end+=increment; //include end in expansion
						for (int k=begin; k!=end; k+=increment) {
							std::string pb = boost::lexical_cast<std::string>(k);
							for (size_t p=pb.size(); p<=padding; p++)
								pb = std::string("0") + pb;
							square_expansion.push_back(pb);
						}
					}
				}
				if (i*square_expansion.size() != (i+1)*size_square_exp ) {
					std::string tmp(i1, i2);
					ERR_GENERAL << "Square bracket lengths do not match up: "+tmp+"\n";
					return res;
				}
				size_square_exp = square_expansion.size();
			}

			//combine square contents and rest of string for comma zone block
			size_t j = 0;
			size_t j_max = 0;
			if (square_left.size() != 0)
				j_max = square_expansion.size() / square_left.size();
			do {
				j1 = i1;
				std::string new_val;
				for (size_t i=0; i < square_left.size(); i++) {
					std::string tmp_val(j1, square_left[i]);
					new_val.append(tmp_val);
					size_t k = j+i*j_max;
					if (k < square_expansion.size())
						new_val.append(square_expansion[k]);
					j1 = square_right[i]+1;
				}
				std::string tmp_val(j1, i2);
				new_val.append(tmp_val);
				if (flags & STRIP_SPACES)
					strip_end(new_val);
				if (!(flags & REMOVE_EMPTY) || !new_val.empty())
					res.push_back(new_val);
				j++;
			} while (j<j_max);

			if (i2 == val.end()) //escape loop
				break;
			++i2;
			if (flags & STRIP_SPACES) { //strip leading spaces
				while (i2 != val.end() && portable_isspace(*i2))
					++i2;
			}
			i1=i2;
			square_left.clear();
			square_right.clear();
			square_expansion.clear();
			continue;
		}
		if(!part.empty() && *i2 == part.back()) {
			part.pop_back();
			if (*i2 == ']') square_right.push_back(i2);
			if (part.empty())
				in_parenthesis = false;
			++i2;
			continue;
		}
		bool found=false;
		for(size_t i=0; i < lp.size(); i++) {
			if (*i2 == lp[i]){
				if (*i2 == '[')
					square_left.push_back(i2);
				++i2;
				part.push_back(rp[i]);
				found=true;
				break;
			}
		}
		if(!found){
			++i2;
		} else
			in_parenthesis = true;
	}

	if(!part.empty()){
			ERR_GENERAL << "Mismatched parenthesis:\n"<<val<<"\n";;
	}

	return res;
}

std::map< std::string, std::string > map_split(
		  std::string const &val
		, char major
		, char minor
		, int flags
		, std::string const& default_value)
{
	//first split by major so that we get a vector with the key-value pairs
	std::vector< std::string > v = split(val, major, flags);

	//now split by minor to extract keys and values
	std::map< std::string, std::string > res;

	for( std::vector< std::string >::iterator i = v.begin(); i != v.end(); ++i) {
		size_t pos = i->find_first_of(minor);
		std::string key, value;

		if(pos == std::string::npos) {
			key = (*i);
			value = default_value;
		} else {
			key = i->substr(0, pos);
			value = i->substr(pos + 1);
		}

		res[key] = value;
	}

	return res;
}

std::vector< std::string > parenthetical_split(std::string const &val,
		const char separator, std::string const &left,
		std::string const &right,const int flags)
{
	std::vector< std::string > res;
	std::vector<char> part;
	bool in_parenthesis = false;

	std::string lp=left;
	std::string rp=right;

	std::string::const_iterator i1 = val.begin();
	std::string::const_iterator i2;
	if (flags & STRIP_SPACES) {
		while (i1 != val.end() && portable_isspace(*i1))
			++i1;
	}
	i2=i1;

	if(left.size()!=right.size()){
		ERR_GENERAL << "Left and Right Parenthesis lists not same length\n";
		return res;
	}

	while (i2 != val.end()) {
		if(!in_parenthesis && separator && *i2 == separator){
			std::string new_val(i1, i2);
			if (flags & STRIP_SPACES)
				strip_end(new_val);
			if (!(flags & REMOVE_EMPTY) || !new_val.empty())
				res.push_back(new_val);
			++i2;
			if (flags & STRIP_SPACES) {
				while (i2 != val.end() && portable_isspace(*i2))
					++i2;
			}
			i1=i2;
			continue;
		}
		if(!part.empty() && *i2 == part.back()){
			part.pop_back();
			if(!separator && part.empty()){
				std::string new_val(i1, i2);
				if (flags & STRIP_SPACES)
					strip(new_val);
				res.push_back(new_val);
				++i2;
				i1=i2;
			}else{
				if (part.empty())
					in_parenthesis = false;
				++i2;
			}
			continue;
		}
		bool found=false;
		for(size_t i=0; i < lp.size(); i++){
			if (*i2 == lp[i]){
				if (!separator && part.empty()){
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
		} else
			in_parenthesis = true;
	}

	std::string new_val(i1, i2);
	if (flags & STRIP_SPACES)
		strip(new_val);
	if (!(flags & REMOVE_EMPTY) || !new_val.empty())
		res.push_back(new_val);

	if(!part.empty()){
			ERR_GENERAL << "Mismatched parenthesis:\n"<<val<<"\n";;
	}

	return res;
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

std::string escape(const std::string &str, const char *special_chars)
{
	std::string::size_type pos = str.find_first_of(special_chars);
	if (pos == std::string::npos) {
		// Fast path, possibly involving only reference counting.
		return str;
	}
	std::string res = str;
	do {
		res.insert(pos, 1, '\\');
		pos = res.find_first_of(special_chars, pos + 2);
	} while (pos != std::string::npos);
	return res;
}

std::string unescape(const std::string &str)
{
	std::string::size_type pos = str.find('\\');
	if (pos == std::string::npos) {
		// Fast path, possibly involving only reference counting.
		return str;
	}
	std::string res = str;
	do {
		res.erase(pos, 1);
		pos = res.find('\\', pos + 1);
	} while (pos != std::string::npos);
	return str;
}

std::string urlencode(const std::string &str)
{
	static std::string nonresv =
		"-."
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"_"
		"abcdefghijklmnopqrstuvwxyz"
		"~";

	std::string res;

	for(size_t n = 0; n < str.length(); ++n) {
		const char& c = str[n];

		if(nonresv.find(c) != std::string::npos) {
			res += c;
			continue;
		}

		char buf[4];
		snprintf(buf, sizeof(buf), "%%%02X", c);
		res += buf;
	}

	return res;
}

bool string_bool(const std::string& str, bool def) {
	if (str.empty()) return def;

	// yes/no is the standard, test it first
	if (str == "yes") return true;
	if (str == "no"|| str == "false" || str == "off" || str == "0" || str == "0.0")
		return false;

	// all other non-empty string are considered as true
	return true;
}

std::string signed_value(int val)
{
	std::ostringstream oss;
	oss << (val >= 0 ? "+" : unicode_minus) << abs(val);
	return oss.str();
}

std::string half_signed_value(int val)
{
	std::ostringstream oss;
	if (val < 0)
		oss << unicode_minus;
	oss << abs(val);
	return oss.str();
}

static void si_string_impl_stream_write(std::stringstream &ss, double input) {
	std::streamsize oldprec = ss.precision();
#ifdef _MSC_VER
	// Visual C++ makes 'precision' set the number of decimal places.
	// Other platforms make it set the number of significant figures
	ss.precision(1);
	ss << std::fixed
	   << input;
#else
	// Workaround to display 1023 KiB instead of 1.02e3 KiB
	if (input >= 1000)
		ss.precision(4);
	else
		ss.precision(3);
	ss << input;
#endif
	ss.precision(oldprec);
}

std::string si_string(double input, bool base2, std::string unit) {
	const double multiplier = base2 ? 1024 : 1000;

	typedef boost::array<std::string, 9> strings9;

	strings9 prefixes;
	strings9::const_iterator prefix;
	if (input < 1.0) {
		strings9 tmp = { {
			"",
			_("prefix_milli^m"),
			_("prefix_micro^µ"),
			_("prefix_nano^n"),
			_("prefix_pico^p"),
			_("prefix_femto^f"),
			_("prefix_atto^a"),
			_("prefix_zepto^z"),
			_("prefix_yocto^y")
		} };
		prefixes = tmp;
		prefix = prefixes.begin();
		while (input < 1.0  && *prefix != prefixes.back()) {
			input *= multiplier;
			++prefix;
		}
	} else {
		strings9 tmp = { {
			"",
			(base2 ?
				_("prefix_kibi^K") :
				_("prefix_kilo^k")
			),
			_("prefix_mega^M"),
			_("prefix_giga^G"),
			_("prefix_tera^T"),
			_("prefix_peta^P"),
			_("prefix_exa^E"),
			_("prefix_zetta^Z"),
			_("prefix_yotta^Y")
		} };
		prefixes = tmp;
		prefix = prefixes.begin();
		while (input > multiplier && *prefix != prefixes.back()) {
			input /= multiplier;
			++prefix;
		}
	}

	std::stringstream ss;
	si_string_impl_stream_write(ss, input);
	ss << ' '
	   << *prefix
	   << (base2 && (*prefix != "") ? _("infix_binary^i") : "")
	   << unit;
	return ss.str();
}

static bool is_username_char(char c) {
	return ((c == '_') || (c == '-'));
}

static bool is_wildcard_char(char c) {
    return ((c == '?') || (c == '*'));
}

bool isvalid_username(const std::string& username) {
	const size_t alnum = std::count_if(username.begin(), username.end(), isalnum);
	const size_t valid_char =
			std::count_if(username.begin(), username.end(), is_username_char);
	if ((alnum + valid_char != username.size())
			|| valid_char == username.size() || username.empty() )
	{
		return false;
	}
	return true;
}

bool isvalid_wildcard(const std::string& username) {
    const size_t alnum = std::count_if(username.begin(), username.end(), isalnum);
	const size_t valid_char =
			std::count_if(username.begin(), username.end(), is_username_char);
    const size_t wild_char =
            std::count_if(username.begin(), username.end(), is_wildcard_char);
	if ((alnum + valid_char + wild_char != username.size())
			|| valid_char == username.size() || username.empty() )
	{
		return false;
	}
	return true;
}


bool word_completion(std::string& text, std::vector<std::string>& wordlist) {
	std::vector<std::string> matches;
	const size_t last_space = text.rfind(" ");
	// If last character is a space return.
	if (last_space == text.size() -1) {
		wordlist = matches;
		return false;
	}

	bool text_start;
	std::string semiword;
	if (last_space == std::string::npos) {
		text_start = true;
		semiword = text;
	} else {
		text_start = false;
		semiword.assign(text, last_space + 1, text.size());
	}

	std::string best_match = semiword;
	for (std::vector<std::string>::const_iterator word = wordlist.begin();
			word != wordlist.end(); ++word)
	{
		if (word->size() < semiword.size()
		|| !std::equal(semiword.begin(), semiword.end(), word->begin(),
				chars_equal_insensitive))
		{
			continue;
		}
		if (matches.empty()) {
			best_match = *word;
		} else {
			int j = 0;
			while (toupper(best_match[j]) == toupper((*word)[j])) j++;
			if (best_match.begin() + j < best_match.end()) {
				best_match.erase(best_match.begin() + j, best_match.end());
			}
		}
		matches.push_back(*word);
	}
	if(!matches.empty()) {
		text.replace(last_space + 1, best_match.size(), best_match);
	}
	wordlist = matches;
	return text_start;
}

static bool is_word_boundary(char c) {
	return (c == ' ' || c == ',' || c == ':' || c == '\'' || c == '"' || c == '-');
}

bool word_match(const std::string& message, const std::string& word) {
	size_t first = message.find(word);
	if (first == std::string::npos) return false;
	if (first == 0 || is_word_boundary(message[first - 1])) {
		size_t next = first + word.size();
		if (next == message.size() || is_word_boundary(message[next])) {
			return true;
		}
	}
	return false;
}

bool wildcard_string_match(const std::string& str, const std::string& match) {
	const bool wild_matching = (!match.empty() && match[0] == '*');
	const std::string::size_type solid_begin = match.find_first_not_of('*');
	const bool have_solids = (solid_begin != std::string::npos);
	// Check the simple case first
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
		// Now try to place the str into the solid space
		const std::string::size_type test_len = str.length() - current;
		for(std::string::size_type i=0; i < solid_len && matches; ++i) {
			char solid_c = match[solid_begin + i];
			if(i > test_len || !(solid_c == '?' || solid_c == str[current+i])) {
				matches = false;
			}
		}
		if(matches) {
			// The solid space matched, now consume it and attempt to find more
			const std::string consumed_match = (solid_begin+solid_len < match.length())
				? match.substr(solid_end) : "";
			const std::string consumed_str = (solid_len < test_len)
				? str.substr(current+solid_len) : "";
			matches = wildcard_string_match(consumed_str, consumed_match);
		}
	} while(wild_matching && !matches && ++current < str.length());
	return matches;
}

std::string indent(const std::string& string, size_t indent_size)
{
	if(indent_size == 0) {
		return string;
	}

	const std::string indent(indent_size, ' ');

	if(string.empty()) {
		return indent;
	}

	const std::vector<std::string>& lines = split(string, '\x0A', 0);
	std::string res;

	for(size_t lno = 0; lno < lines.size();) {
		const std::string& line = lines[lno];

		// Lines containing only a carriage return count as empty.
		if(!line.empty() && line != "\x0D") {
			res += indent;
		}

		res += line;

		if(++lno < lines.size()) {
			res += '\x0A';
		}
	}

	return res;
}

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

std::string ucs4string_to_string(const ucs4::string &src)
{
	std::string ret;

	try {
		for(ucs4::string::const_iterator i = src.begin(); i != src.end(); ++i) {
			unsigned int count;
			ucs4::char_t ch = *i;

			// Determine the bytes required
			count = byte_size_from_ucs4_codepoint(ch);

			if(count == 1) {
				ret.push_back(static_cast<char>(ch));
			} else {
				for(int j = static_cast<int>(count) - 1; j >= 0; --j) {
					unsigned char c = (ch >> (6 * j)) & 0x3f;
					c |= 0x80;
					if(j == static_cast<int>(count) - 1) {
						c |= 0xff << (8 - count);
					}
					ret.push_back(c);
				}
			}

		}

		return ret;
	}
	catch(utf8::invalid_utf8_exception&) {
		ERR_GENERAL << "Invalid UCS-4 character string\n";
		return ret;
	}
}

std::string ucs4char_to_string(const ucs4::char_t c)
{
	ucs4::string s;
	s.push_back(c);
	return ucs4string_to_string(s);
}

ucs4::string string_to_ucs4string(const std::string &src)
{
	ucs4::string res;

	try {
		utf8::iterator i1(src);
		const utf8::iterator i2(utf8::iterator::end(src));

		// Equivalent to res.insert(res.end(),i1,i2) which doesn't work on VC++6.
		while(i1 != i2) {
			res.push_back(*i1);
			++i1;
		}
	}
	catch(utf8::invalid_utf8_exception&) {
		ERR_GENERAL << "Invalid UTF-8 string: \"" << src << "\"\n";
		return res;
	}

	return res;
}

utf16::string ucs4string_to_utf16string(const ucs4::string &src)
{
	utf16::string res;
	const Uint32 bit17 = 0x10000;
	BOOST_FOREACH(const ucs4::char_t &u4, src) {
		if(u4 < bit17)
			res.push_back(static_cast<wchar_t>(u4));
		else {
			const Uint32 char20 = u4 - bit17;
			assert(char20 < (1 << 20));
			const ucs4::char_t lead = 0xD800 + (char20 >> 10);
			const ucs4::char_t trail = 0xDC00 + (char20 & 0x3FF);
			assert(lead < bit17);
			assert(trail < bit17);
			res.push_back(static_cast<wchar_t>(lead));
			res.push_back(static_cast<wchar_t>(trail));
		}
	}

	return res;
}

void truncate_as_ucs4string(std::string& str, const size_t size)
{
	ucs4::string u4_str = utils::string_to_ucs4string(str);
	if(u4_str.size() > size) {
		u4_str.resize(size);
		str = utils::ucs4string_to_string(u4_str);
	}
}

void ellipsis_truncate(std::string& str, const size_t size)
{
	const size_t prev_size = str.length();

	utf8::truncate(str, size);

	if(str.length() != prev_size) {
		str += ellipsis;
	}
}

} // end namespace utils

namespace utf8 {

static int byte_size_from_utf8_first(const unsigned char ch)
{
	if (!(ch & 0x80)) {
		return 1;  // US-ASCII character, 1 byte
	}
	/* first bit set: character not in US-ASCII, multiple bytes
	 * number of set bits at the beginning = bytes per character
	 * e.g. 11110xxx indicates a 4-byte character */
	int count = count_leading_ones(ch);
	if (count == 1 || count > 6) {		// count > 4 after RFC 3629
		throw invalid_utf8_exception(); // Stop on invalid characters
	}
	return count;
}

iterator::iterator(const std::string& str) :
	current_char(0),
	string_end(str.end()),
	current_substr(std::make_pair(str.begin(), str.begin()))
{
	update();
}

iterator::iterator(std::string::const_iterator const &beg,
		std::string::const_iterator const &end) :
	current_char(0),
	string_end(end),
	current_substr(std::make_pair(beg, beg))
{
	update();
}

iterator iterator::begin(std::string const &str)
{
	return iterator(str.begin(), str.end());
}

iterator iterator::end(const std::string& str)
{
	return iterator(str.end(), str.end());
}

bool iterator::operator==(const utf8::iterator& a) const
{
	return current_substr.first == a.current_substr.first;
}

iterator& iterator::operator++()
{
	current_substr.first = current_substr.second;
	update();
	return *this;
}

ucs4::char_t iterator::operator*() const
{
	return current_char;
}

bool iterator::next_is_end()
{
	if(current_substr.second == string_end)
		return true;
	return false;
}

const std::pair<std::string::const_iterator, std::string::const_iterator>& iterator::substr() const
{
	return current_substr;
}

void iterator::update()
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

	// Check for non-shortest-form encoding
	// This has been forbidden in Unicode 3.1 for security reasons
	if (size > ::byte_size_from_ucs4_codepoint(current_char))
		throw invalid_utf8_exception();
}


utf8::string lowercase(const utf8::string& s)
{
	if(!s.empty()) {
		utf8::iterator itor(s);
		std::string res;

		for(;itor != utf8::iterator::end(s); ++itor) {
			ucs4::char_t uchar = *itor;
			// If wchar_t is less than 32 bits wide, we cannot apply towlower() to all codepoints
			if(uchar <= static_cast<ucs4::char_t>(std::numeric_limits<wchar_t>::max()) &&
			   uchar >= static_cast<ucs4::char_t>(std::numeric_limits<wchar_t>::min()))
				uchar = towlower(static_cast<wchar_t>(uchar));
			res += utils::ucs4char_to_string(uchar);
		}

		res.append(itor.substr().second, s.end());
		return res;
	}
	return s;
}

size_t index(const utf8::string& str, const size_t index)
{
	// chr counts characters, i is the codepoint index
	// remark: several functions rely on the fallback to str.length()
	unsigned int chr, i = 0, len = str.size();
	try {
		for (chr=0; chr<index && i<len; ++chr) {
			i += byte_size_from_utf8_first(str[i]);
		}
	} catch(invalid_utf8_exception&) {
		ERR_GENERAL << "Invalid UTF-8 string.\n";
	}
	return i;
}

size_t size(const utf8::string& str)
{
	unsigned int chr, i = 0, len = str.size();
	try {
		for (chr=0; i<len; ++chr) {
			i += byte_size_from_utf8_first(str[i]);
		}
	} catch(invalid_utf8_exception&) {
		ERR_GENERAL << "Invalid UTF-8 string.\n";
	}
	return chr;
}

utf8::string& insert(utf8::string& str, const size_t pos, const utf8::string& insert)
{
	return str.insert(index(str, pos), insert);
}

utf8::string& erase(utf8::string& str, const size_t start, const size_t len)
{
	if (start > size(str)) return str;
	unsigned pos = index(str, start);

	if (len == std::string::npos) {
		// without second argument, std::string::erase truncates
		return str.erase(pos);
	} else {
		return str.erase(pos, index(str,start+len) - pos);
	}
}

utf8::string& truncate(utf8::string& str, const size_t size)
{
	return erase(str, size);
}

} // end namespace utf8
