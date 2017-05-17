/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Copyright (C) 2005 - 2017 by Philippe Plantier <ayin@anathas.org>
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

#include "gettext.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"
#include "utils/general.hpp"
#include <cassert>
#include <array>

#include <boost/algorithm/string.hpp>

static lg::log_domain log_engine("engine");
#define ERR_GENERAL LOG_STREAM(err, lg::general())
#define ERR_NG LOG_STREAM(err, log_engine)

namespace utils {

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

/**
 * Splits a (comma-)separated string into a vector of pieces.
 * @param[in]  val    A (comma-)separated string.
 * @param[in]  c      The separator character (usually a comma).
 * @param[in]  flags  Flags controlling how the split is done.
 *                    This is a bit field with two settings (both on by default):
 *                    REMOVE_EMPTY causes empty pieces to be skipped/removed.
 *                    STRIP_SPACES causes the leading and trailing spaces of each piece to be ignored/stripped.
 *
 *                    Basic method taken from http://stackoverflow.com/a/236803
 */
std::vector<std::string> split(const std::string& val, const char c, const int flags)
{
	std::vector<std::string> res;

	std::stringstream ss;
	ss.str(val);

	std::string item;
	while(std::getline(ss, item, c)) {
		if(flags & STRIP_SPACES) {
			boost::trim(item);
		}

		if(!(flags & REMOVE_EMPTY) || !item.empty()) {
			res.push_back(std::move(item));
		}
	}

	return res;
}

std::vector<std::string> square_parenthetical_split(const std::string& val,
		const char separator, const std::string& left,
		const std::string& right,const int flags)
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
		ERR_GENERAL << "Separator must be specified for square bracket split funtion." << std::endl;
		return res;
	}

	if(left.size()!=right.size()){
		ERR_GENERAL << "Left and Right Parenthesis lists not same length" << std::endl;
		return res;
	}

	while (true) {
		if(i2 == val.end() || (!in_parenthesis && *i2 == separator)) {
			//push back square contents
			size_t size_square_exp = 0;
			for (size_t i=0; i < square_left.size(); i++) {
				std::string tmp_val(square_left[i]+1,square_right[i]);
				std::vector< std::string > tmp = split(tmp_val);
				for(const std::string& piece : tmp) {
					size_t found_tilde = piece.find_first_of('~');
					if (found_tilde == std::string::npos) {
						size_t found_asterisk = piece.find_first_of('*');
						if (found_asterisk == std::string::npos) {
							std::string tmp2(piece);
							boost::trim(tmp2);
							square_expansion.push_back(tmp2);
						}
						else { //'*' multiple expansion
							std::string s_begin = piece.substr(0,found_asterisk);
							boost::trim(s_begin);
							std::string s_end = piece.substr(found_asterisk+1);
							boost::trim(s_end);
							for (int ast=std::stoi(s_end); ast>0; --ast)
								square_expansion.push_back(s_begin);
						}
					}
					else { //expand number range
						std::string s_begin = piece.substr(0,found_tilde);
						boost::trim(s_begin);
						int begin = std::stoi(s_begin);
						size_t padding = 0, padding_end = 0;
						while (padding<s_begin.size() && s_begin[padding]=='0') {
							padding++;
						}
						std::string s_end = piece.substr(found_tilde+1);
						boost::trim(s_end);
						int end = std::stoi(s_end);
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
							std::string pb = std::to_string(k);
							for (size_t p=pb.size(); p<=padding; p++)
								pb = std::string("0") + pb;
							square_expansion.push_back(pb);
						}
					}
				}
				if (i*square_expansion.size() != (i+1)*size_square_exp ) {
					std::string tmp2(i1, i2);
					ERR_GENERAL << "Square bracket lengths do not match up: " << tmp2 << std::endl;
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
					boost::trim_right(new_val);
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
			ERR_GENERAL << "Mismatched parenthesis:\n"<<val<< std::endl;;
	}

	return res;
}

std::map<std::string, std::string> map_split(
		  const std::string& val
		, char major
		, char minor
		, int flags
		, const std::string& default_value)
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

std::vector<std::string> parenthetical_split(const std::string& val,
		const char separator, const std::string& left,
		const std::string& right,const int flags)
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
		ERR_GENERAL << "Left and Right Parenthesis lists not same length" << std::endl;
		return res;
	}

	while (i2 != val.end()) {
		if(!in_parenthesis && separator && *i2 == separator){
			std::string new_val(i1, i2);
			if (flags & STRIP_SPACES)
				boost::trim_right(new_val);
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
					boost::trim(new_val);
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
						boost::trim(new_val);
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
		boost::trim(new_val);
	if (!(flags & REMOVE_EMPTY) || !new_val.empty())
		res.push_back(std::move(new_val));

	if(!part.empty()){
			ERR_GENERAL << "Mismatched parenthesis:\n"<<val<< std::endl;;
	}

	return res;
}

// Modify a number by string representing integer difference, or optionally %
int apply_modifier( const int number, const std::string &amount, const int minimum ) {
	// wassert( amount.empty() == false );
	int value = 0;
	try {
		value = std::stoi(amount);
	} catch(std::invalid_argument) {}
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
	static const std::string nonresv_str =
		"-."
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"_"
		"abcdefghijklmnopqrstuvwxyz"
		"~";
	static const std::set<char> nonresv(nonresv_str.begin(), nonresv_str.end());

	std::ostringstream res;
	res << std::hex;
	res.fill('0');

	for(char c : str) {
		if(nonresv.count(c) == 0) {
			res << c;
			continue;
		}

		res << '%';
		res.width(2);
		res << int(c);
	}

	return res.str();
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

std::string bool_string(const bool value)
{
	std::ostringstream ss;
	ss << std::boolalpha << value;

	return ss.str();
}

std::string signed_value(int val)
{
	std::ostringstream oss;
	oss << (val >= 0 ? "+" : font::unicode_minus) << std::abs(val);
	return oss.str();
}

std::string half_signed_value(int val)
{
	std::ostringstream oss;
	if (val < 0)
		oss << font::unicode_minus;
	oss << std::abs(val);
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

std::string si_string(double input, bool base2, const std::string& unit) {
	const double multiplier = base2 ? 1024 : 1000;

	typedef std::array<std::string, 9> strings9;

	strings9 prefixes;
	strings9::const_iterator prefix;
	if (input == 0.0) {
		strings9 tmp { { "","","","","","","","","" } };
		prefixes = tmp;
		prefix = prefixes.begin();
	} else if (input < 1.0) {
		strings9 tmp { {
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
		strings9 tmp { {
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

std::vector<std::string> quoted_split(const std::string& val, char c, int flags, char quote)
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
				boost::trim(new_val);
			if (!(flags & REMOVE_EMPTY) || !new_val.empty())
				res.push_back(std::move(new_val));
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
		boost::trim(new_val);
	if (!(flags & REMOVE_EMPTY) || !new_val.empty())
		res.push_back(new_val);

	return res;
}

std::pair<int, int> parse_range(const std::string& str)
{
	const std::string::const_iterator dash = std::find(str.begin(), str.end(), '-');
	const std::string a(str.begin(), dash);
	const std::string b = dash != str.end() ? std::string(dash + 1, str.end()) : a;
	std::pair<int,int> res {0,0};
	try {
		res = std::make_pair(std::stoi(a), std::stoi(b));
		if (res.second < res.first) {
			res.second = res.first;
		}
	} catch(std::invalid_argument) {}

	return res;
}

std::vector<std::pair<int, int>> parse_ranges(const std::string& str)
{
	std::vector< std::pair< int, int > > to_return;
	std::vector<std::string> strs = utils::split(str);
	std::vector<std::string>::const_iterator i, i_end=strs.end();
	for(i = strs.begin(); i != i_end; ++i) {
		to_return.push_back(parse_range(*i));
	}
	return to_return;
}


void ellipsis_truncate(std::string& str, const size_t size)
{
	const size_t prev_size = str.length();

	utf8::truncate(str, size);

	if(str.length() != prev_size) {
		str += font::ellipsis;
	}
}

} // end namespace utils
