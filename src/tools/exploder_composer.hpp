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

#ifndef EXPLODER_COMPOSER_HPP_INCLUDED
#define EXPLODER_COMPOSER_HPP_INCLUDED

#include "exploder_utils.hpp"
#include "exploder_cutter.hpp"

class composer
{
public:
	composer();

	shared_sdl_surface compose(const std::string &src, const std::string &dest);

	void set_interactive(bool value);
	void set_verbose(bool value);
private:
	bool interactive_;
	bool verbose_;
};

#endif

