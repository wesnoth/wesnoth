/* $Id$ */
/*
   Copyright (C) 2006 - 2008 by Rusty Russell <rusty@rustcorp.com.au>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file unit_map.hpp
//!

#ifndef UNIT_MAP_H_INCLUDED
#define UNIT_MAP_H_INCLUDED

class unit;
#include <cstring>
#include "map.hpp"

// We used to just open-code a std::map<location,unit>,
// but as unit gained weight leading up to 1.1.3,
// manipulating the map caused significant performance issues
// for the AI, which had to actually move units for accurate
// simulation with the new, more powerful filtering.
// This class eases the transition, by providing a wrapper
// which acts like a map of units, not unit pointers,
// except implemented with pointers and hence providing
// a cheap move function.

class unit_map
{
public:
	unit_map() { };
	unit_map(const unit_map &that);
	unit_map &operator =(const unit_map &that);
	//! A unit map with a single unit in it.
	explicit unit_map(const gamemap::location &loc, const unit &u);
	~unit_map();

	//! We actually keep map to pointers to pairs.  Easy to fake iterators.
	typedef std::map<gamemap::location,std::pair<gamemap::location,unit>*> pmap;
	struct iterator;
	struct const_iterator {
		const_iterator() { }
		const_iterator(const iterator &i) : i_(i.i_) { }

		const std::pair<gamemap::location,unit>* operator->() const
			{ return i_->second; }

		std::pair<gamemap::location,unit> operator*() const;

		const_iterator operator++()
			{ return const_iterator(++i_); }

		const_iterator operator++(int)
			{ return const_iterator(i_++); }

		const_iterator operator--()
			{ return const_iterator(--i_); }

		bool operator==(const const_iterator &that) const
			{ return that.i_ == this->i_; }

		bool operator!=(const const_iterator &that) const
			{ return that.i_ != this->i_; }

		explicit const_iterator(pmap::const_iterator i)	: i_(i) { }

	private:
		pmap::const_iterator i_;
	};

	struct iterator {
		iterator() { }

		std::pair<gamemap::location,unit> *operator->() const
			{ return i_->second; }

		std::pair<gamemap::location,unit> operator*() const;

		iterator operator++()
			{ return iterator(++i_); }

		iterator operator++(int)
			{ return iterator(i_++); }

		bool operator==(const iterator &that) const
			{ return that.i_ == this->i_; }

		bool operator!=(const iterator &that) const
			{ return that.i_ != this->i_; }

		explicit iterator(pmap::iterator i)	: i_(i) { }

		friend struct const_iterator;
	private:
		pmap::iterator i_;
	};

	iterator find(const gamemap::location &loc) {
		return iterator(map_.find(loc));
	}
	const_iterator find(const gamemap::location &loc) const	{
		return const_iterator(map_.find(loc));
	}

	size_t count(const gamemap::location &loc) const {
		return map_.count(loc);
	}

	iterator begin() {
		return iterator(map_.begin());
	}

	const_iterator begin() const {
		return const_iterator(map_.begin());
	}

	iterator end() {
		return iterator(map_.end());
	}

	const_iterator end() const {
		return const_iterator(map_.end());
	}

	size_t size() const {
		return map_.size();
	}

	void clear();

	//! Extract (like erase, only don't delete).
	std::pair<gamemap::location,unit> *extract(const gamemap::location &loc);

	//! Map owns pointer after this.  Loc must be currently empty.
	void add(std::pair<gamemap::location,unit> *p);

	//! Like add, but loc must be occupied (implicitly erased).
	void replace(std::pair<gamemap::location,unit> *p);

	void erase(iterator pos);
	size_t erase(const gamemap::location &loc);

private:

	void delete_all();

	//! A map of pairs is redundant, but makes it possible to imitate a map of location,unit.
	std::map<gamemap::location,std::pair<gamemap::location,unit>*> map_;
};

#endif	// UNIT_MAP_H_INCLUDED
