/* $Id$ */
/*
   Copyright (C) 2010 - 2011 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "exceptions.hpp"
#include "gettext.hpp"
#include <sstream>

namespace game {

wml_syntax_error::wml_syntax_error(std::string const & reason)
	: output_( _("WML Syntax error:: Variable in WML string cannot be evaluated because, ") + reason) {}

/**Tries to parse the error message so that an arror <-- points to the location of the error */
wml_syntax_error::wml_syntax_error(std::string const & str, size_t const & pos, std::string const & reason)
	: output_() {
	std::stringstream ss;
	ss << _("WML Syntax error:: Variable in WML string cannot be evaluated because, ") << reason<< _(" in \n\"");
	ss << str << "\" at \n\"";
	ss << str.substr(0, pos);
	if(pos != str.size()){
		ss << "\" <--";
		if((pos+1) != str.size()){
			ss << "-->\"";
			ss << str.substr(pos) << "\"";
		}
	}
	else { ss << "\"<--"; }

	output_ = ss.str();
}


const char * wml_syntax_error::what() const throw() {
	return output_.c_str();
}

}
