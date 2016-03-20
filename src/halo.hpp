/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#ifndef HALO_HPP_INCLUDED
#define HALO_HPP_INCLUDED

class display;

#include "map/location.hpp"

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace halo
{


class halo_impl;

class halo_record;

typedef boost::shared_ptr<halo_record> handle;

enum ORIENTATION { NORMAL, HREVERSE, VREVERSE, HVREVERSE };

const int NO_HALO = 0;

class manager
{
public:
	manager(display& disp);

	/**
	 * Add a haloing effect using 'image centered on (x,y).
	 * @return 	The handle to the halo object.
	 * @retval 	0 is the invalid handle.
	 *
	 * If the halo is attached to an item, it needs to be hidden if the
	 * shroud is active.  (Note it will be shown with the fog active.)
	 * If it is not attached to an item, the location should be set to -1, -1
	 */
	handle add(int x, int y, const std::string& image, const map_location& loc,
			halo::ORIENTATION orientation=NORMAL, bool infinite=true);

	/** Set the position of an existing haloing effect, according to its handle. */
	void set_location(const handle & h, int x, int y);

	/** Remove the halo with the given handle. */
	void remove(const handle & h);

	/**
	 * Render and unrender haloes.
	 *
	 * Which haloes are rendered is determined by invalidated_locations and the
	 * internal state in the control sets (in halo.cpp).
	 */
	void unrender(std::set<map_location> invalidated_locations);
	void render();

private:
	boost::shared_ptr<halo_impl> impl_;
};

/**
 * RAII object which manages a halo. When it goes out of scope it removes the corresponding halo entry.
 */
class halo_record : public boost::noncopyable
{
public:
	halo_record();
	halo_record(int id, const boost::shared_ptr<halo_impl> & my_manager);
	~halo_record();

	bool valid() const {
		return id_ != NO_HALO && !my_manager_.expired();
	}

	friend class manager;
private:
	int id_;
	boost::weak_ptr<halo_impl> my_manager_;

};

} // end namespace halo

#endif
