/* $Id$ */
/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2011 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "formula_string_utils.hpp"

#include "config.hpp"
#include "gettext.hpp"

#include "formula_string_utils_backend.hpp"


namespace utils {

class string_map_variable_set : public variable_set
{
public:
	string_map_variable_set(const string_map& map) : map_(map) {};

	virtual config::attribute_value get_variable_const(const n_token::t_token &key) const {
		config::attribute_value val;
		const string_map::const_iterator itor = map_.find(key);
		if (itor != map_.end())
			val = itor->second;
		return val;
	}
	virtual config::attribute_value get_variable_const(const std::string &key) const{
		return get_variable_const(n_token::t_token(key));}
private:
	const string_map& map_;

};
}

namespace utils {

std::string interpolate_variables_into_string(const std::string &str, const string_map * const symbols) {
	string_map_variable_set set(*symbols);
	return interpolate_variables_into_string(str, set);
}

std::string interpolate_variables_into_string(const std::string &str, const variable_set& variables) {
	n_token::t_token token(str);
	wml_interpolation::t_parse_and_interpolator interpolator(token);
	token = interpolator.parse_and_interpolate(variables);
	return (*token);
}

void interpolate_variables_into_token(n_token::t_token &token, const variable_set& variables) {
	wml_interpolation::t_parse_and_interpolator interpolator(token);
	token = interpolator.parse_and_interpolate(variables);
}

t_string interpolate_variables_into_tstring(const t_string &tstr, const variable_set& variables) {
	if(!tstr.str().empty()) {
		n_token::t_token token(tstr);
		wml_interpolation::t_parse_and_interpolator interpolator(token);
		token = interpolator.parse_and_interpolate(variables);
		if(tstr.str() != token) {
			return t_string( (*token) );
		}
	}
	return tstr;
}

}

std::string vgettext(const char *msgid, const utils::string_map& symbols)
{
	const std::string orig(_(msgid));
	const std::string msg = utils::interpolate_variables_into_string(orig, &symbols);
	return msg;
}

std::string vgettext(const char *domain
		, const char *msgid
		, const utils::string_map& symbols)
{
	const std::string orig(dgettext(domain, msgid));
	const std::string msg = utils::interpolate_variables_into_string(orig, &symbols);
	return msg;
}
std::string vngettext(const char* sing, const char* plur, int n, const utils::string_map& symbols)
{
	const std::string orig(_n(sing, plur, n));
	const std::string msg = utils::interpolate_variables_into_string(orig, &symbols);
	return msg;
}
