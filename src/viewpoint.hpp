/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file map.hpp  */

#ifndef VIEWPOINT_H_INCLUDED
#define VIEWPOINT_H_INCLUDED

struct map_location;

class viewpoint
{
public:
	virtual bool shrouded(const map_location& loc) const = 0;
	virtual bool fogged(const map_location& loc) const = 0;
	virtual ~viewpoint() {};
};

#endif
