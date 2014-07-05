/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
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
 * This namespace contains the function that checks if a unit matches
 * a filter. It helps by simplifying the unit object (which before now
 * holds the "match" function).
 *
 * TODO:
 * Make a class that abstracts a unit filter, assembles the constituent
 * side filters and terrain filters and conditional filters, and caches
 * these to speed up repeated application of the filter.
 */

#ifndef INCLUDED_UNIT_FILTER_HPP_
#define INCLUDED_UNIT_FILTER_HPP_

#include <boost/shared_ptr.hpp>

class filter_context;
class unit;
class vconfig;
struct map_location;

class unit_filter_abstract_impl {
public:
	virtual bool matches(const unit & u, const map_location & loc) const = 0;
	virtual ~unit_filter_abstract_impl() {}
};

class unit_filter {
public:
	unit_filter(const vconfig & cfg, const filter_context * fc, bool use_flat_tod = false); //!< Constructs a unit filter from a config and a context. This function should give the most efficient implementation available.

	// Copy and Swap Idiom for the interface -- does not copy the underlying impl
	unit_filter(const unit_filter & o ) : impl_(o.impl_) {}
	void swap(unit_filter & o) {
		impl_.swap(o.impl_);
	}
	unit_filter & operator=(unit_filter o) {
		swap(o);
		return *this;
	}

	bool matches(const unit & u, const map_location & loc) const {
		return impl_->matches(u,loc);
	}
	/// Determine if *this matches @a filter at its current location.
	/// (Only use for units currently on the map; otherwise use the overload
	/// that takes a location, possibly with a null location.)
	bool matches(const unit & u) const;

	bool operator()(const unit & u, const map_location & loc) const {
		return matches(u,loc);
	}

	bool operator()(const unit & u) const {
		return matches(u);
	}
private:
	boost::shared_ptr<unit_filter_abstract_impl> impl_;
};

#endif
